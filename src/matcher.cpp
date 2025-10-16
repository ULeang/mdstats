#include "matcher.hpp"

#include <expected>
#include <tuple>

MatcherWorker::MatcherWorker() : QObject() {}

static std::expected<std::tuple<std::string, cv::Rect, cv::Rect>, ErrorType>
_determine_resolution(long width, long height);

ErrorType MatcherWorker::main_matcher()
{
#define emit_exited_return(err) \
    do                          \
    {                           \
        emit exited(err);       \
        return err;             \
    } while (false)

    using std::format;

    external_input_flag.store(false);
    stop_requested.store(false);

    ScreenShot ss;

    if (!ss.is_ok())
    {
        logln("cannot acquire device context, check your monitor");
        emit_exited_return(ErrorType::ErrDeviceContext);
    }

    HWND hwnd = FindWindowA(NULL, prog::env::capture_window_title.c_str());
    if (!hwnd)
    {
        logln(format("cannot find window '{}'", prog::env::capture_window_title));
        emit_exited_return(ErrorType::ErrFindWindow);
    }
    auto rect_e = ss.get_window_rect(hwnd);
    if (!rect_e.has_value())
    {
        logln(format("cannot get window rect"));
        emit_exited_return(rect_e.error());
    }

    double scale = ss.get_screen_scale(true);

    RECT rect = rect_e.value();
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    logln(format("init window size \t: {}x{}", width, height));
    auto resolution_e = _determine_resolution(width, height);
    if (!resolution_e.has_value())
    {
        logln(format("cannot determine resolution"));
        emit_exited_return(resolution_e.error());
    }
    auto [res_text, crop_coin, crop_result] = resolution_e.value();
    logln(format("determined resolution \t: {}", res_text));

    std::string path{prog::env::opencv_templ_directory + res_text + "\\"};
    if (!check_resources({std::filesystem::path{path + "coin_win.png"},
                          std::filesystem::path{path + "coin_lose.png"},
                          std::filesystem::path{path + "go_first.png"},
                          std::filesystem::path{path + "go_second.png"},
                          std::filesystem::path{path + "victory.png"},
                          std::filesystem::path{path + "defeat.png"}}))
    {
        logln(format(
            "opencv template is missing, or current window resolution '{}' is not supported yet",
            res_text));
        emit_exited_return(ErrorType::ErrCheckResources);
    }

    auto f_coin = capture_fn_generator(ss, hwnd, scale, crop_coin);
    auto f_result = capture_fn_generator(ss, hwnd, scale, crop_result);

    if constexpr (prog::env::debug::test_capture_flag)
    {
        auto cap_coin = f_coin();
        auto cap_result = f_result();
        logln(format("cap_coin {}exists, cap_result {}exists",
                     cap_coin.has_value() ? "" : "NOT ", cap_result.has_value() ? "" : "NOT "),
              LogLevel::ALWAYS);
        if (cap_coin.has_value())
        {
            cv::imwrite("cap_coin.png", cap_coin.value());
        }
        if (cap_result.has_value())
        {
            cv::imwrite("cap_result.png", cap_result.value());
        }
    }

    Matcher match_coin({(path + "coin_win.png").c_str(), (path + "coin_lose.png").c_str()},
                       prog::env::matcher_threshold,
                       prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);
    Matcher match_st_nd({(path + "go_first.png").c_str(), (path + "go_second.png").c_str()},
                        prog::env::matcher_threshold,
                        prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);
    Matcher match_result({(path + "victory.png").c_str(), (path + "defeat.png").c_str()},
                         prog::env::matcher_threshold,
                         prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);

    auto _matcher_thread_helper =
        [this,
         &stop_requested = stop_requested,
         &external_input_flag = external_input_flag,
         &external_input = external_input](
            Matcher &matcher,
            const std::function<std::optional<cv::Mat>()> &f,
            MatcherGotType got) -> std::expected<size_t, ErrorType>
    {
        while (true)
        {
            // ad-hoc for manual
            if (external_input_flag.load(std::memory_order_acquire) == true)
            {
                auto ei = external_input;
                external_input_flag.store(false, std::memory_order_release);
                emit got_match_step(got, ei);
                return ei;
            }

            auto match_r = matcher.try_once(f);
            if (match_r.has_value())
            {
                emit got_match_step(got, match_r.value());
                return match_r.value();
            }
            if (stop_requested.load(std::memory_order_acquire) == true)
            {
                emit exited(ErrorType::ErrStopRequested);
                return std::unexpected{ErrorType::ErrStopRequested};
            }
            QThread::msleep(prog::env::config::misc_matcher_sleep_ms);
        }
    };

    while (true)
    {
        auto coin = _matcher_thread_helper(match_coin, f_coin, MatcherGotType::Coin);
        if (coin.has_value())
        {
            logln(format("{}", coin.value() == 0 ? "coin win" : "coin lose"));
        }
        else
        {
            return coin.error();
        }
        auto st_nd = _matcher_thread_helper(match_st_nd, f_coin, MatcherGotType::St_nd);
        if (st_nd.has_value())
        {
            logln(format("{}", st_nd.value() == 0 ? "go first" : "go second"));
        }
        else
        {
            return st_nd.error();
        }
        auto result = _matcher_thread_helper(match_result, f_result, MatcherGotType::Result);
        if (result.has_value())
        {
            logln(format("{}", result.value() == 0 ? "victory" : "defeat"));
        }
        else
        {
            return result.error();
        }
    }
}
void MatcherWorker::set_extern_input(size_t input)
{
    external_input = input;
    external_input_flag.store(true, std::memory_order_release);
}
void MatcherWorker::request_stop()
{
    stop_requested.store(true, std::memory_order_release);
}

std::expected<std::tuple<std::string, cv::Rect, cv::Rect>, ErrorType>
_determine_resolution(long width, long height)
{
    if (width < 1280)
    {
        return std::unexpected{ErrorType::ErrDeterRes};
    }
    if (width < 1366)
    {
        return std::make_tuple("1280x720", cv::Rect{0, 0, 1280, 720}, cv::Rect{0, 0, 1280, 720});
    }
    if (width < 1440)
    {
        return std::make_tuple("1366x768", cv::Rect{0, 0, 1366, 768}, cv::Rect{0, 0, 1366, 768});
    }
    if (width < 1600)
    {
        return std::make_tuple("1440x810", cv::Rect{0, 0, 1440, 810}, cv::Rect{0, 0, 1440, 810});
    }
    if (width < 1920)
    {
        return std::make_tuple("1600x900", cv::Rect{600, 530, 400, 160}, cv::Rect{550, 300, 570, 380});
    }
    if (width < 2048)
    {
        return std::make_tuple("1920x1080", cv::Rect{700, 630, 500, 200}, cv::Rect{650, 350, 620, 450});
    }
    if (width < 2560)
    {
        return std::make_tuple("2048x1152", cv::Rect{750, 650, 550, 220}, cv::Rect{700, 380, 670, 480});
    }
    if (width < 3200)
    {
        return std::make_tuple("2560x1440", cv::Rect{950, 850, 700, 250}, cv::Rect{880, 450, 840, 600});
    }
    if (width < 3840)
    {
        return std::make_tuple("3200x1800", cv::Rect{1200, 1050, 800, 280}, cv::Rect{1100, 600, 1050, 700});
    }
    return std::make_tuple("3840x2160", cv::Rect{1450, 1300, 950, 300}, cv::Rect{1400, 750, 1150, 700});
}
