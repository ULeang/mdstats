#include "matcher.hpp"

#include <expected>
#include <toml.hpp>
#include <tuple>

#include "opencv_matcher.hpp"
#include "prog_config.hpp"
#include "screenshot.hpp"

MatcherWorker::MatcherWorker() : QObject() {}

static std::expected<std::tuple<std::string, cv::Rect, cv::Rect, cv::Rect>, ErrorType>
  _determine_resolution(long width, long height);

ErrorType MatcherWorker::main_matcher() {
#define emit_exited_return(err) \
  do {                          \
    emit exited(err);           \
    return err;                 \
  } while (false)

  using std::format;

  external_input_flag.store(false);
  stop_requested.store(false);

  ScreenShot ss;

  if (!ss.is_ok()) {
    logln("cannot acquire device context, check your monitor");
    emit_exited_return(ErrorType::ErrDeviceContext);
  }

  HWND hwnd = FindWindowA(NULL, prog::env::capture_window_title.c_str());
  if (!hwnd) {
    logln(format("cannot find window '{}'", prog::env::capture_window_title));
    emit_exited_return(ErrorType::ErrFindWindow);
  }
  auto rect_e = ss.get_window_rect(hwnd);
  if (!rect_e.has_value()) {
    logln(format("cannot get window rect"));
    emit_exited_return(rect_e.error());
  }

  double scale = ss.get_screen_scale(true);

  RECT rect   = rect_e.value();
  auto width  = rect.right - rect.left;
  auto height = rect.bottom - rect.top;
  logln(format("init window size \t: {}x{}", width, height));
  auto resolution_e = _determine_resolution(width, height);
  if (!resolution_e.has_value()) {
    logln(format("cannot determine resolution"));
    emit_exited_return(resolution_e.error());
  }
  auto [res_name, crop_coin, crop_stnd, crop_result] = resolution_e.value();
  logln(format("determined resolution \t: {}", res_name));

  std::string path{prog::env::opencv_templ_directory + res_name + "\\"};
  if (!check_resources({std::filesystem::path{path + "coin_win.png"},
                        std::filesystem::path{path + "coin_lose.png"},
                        std::filesystem::path{path + "go_first.png"},
                        std::filesystem::path{path + "go_second.png"},
                        std::filesystem::path{path + "victory.png"},
                        std::filesystem::path{path + "defeat.png"}})) {
    logln(
      format("opencv template is missing, or current window resolution '{}' is not supported yet, "
             "check '{}'",
             res_name, path));
    emit_exited_return(ErrorType::ErrCheckResources);
  }

  auto f_coin   = capture_fn_generator(ss, hwnd, scale, crop_coin);
  auto f_stnd   = capture_fn_generator(ss, hwnd, scale, crop_stnd);
  auto f_result = capture_fn_generator(ss, hwnd, scale, crop_result);

  if (prog::env::config::debug_test_capture) {
    auto cap_coin   = f_coin();
    auto cap_stnd   = f_stnd();
    auto cap_result = f_result();
    logln(format("cap_coin {}exists, cap_stnd {}exists, cap_result {}exists",
                 cap_coin.has_value() ? "" : "NOT ", cap_stnd.has_value() ? "" : "NOT ",
                 cap_result.has_value() ? "" : "NOT "),
          LogLevel::ALWAYS);
    if (cap_coin.has_value()) {
      cv::imwrite("cap_coin.png", cap_coin.value());
    }
    if (cap_stnd.has_value()) {
      cv::imwrite("cap_stnd.png", cap_stnd.value());
    }
    if (cap_result.has_value()) {
      cv::imwrite("cap_result.png", cap_result.value());
    }
  }

  Matcher match_coin({(path + "coin_win.png").c_str(), (path + "coin_lose.png").c_str()},
                     prog::env::matcher_threshold, prog::env::debug::matcher_img_log,
                     prog::env::debug::matcher_text_log);
  Matcher match_st_nd({(path + "go_first.png").c_str(), (path + "go_second.png").c_str()},
                      prog::env::matcher_threshold, prog::env::debug::matcher_img_log,
                      prog::env::debug::matcher_text_log);
  Matcher match_result({(path + "victory.png").c_str(), (path + "defeat.png").c_str()},
                       prog::env::matcher_threshold, prog::env::debug::matcher_img_log,
                       prog::env::debug::matcher_text_log);

  auto _matcher_thread_helper =
    [this, &stop_requested = stop_requested, &external_input_flag = external_input_flag,
     &external_input = external_input](Matcher                                       &matcher,
                                       const std::function<std::optional<cv::Mat>()> &f,
                                       MatcherGotType got) -> std::expected<size_t, ErrorType> {
    while (true) {
      // ad-hoc for manual
      if (external_input_flag.load(std::memory_order_acquire) == true) {
        auto ei = external_input;
        external_input_flag.store(false, std::memory_order_release);
        emit got_match_step(got, ei);
        return ei;
      }

      auto match_r = matcher.try_once(f);
      if (match_r.has_value()) {
        emit got_match_step(got, match_r.value());
        return match_r.value();
      }
      if (stop_requested.load(std::memory_order_acquire) == true) {
        emit exited(ErrorType::ErrStopRequested);
        return std::unexpected{ErrorType::ErrStopRequested};
      }
      QThread::msleep(prog::env::config::misc_matcher_sleep_ms);
    }
  };

  while (true) {
    auto coin = _matcher_thread_helper(match_coin, f_coin, MatcherGotType::Coin);
    if (coin.has_value()) {
      logln(format("{}", coin.value() == 0 ? "coin win" : "coin lose"));
    } else {
      return coin.error();
    }
    auto st_nd = _matcher_thread_helper(match_st_nd, f_stnd, MatcherGotType::St_nd);
    if (st_nd.has_value()) {
      logln(format("{}", st_nd.value() == 0 ? "go first" : "go second"));
    } else {
      return st_nd.error();
    }
    auto result = _matcher_thread_helper(match_result, f_result, MatcherGotType::Result);
    if (result.has_value()) {
      logln(format("{}", result.value() == 0 ? "victory" : "defeat"));
    } else {
      return result.error();
    }
  }
}
void MatcherWorker::set_extern_input(size_t input) {
  external_input = input;
  external_input_flag.store(true, std::memory_order_release);
}
void MatcherWorker::request_stop() {
  stop_requested.store(true, std::memory_order_release);
}

std::expected<std::tuple<std::string, cv::Rect, cv::Rect, cv::Rect>,
              ErrorType> static _determine_resolution(long width, long height) {
  int resolutions[] = {1280, 1366, 1440, 1600, 1920, 2048, 2560, 3200, 3840};
  for (size_t i = sizeof(resolutions) / sizeof(resolutions[0]) - 1; i > 0; --i) {
    if (width >= resolutions[i]) {
      auto config_r = toml::try_parse(prog::env::opencv_templ_directory
                                      + prog::env::opencv_templ_config_filename);
      if (!config_r.is_ok()) {
        std::cerr << config_r.unwrap_err().at(0) << std::endl;
        logln("fatal : load template config fail");
        return std::unexpected{ErrorType::ErrDeterRes};
      }
      auto config = config_r.unwrap();
      auto res    = config.at(std::to_string(resolutions[i]));

      auto name               = toml::find<std::string>(res, "name");
      auto crop_coin_x        = toml::find<size_t>(res, "crop_coin", "x");
      auto crop_coin_y        = toml::find<size_t>(res, "crop_coin", "y");
      auto crop_coin_width    = toml::find<size_t>(res, "crop_coin", "width");
      auto crop_coin_height   = toml::find<size_t>(res, "crop_coin", "height");
      auto crop_stnd_x        = toml::find<size_t>(res, "crop_stnd", "x");
      auto crop_stnd_y        = toml::find<size_t>(res, "crop_stnd", "y");
      auto crop_stnd_width    = toml::find<size_t>(res, "crop_stnd", "width");
      auto crop_stnd_height   = toml::find<size_t>(res, "crop_stnd", "height");
      auto crop_result_x      = toml::find<size_t>(res, "crop_result", "x");
      auto crop_result_y      = toml::find<size_t>(res, "crop_result", "y");
      auto crop_result_width  = toml::find<size_t>(res, "crop_result", "width");
      auto crop_result_height = toml::find<size_t>(res, "crop_result", "height");

      return std::make_tuple(
        name,
        cv::Rect{static_cast<int>(crop_coin_x), static_cast<int>(crop_coin_y),
                 static_cast<int>(crop_coin_width), static_cast<int>(crop_coin_height)},
        cv::Rect{static_cast<int>(crop_stnd_x), static_cast<int>(crop_stnd_y),
                 static_cast<int>(crop_stnd_width), static_cast<int>(crop_stnd_height)},
        cv::Rect{static_cast<int>(crop_result_x), static_cast<int>(crop_result_y),
                 static_cast<int>(crop_result_width), static_cast<int>(crop_result_height)});
    }
  }
  return std::unexpected{ErrorType::ErrDeterRes};
}
