#include "../statstable.hpp"
#include <format>
#include <iostream>
#include <vector>
#include "prog_config.hpp"
#include "toml_reader.hpp"
#include "utils.hpp"

namespace MyModule {
namespace StatsTable {
static const size_t        rowc = 6;
static const size_t        colc = 3;
static Spans               _spans;
static StatsTableText      _stats_table_text;
static std::vector<size_t> _cols_width;
static size_t              _rows_height;

RAII::RAII() {
  reset();
}
RAII::~RAII() {}

size_t rows_count() {
  return rowc;
}
size_t cols_count() {
  return colc;
}
const std::vector<size_t> &cols_width() {
  return _cols_width;
}
size_t rows_height() {
  return _rows_height;
}
const Spans &spans() {
  return _spans;
}
const StatsTableText *stats_table_text() {
  return &_stats_table_text;
}

void reset() {
  _spans = {
    {1, 2},
    {1, 1, 1},
    {1, 1, 1},
    {1, 2},
    {1, 2},
    {1, 2}
  };
  _stats_table_text = {
    {"总场次",      "", ""},
    {"硬币(赢/输)", "", ""},
    {"胜负(胜/负)", "", ""},
    {"赢币胜率",    "", ""},
    {"输币胜率",    "", ""},
    {"综合胜率",    "", ""}
  };
  _cols_width  = {150, 50, 50};
  _rows_height = {30};

  auto config_r = toml::try_parse(prog::env::config_filename);
  if (!config_r.is_ok()) {
    std::cerr << config_r.unwrap_err().at(0) << std::endl;
    logln("module statstable : load prog config fail, using default config");
  }
  auto config = config_r.unwrap();

  load_value(config, _cols_width, "stats_tbl", "column_width");
  load_value(config, _rows_height, "stats_tbl", "rows_height");

  update_stats_table_text({});
}
void update_stats_table_text(const EssentialData &data) {
  auto coin_win = data.w_st_wins + data.w_st_loses + data.w_st_others + data.w_nd_wins
                + data.w_nd_loses + data.w_nd_others;
  auto coin_lose = data.l_st_wins + data.l_st_loses + data.l_st_others + data.l_nd_wins
                 + data.l_nd_loses + data.l_nd_others;
  auto res_win           = data.w_st_wins + data.w_nd_wins + data.l_st_wins + data.l_nd_wins;
  auto res_lose          = data.w_st_loses + data.w_nd_loses + data.l_st_loses + data.l_nd_loses;
  auto coin_win_res_win  = data.w_st_wins + data.w_nd_wins;
  auto coin_lose_res_win = data.l_st_wins + data.l_nd_wins;

  _stats_table_text[0][1] = std::to_string(data.total);
  _stats_table_text[1][1] = std::to_string(coin_win);
  _stats_table_text[1][2] = std::to_string(coin_lose);
  _stats_table_text[2][1] = std::to_string(res_win);
  _stats_table_text[2][2] = std::to_string(res_lose);

  _stats_table_text[3][1] =
    coin_win == 0 ? "0" : std::format("{:.2f}%", double(coin_win_res_win) / coin_win * 100);
  _stats_table_text[4][1] =
    coin_lose == 0 ? "0" : std::format("{:.2f}%", double(coin_lose_res_win) / coin_lose * 100);
  _stats_table_text[5][1] =
    data.total == 0 ? "0" : std::format("{:.2f}%", double(res_win) / data.total * 100);
  return;
}
}  // namespace StatsTable
}  // namespace MyModule
