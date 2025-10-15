#include "utils.hpp"

#include <print>
#include <fstream>

std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss, HWND hwnd, double scale)
{
    return [&ss, hwnd, scale]()
    {
        auto mat = ss.capture_window_mat(hwnd, scale);
        return mat.has_value() ? std::optional<cv::Mat>(mat.value())
                               : std::optional<cv::Mat>{};
    };
}
std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss, HWND hwnd, double scale, cv::Rect crop)
{
    return [&ss, hwnd, scale, crop]()
    {
        auto mat = ss.capture_window_mat(hwnd, scale);
        return mat.has_value() ? std::optional<cv::Mat>(mat.value()(crop))
                               : std::optional<cv::Mat>{};
    };
}

std::tm *get_local_time()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time);
    return local_time;
}

bool check_resources(const std::vector<std::filesystem::path> &files)
{
    for (const auto &f : files)
    {
        if (!std::filesystem::exists(f))
        {
            return false;
        }
    }
    return true;
}

bool log(const std::string &l, LogLevel log_lv)
{
    if (log_lv >= prog::global::log_level)
    {
        std::clog << l << std::flush;
        return true;
    }
    return false;
}
bool logln(const std::string &l, LogLevel log_lv)
{
    if (log_lv >= prog::global::log_level)
    {
        std::clog << l << std::endl;
        return true;
    }
    return false;
}
void CopyToClipboard(const char *text)
{
    // 打开剪贴板
    if (!OpenClipboard(nullptr))
        return;

    // 清空剪贴板
    EmptyClipboard();

    // 为文本分配全局内存
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size_needed * sizeof(wchar_t));
    if (!hGlobal)
    {
        CloseClipboard();
        return;
    }

    // 将文本复制到全局内存
    wchar_t *pGlobal = static_cast<wchar_t *>(GlobalLock(hGlobal));
    MultiByteToWideChar(CP_UTF8, 0, text, -1, pGlobal, size_needed);
    GlobalUnlock(hGlobal);

    // 设置剪贴板数据
    SetClipboardData(CF_UNICODETEXT, hGlobal);

    // 关闭剪贴板
    CloseClipboard();
}

template <typename T, typename V>
concept TOML = requires(T t) {
    { toml::find<std::optional<V>>(t, "") } -> std::same_as<std::optional<V>>;
};

template <typename V, TOML<V> T>
static bool load_value_by_name(T &toml, V &value, const char *name)
{
    std::optional<V> v_o = toml::find<std::optional<V>>(toml, name);
    if (!v_o.has_value())
    {
        return false;
    }
    value = v_o.value();
    return true;
}
bool prog::env::config::load_prog_config()
{
    auto config_r = toml::try_parse(config_filename);
    if (!config_r.is_ok())
    {
        std::cerr << config_r.unwrap_err().at(0) << std::endl;
        logln("load prog config fail, using default config");
        return false;
    }

    auto config = config_r.unwrap();

#define LOAD(var) \
    load_value_by_name(config, var, #var)

    LOAD(stats_tbl_background_color);
    LOAD(stats_tbl_foreground_color);
    LOAD(prog_window_init_x_y_width_height);
    LOAD(matcher_sleep_ms);
    LOAD(use_daily_record_csv);
    LOAD(stats_tbl_column_width);
    LOAD(record_tbl_column_width);
    LOAD(custom_deck_list);
    LOAD(custom_note_list);
    LOAD(hide_console);

    return true;
}