/*
 * this header is a pure standard cpp header,
 * when include this, there is no need to use qt framework
 * all other qt related contents locate in 'prog.hpp'
 */
#ifndef PROG_CONFIG_HPP_
#define PROG_CONFIG_HPP_

#include <string>

#include <array>
#include <vector>

namespace prog {
// c++17
namespace env {
using std::string;

inline const string                   data_csv_path{"resource\\csv\\"};
inline const string                   default_data_csv_name{"data.csv"};
inline const string                   opencv_templ_directory{"resource\\template\\"};
inline const string                   opencv_log_directory{"log\\opencv\\"};
inline const string                   capture_window_title{"masterduel"};
inline const string                   config_filename{"config.toml"};
inline const string                   clip_pic_path{"resource\\pic\\iqltv\\"};
inline const std::vector<std::string> clip_pic_name_list{"ufxrziji.gif", "dabmlaziji.png",
                                                         "qihlvidk.gif", "yrzvbcpcyklp.jpeg"};
inline const string                   opencv_templ_config_filename{"templateconfig.toml"};

inline const string default_font_filename{"resource\\font\\SourceHanSansCN-Bold.otf"};
inline const double matcher_threshold{0.9};

namespace config {
inline std::vector<std::string> custom_list_deck{};
inline std::vector<std::string> custom_list_note{};

// this variable is not used by this program
inline std::vector<size_t> stats_tbl_column_width{};
// this variable is not used by this program
inline size_t              stats_tbl_rows_height{};
inline string              stats_tbl_color_background{};
inline string              stats_tbl_color_foreground{};

inline std::array<size_t, 6> record_tbl_column_width{};
inline string                record_tbl_color_coin_win_background{};
inline string                record_tbl_color_coin_win_foreground{};
inline string                record_tbl_color_coin_lose_background{};
inline string                record_tbl_color_coin_lose_foreground{};

inline string record_tbl_color_st_nd_first_background{};
inline string record_tbl_color_st_nd_first_foreground{};
inline string record_tbl_color_st_nd_second_background{};
inline string record_tbl_color_st_nd_second_foreground{};

inline string record_tbl_color_result_victory_background{};
inline string record_tbl_color_result_victory_foreground{};
inline string record_tbl_color_result_defeat_background{};
inline string record_tbl_color_result_defeat_foreground{};
inline string record_tbl_color_result_other_background{};
inline string record_tbl_color_result_other_foreground{};

inline string button_color_start_enabled_background{};
inline string button_color_start_enabled_foreground{};
inline string button_color_start_disabled_background{};
inline string button_color_start_disabled_foreground{};

inline string button_color_stop_enabled_background{};
inline string button_color_stop_enabled_foreground{};
inline string button_color_stop_disabled_background{};
inline string button_color_stop_disabled_foreground{};

inline string button_color_manual0_enabled_background{};
inline string button_color_manual0_enabled_foreground{};
inline string button_color_manual0_disabled_background{};
inline string button_color_manual0_disabled_foreground{};

inline string button_color_manual1_enabled_background{};
inline string button_color_manual1_enabled_foreground{};
inline string button_color_manual1_disabled_background{};
inline string button_color_manual1_disabled_foreground{};

inline size_t misc_prog_window_init_geometry_x{};
inline size_t misc_prog_window_init_geometry_y{};
inline size_t misc_prog_window_init_geometry_width{};
inline size_t misc_prog_window_init_geometry_height{};

inline size_t misc_matcher_sleep_ms{};
inline bool   misc_use_daily_record_csv{};
inline bool   misc_show_clip_success{};
// this variable is not used by this program
inline bool   misc_hide_console{};

inline std::string misc_launch_steam_cmdline{};
inline std::string misc_launch_masterduel_cmdline{};

inline bool debug_test_capture{};

bool load_prog_config();
void reset_prog_config();
}  // namespace config

namespace debug {
inline const bool matcher_text_log{false};
inline const bool matcher_img_log{false};
}  // namespace debug
}  // namespace env
}  // namespace prog

#endif
