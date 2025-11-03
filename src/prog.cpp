#include "utils.hpp"

#include <QColor>

#include <iostream>
#include <toml.hpp>
#include "prog.hpp"
#include "prog_config.hpp"
#include "toml_reader.hpp"

bool prog::env::config::load_prog_config() {
  reset_prog_config();
  auto config_r = toml::try_parse(config_filename);
  if (!config_r.is_ok()) {
    std::cerr << config_r.unwrap_err().at(0) << std::endl;
    logln("load prog config fail, using default config");
    return false;
  }
  auto config = config_r.unwrap();

  LOAD(config, custom_list, deck);
  LOAD(config, custom_list, note);

  LOAD(config, stats_tbl, column_width);
  LOAD(config, stats_tbl, rows_height);
  LOAD(config, stats_tbl, color, background);
  LOAD(config, stats_tbl, color, foreground);

  LOAD(config, record_tbl, column_width);
  LOAD(config, record_tbl, color, coin, win, background);
  LOAD(config, record_tbl, color, coin, win, foreground);
  LOAD(config, record_tbl, color, coin, lose, background);
  LOAD(config, record_tbl, color, coin, lose, foreground);

  LOAD(config, record_tbl, color, st_nd, first, background);
  LOAD(config, record_tbl, color, st_nd, first, foreground);
  LOAD(config, record_tbl, color, st_nd, second, background);
  LOAD(config, record_tbl, color, st_nd, second, foreground);

  LOAD(config, record_tbl, color, result, victory, background);
  LOAD(config, record_tbl, color, result, victory, foreground);
  LOAD(config, record_tbl, color, result, defeat, background);
  LOAD(config, record_tbl, color, result, defeat, foreground);
  LOAD(config, record_tbl, color, result, other, background);
  LOAD(config, record_tbl, color, result, other, foreground);

  LOAD(config, button, color, start, enabled, background);
  LOAD(config, button, color, start, enabled, foreground);
  LOAD(config, button, color, start, disabled, background);
  LOAD(config, button, color, start, disabled, foreground);

  LOAD(config, button, color, stop, enabled, background);
  LOAD(config, button, color, stop, enabled, foreground);
  LOAD(config, button, color, stop, disabled, background);
  LOAD(config, button, color, stop, disabled, foreground);

  LOAD(config, button, color, manual0, enabled, background);
  LOAD(config, button, color, manual0, enabled, foreground);
  LOAD(config, button, color, manual0, disabled, background);
  LOAD(config, button, color, manual0, disabled, foreground);

  LOAD(config, button, color, manual1, enabled, background);
  LOAD(config, button, color, manual1, enabled, foreground);
  LOAD(config, button, color, manual1, disabled, background);
  LOAD(config, button, color, manual1, disabled, foreground);

  LOAD(config, misc, prog_window_init_geometry, x);
  LOAD(config, misc, prog_window_init_geometry, y);
  LOAD(config, misc, prog_window_init_geometry, width);
  LOAD(config, misc, prog_window_init_geometry, height);

  LOAD(config, misc, matcher_sleep_ms);
  LOAD(config, misc, use_daily_record_csv);
  LOAD(config, misc, hide_console);
  LOAD(config, misc, show_clip_success);
  LOAD(config, misc, launch_steam_cmdline);
  LOAD(config, misc, launch_masterduel_cmdline);
  LOAD(config, misc, launch_masterduel_matcher_delay);

  LOAD(config, debug, test_capture);

  return preprocessed::preprocess();
}

void prog::env::config::reset_prog_config() {
  custom_list_deck = {};
  custom_list_note = {};

  stats_tbl_column_width     = {150, 50, 50};
  stats_tbl_rows_height      = {30};
  stats_tbl_color_background = {};
  stats_tbl_color_foreground = {};

  record_tbl_column_width               = {0, 0, 0, 0, 150, 0};
  record_tbl_color_coin_win_background  = {};
  record_tbl_color_coin_win_foreground  = {};
  record_tbl_color_coin_lose_background = {};
  record_tbl_color_coin_lose_foreground = {};

  record_tbl_color_st_nd_first_background  = {};
  record_tbl_color_st_nd_first_foreground  = {};
  record_tbl_color_st_nd_second_background = {};
  record_tbl_color_st_nd_second_foreground = {};

  record_tbl_color_result_victory_background = {};
  record_tbl_color_result_victory_foreground = {};
  record_tbl_color_result_defeat_background  = {};
  record_tbl_color_result_defeat_foreground  = {};
  record_tbl_color_result_other_background   = {};
  record_tbl_color_result_other_foreground   = {};

  button_color_start_enabled_background  = {};
  button_color_start_enabled_foreground  = {};
  button_color_start_disabled_background = {};
  button_color_start_disabled_foreground = {};

  button_color_stop_enabled_background  = {};
  button_color_stop_enabled_foreground  = {};
  button_color_stop_disabled_background = {};
  button_color_stop_disabled_foreground = {};

  button_color_manual0_enabled_background  = {};
  button_color_manual0_enabled_foreground  = {};
  button_color_manual0_disabled_background = {};
  button_color_manual0_disabled_foreground = {};

  button_color_manual1_enabled_background  = {};
  button_color_manual1_enabled_foreground  = {};
  button_color_manual1_disabled_background = {};
  button_color_manual1_disabled_foreground = {};

  misc_prog_window_init_geometry_x      = {800};
  misc_prog_window_init_geometry_y      = {400};
  misc_prog_window_init_geometry_width  = {900};
  misc_prog_window_init_geometry_height = {400};

  misc_matcher_sleep_ms                = {500};
  misc_use_daily_record_csv            = {false};
  misc_hide_console                    = {false};
  misc_show_clip_success               = {true};
  misc_launch_steam_cmdline            = "cmd /c start steam://open/main";
  misc_launch_masterduel_cmdline       = "cmd /c start steam://rungameid/1449850";
  misc_launch_masterduel_matcher_delay = {3000};

  debug_test_capture = {false};
}

bool prog::env::config::preprocessed::preprocess() {
  custom_list_deck.clear();
  custom_list_deck.reserve(prog::env::config::custom_list_deck.size());
  for (const auto &d : prog::env::config::custom_list_deck) {
    custom_list_deck.push_back(d.c_str());
  }

  custom_list_note.clear();
  custom_list_note.reserve(prog::env::config::custom_list_note.size());
  for (const auto &d : prog::env::config::custom_list_note) {
    custom_list_note.push_back(d.c_str());
  }

  QColor color;

#define SETCOLOR(v)                     \
  color = prog::env::config::v.c_str(); \
  v     = color.isValid() ? QVariant{color} : QVariant{};

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

#define SETQSTRING(v) v = prog::env::config::v.c_str()
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
