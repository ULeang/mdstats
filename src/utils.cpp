#include "utils.hpp"
#include "evil.h"

#include <QColor>

#include <print>
#include <fstream>
#include <toml.hpp>

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
void CopyToClipboard(const wchar_t *text)
{
    // 打开剪贴板
    if (!OpenClipboard(nullptr))
        return;

    // 清空剪贴板
    EmptyClipboard();

    // 为文本分配全局内存
    size_t size = wcslen(text) + 1;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size * sizeof(wchar_t));
    if (!hGlobal)
    {
        CloseClipboard();
        return;
    }

    // 将文本复制到全局内存
    wchar_t *pGlobal = static_cast<wchar_t *>(GlobalLock(hGlobal));
    wcsncpy(pGlobal, text, size);
    GlobalUnlock(hGlobal);

    // 设置剪贴板数据
    SetClipboardData(CF_UNICODETEXT, hGlobal);

    // 关闭剪贴板
    CloseClipboard();
}

template <typename T, typename V, typename... Ks>
concept TOML = requires(T t, V v, V _v, Ks... ks) {
    { toml::find<std::optional<V>>(t, ks...) } -> std::same_as<std::optional<V>>;
    { v = _v };
};

template <typename T, typename V, typename... Ks>
    requires TOML<T, V, Ks...>
static bool load_value(T &toml, V &value, const Ks &...ks)
{
    std::optional<V> v_o = toml::find<std::optional<V>>(toml, ks...);
    if (!v_o.has_value())
    {
        return false;
    }
    value = v_o.value();
    return true;
}
bool prog::env::config::load_prog_config()
{
    reset_prog_config();
    auto config_r = toml::try_parse(config_filename);
    if (!config_r.is_ok())
    {
        std::cerr << config_r.unwrap_err().at(0) << std::endl;
        logln("load prog config fail, using default config");
        return false;
    }
    auto config = config_r.unwrap();

#define LOAD(...) \
    load_value(config, VARLOOK(__VA_ARGS__))

    LOAD(custom_list, deck);
    LOAD(custom_list, note);

    LOAD(stats_tbl, column_width);
    LOAD(stats_tbl, rows_height);
    LOAD(stats_tbl, color, background);
    LOAD(stats_tbl, color, foreground);

    LOAD(record_tbl, column_width);
    LOAD(record_tbl, color, coin, win, background);
    LOAD(record_tbl, color, coin, win, foreground);
    LOAD(record_tbl, color, coin, lose, background);
    LOAD(record_tbl, color, coin, lose, foreground);

    LOAD(record_tbl, color, st_nd, first, background);
    LOAD(record_tbl, color, st_nd, first, foreground);
    LOAD(record_tbl, color, st_nd, second, background);
    LOAD(record_tbl, color, st_nd, second, foreground);

    LOAD(record_tbl, color, result, victory, background);
    LOAD(record_tbl, color, result, victory, foreground);
    LOAD(record_tbl, color, result, defeat, background);
    LOAD(record_tbl, color, result, defeat, foreground);
    LOAD(record_tbl, color, result, other, background);
    LOAD(record_tbl, color, result, other, foreground);

    LOAD(button, color, start, enabled, background);
    LOAD(button, color, start, enabled, foreground);
    LOAD(button, color, start, disabled, background);
    LOAD(button, color, start, disabled, foreground);

    LOAD(button, color, stop, enabled, background);
    LOAD(button, color, stop, enabled, foreground);
    LOAD(button, color, stop, disabled, background);
    LOAD(button, color, stop, disabled, foreground);

    LOAD(button, color, manual0, enabled, background);
    LOAD(button, color, manual0, enabled, foreground);
    LOAD(button, color, manual0, disabled, background);
    LOAD(button, color, manual0, disabled, foreground);

    LOAD(button, color, manual1, enabled, background);
    LOAD(button, color, manual1, enabled, foreground);
    LOAD(button, color, manual1, disabled, background);
    LOAD(button, color, manual1, disabled, foreground);

    LOAD(misc, prog_window_init_geometry, x);
    LOAD(misc, prog_window_init_geometry, y);
    LOAD(misc, prog_window_init_geometry, width);
    LOAD(misc, prog_window_init_geometry, height);

    LOAD(misc, matcher_sleep_ms);
    LOAD(misc, use_daily_record_csv);
    LOAD(misc, hide_console);
    LOAD(misc, show_clip_success);

    return preprocessed::preprocess();
}

void prog::env::config::reset_prog_config()
{
    custom_list_deck = {};
    custom_list_note = {};

    stats_tbl_column_width = {150, 50, 50};
    stats_tbl_rows_height = {0};
    stats_tbl_color_background = {};
    stats_tbl_color_foreground = {};

    record_tbl_column_width = {0, 0, 0, 0, 150, 0};
    record_tbl_color_coin_win_background = {};
    record_tbl_color_coin_win_foreground = {};
    record_tbl_color_coin_lose_background = {};
    record_tbl_color_coin_lose_foreground = {};

    record_tbl_color_st_nd_first_background = {};
    record_tbl_color_st_nd_first_foreground = {};
    record_tbl_color_st_nd_second_background = {};
    record_tbl_color_st_nd_second_foreground = {};

    record_tbl_color_result_victory_background = {};
    record_tbl_color_result_victory_foreground = {};
    record_tbl_color_result_defeat_background = {};
    record_tbl_color_result_defeat_foreground = {};
    record_tbl_color_result_other_background = {};
    record_tbl_color_result_other_foreground = {};

    button_color_start_enabled_background = {};
    button_color_start_enabled_foreground = {};
    button_color_start_disabled_background = {};
    button_color_start_disabled_foreground = {};

    button_color_stop_enabled_background = {};
    button_color_stop_enabled_foreground = {};
    button_color_stop_disabled_background = {};
    button_color_stop_disabled_foreground = {};

    button_color_manual0_enabled_background = {};
    button_color_manual0_enabled_foreground = {};
    button_color_manual0_disabled_background = {};
    button_color_manual0_disabled_foreground = {};

    button_color_manual1_enabled_background = {};
    button_color_manual1_enabled_foreground = {};
    button_color_manual1_disabled_background = {};
    button_color_manual1_disabled_foreground = {};

    misc_prog_window_init_geometry_x = {800};
    misc_prog_window_init_geometry_y = {400};
    misc_prog_window_init_geometry_width = {900};
    misc_prog_window_init_geometry_height = {400};

    misc_matcher_sleep_ms = {500};
    misc_use_daily_record_csv = {false};
    misc_hide_console = {true};
    misc_show_clip_success = {false};
}

bool prog::env::config::preprocessed::preprocess()
{
    custom_list_deck.clear();
    custom_list_deck.reserve(prog::env::config::custom_list_deck.size());
    for (const auto &d : prog::env::config::custom_list_deck)
    {
        custom_list_deck.push_back(d.c_str());
    }

    custom_list_note.clear();
    custom_list_note.reserve(prog::env::config::custom_list_note.size());
    for (const auto &d : prog::env::config::custom_list_note)
    {
        custom_list_note.push_back(d.c_str());
    }

    QColor color;

#define SETCOLOR(v)                       \
    color = prog::env::config::v.c_str(); \
    v = color.isValid() ? QVariant{color} : QVariant{};

    SETCOLOR(stats_tbl_color_background);
    SETCOLOR(stats_tbl_color_foreground);

    SETCOLOR(record_tbl_color_coin_win_background);
    SETCOLOR(record_tbl_color_coin_win_foreground);
    SETCOLOR(record_tbl_color_coin_lose_background);
    SETCOLOR(record_tbl_color_coin_lose_foreground);

    SETCOLOR(record_tbl_color_st_nd_first_background);
    SETCOLOR(record_tbl_color_st_nd_first_foreground);
    SETCOLOR(record_tbl_color_st_nd_second_background);
    SETCOLOR(record_tbl_color_st_nd_second_foreground);

    SETCOLOR(record_tbl_color_result_victory_background);
    SETCOLOR(record_tbl_color_result_victory_foreground);
    SETCOLOR(record_tbl_color_result_defeat_background);
    SETCOLOR(record_tbl_color_result_defeat_foreground);
    SETCOLOR(record_tbl_color_result_other_background);
    SETCOLOR(record_tbl_color_result_other_foreground);

#define SETQSTRING(v) \
    v = prog::env::config::v.c_str()
    SETQSTRING(button_color_start_enabled_background);
    SETQSTRING(button_color_start_enabled_foreground);
    SETQSTRING(button_color_start_disabled_background);
    SETQSTRING(button_color_start_disabled_foreground);

    SETQSTRING(button_color_stop_enabled_background);
    SETQSTRING(button_color_stop_enabled_foreground);
    SETQSTRING(button_color_stop_disabled_background);
    SETQSTRING(button_color_stop_disabled_foreground);

    SETQSTRING(button_color_manual0_enabled_background);
    SETQSTRING(button_color_manual0_enabled_foreground);
    SETQSTRING(button_color_manual0_disabled_background);
    SETQSTRING(button_color_manual0_disabled_foreground);

    SETQSTRING(button_color_manual1_enabled_background);
    SETQSTRING(button_color_manual1_enabled_foreground);
    SETQSTRING(button_color_manual1_disabled_background);
    SETQSTRING(button_color_manual1_disabled_foreground);

    return true;
}