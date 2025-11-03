#include "../statstable.hpp"
#include <format>
#include <iostream>
#include <vector>
#include "eval.hpp"
#include "prog_config.hpp"
#include "toml_reader.hpp"
#include "utils.hpp"

namespace MyModule {
namespace StatsTable {
static size_t              _rowc;
static size_t              _colc;
static Spans               _spans;
static StatsTableText      _stats_table_text;
static std::vector<size_t> _cols_width;
static size_t              _rows_height;

class Cell;
using pCell = std::shared_ptr<Cell>;
using Cells = std::vector<std::vector<pCell>>;
static Cells _cells;

class Cell {
protected:
  size_t _span;

public:
  Cell(size_t span) : _span(span) {}
  virtual std::string display(const EssentialData &data, AST::Env &env) const = 0;
  size_t              span() const { return _span; }
};

class DefaultCell : public Cell {
public:
  DefaultCell(size_t span = 1) : Cell(span) {}
  std::string display(const EssentialData &data, AST::Env &env) const override { return {}; }
};
class TextCell : public Cell {
  std::string _text;

public:
  TextCell(size_t span, std::string text) : Cell(span), _text(text) {}
  std::string display(const EssentialData &data, AST::Env &env) const override { return _text; }
};
class ExprCell : public Cell {
  using pAST = PARSER::pAST;
  bool                       valid;
  pAST                       _expr;
  std::string                _format;
  std::optional<pAST>        _opt;
  std::optional<pAST>        _expr_opt;
  std::optional<std::string> _format_opt;

public:
  ExprCell(size_t span, std::string expr, std::string format) : Cell(span), _format(format) {
    auto pAST_o = Run(PARSER::Single_expression(expr));
    if (!pAST_o.has_value()) {
      valid = false;
      logln(std::format("module statstable : invalid expr '{}'", expr));
    } else {
      valid = true;
      _expr = pAST_o.value();
    }
  }
  ExprCell(size_t      span,
           std::string expr,
           std::string format,
           std::string opt,
           std::string expr_opt,
           std::string format_opt)
    : ExprCell(span, expr, format) {
    auto _opt_ast_o = Run(PARSER::Single_expression(opt));
    if (!_opt_ast_o.has_value()) {
      valid = false;
      logln(std::format("module statstable : invalid expr '{}'", opt));
      return;
    } else {
      _opt = _opt_ast_o.value();
    }
    auto _expr_opt_ast_o = Run(PARSER::Single_expression(expr_opt));
    if (!_expr_opt_ast_o.has_value()) {
      valid = false;
      logln(std::format("module statstable : invalid expr '{}'", expr_opt));
      return;
    } else {
      _expr_opt = _expr_opt_ast_o.value();
    }
    _format_opt = format_opt;
  }
  std::string display(const EssentialData &data, AST::Env &env) const override {
    if (!valid) {
      return "E:INVLAID";
    }
    bool use_opt = false;
    if (_opt.has_value()) {
      auto opt_r = _opt.value()->eval(env);
      if (opt_r.index() == 2) {
        logln("module statstable : opt eval failed");
        return "E:OPT";
      }
      if (opt_r.index() == 0) {
        if (std::get<int64_t>(opt_r) != 0) use_opt = true;
      } else {
        if (std::get<double>(opt_r) != double(0)) use_opt = true;
      }
    }
    const auto &used_expr   = use_opt ? _expr_opt.value() : _expr;
    const auto &used_format = use_opt ? _format_opt.value() : _format;
    auto        expr_r      = used_expr->eval(env);
    try {
      if (expr_r.index() == 0) {
        return std::format(std::runtime_format(used_format), std::get<int64_t>(expr_r));
      } else if (expr_r.index() == 1) {
        return std::format(std::runtime_format(used_format), std::get<double>(expr_r));
      } else {
        logln("module statstable : expr eval failed");
        return "E:EXPR";
      }
    } catch (...) {
      logln("module statstable : format error");
      return "E:FORMAT";
    }
  }
};
class PresetCell : public Cell {
  enum Preset {
    NOPRESET,
    TOTAL,
    WIN,
    LOSE,
    COINWIN,
    COINLOSE,
    COINWINWINRATE,
    COINLOSEWINRATE,
    TOTALWINRATE,
  } _preset;
#define SETLOOKUP(_enum) \
  {                      \
    #_enum, _enum        \
  }
  const std::unordered_map<std::string, Preset> lookup = {
    SETLOOKUP(TOTAL),           SETLOOKUP(WIN),          SETLOOKUP(LOSE),
    SETLOOKUP(COINWIN),         SETLOOKUP(COINLOSE),     SETLOOKUP(COINWINWINRATE),
    SETLOOKUP(COINLOSEWINRATE), SETLOOKUP(TOTALWINRATE),
  };

public:
  PresetCell(size_t span, std::string preset) : Cell(span) {
    try {
      _preset = lookup.at(preset);
    } catch (...) {
      _preset = NOPRESET;
    }
  }
  std::string display(const EssentialData &data, AST::Env &env) const override {
    switch (_preset) {
      case TOTAL: return std::to_string(data.total);
      case WIN:   {
        auto res_win = data.w_st_wins + data.w_nd_wins + data.l_st_wins + data.l_nd_wins;
        return std::to_string(res_win);
      }
      case LOSE: {
        auto res_lose = data.w_st_loses + data.w_nd_loses + data.l_st_loses + data.l_nd_loses;
        return std::to_string(res_lose);
      }
      case COINWIN: {
        auto coin_win = data.w_st_wins + data.w_st_loses + data.w_st_others + data.w_nd_wins
                      + data.w_nd_loses + data.w_nd_others;
        return std::to_string(coin_win);
      }
      case COINLOSE: {
        auto coin_lose = data.l_st_wins + data.l_st_loses + data.l_st_others + data.l_nd_wins
                       + data.l_nd_loses + data.l_nd_others;
        return std::to_string(coin_lose);
      }
      case COINWINWINRATE: {
        auto coin_win = data.w_st_wins + data.w_st_loses + data.w_st_others + data.w_nd_wins
                      + data.w_nd_loses + data.w_nd_others;
        auto coin_win_res_win = data.w_st_wins + data.w_nd_wins;
        return coin_win == 0 ? "0"
                             : std::format("{:.2f}%", double(coin_win_res_win) / coin_win * 100);
      }
      case COINLOSEWINRATE: {
        auto coin_lose = data.l_st_wins + data.l_st_loses + data.l_st_others + data.l_nd_wins
                       + data.l_nd_loses + data.l_nd_others;
        auto coin_lose_res_win = data.l_st_wins + data.l_nd_wins;
        return coin_lose == 0 ? "0"
                              : std::format("{:.2f}%", double(coin_lose_res_win) / coin_lose * 100);
      }
      case TOTALWINRATE: {
        auto res_win = data.w_st_wins + data.w_nd_wins + data.l_st_wins + data.l_nd_wins;
        return data.total == 0 ? "0" : std::format("{:.2f}%", double(res_win) / data.total * 100);
      }
      default: return "NOPRESET";
    }
  }
};

RAII::RAII() {
  reset();
}
RAII::~RAII() {}

size_t rows_count() {
  return _rowc;
}
size_t cols_count() {
  return _colc;
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

static const Cells default_cells = {
  {std::make_shared<TextCell>(1, "总场次"), std::make_shared<PresetCell>(2, "TOTAL")},
  {std::make_shared<TextCell>(1, "硬币(赢/输)"), std::make_shared<PresetCell>(1, "COINWIN"),
   std::make_shared<PresetCell>(1, "COINLOSE")},
  {std::make_shared<TextCell>(1, "胜负(胜/负)"), std::make_shared<PresetCell>(1, "WIN"),
   std::make_shared<PresetCell>(1, "LOSE")},
  {std::make_shared<TextCell>(1, "赢币胜率"), std::make_shared<PresetCell>(2, "COINWINWINRATE")},
  {std::make_shared<TextCell>(1, "输币胜率"), std::make_shared<PresetCell>(2, "COINLOSEWINRATE")},
  {std::make_shared<TextCell>(1, "综合胜率"), std::make_shared<PresetCell>(2, "TOTALWINRATE")},
};
static std::tuple<size_t, size_t, Cells> load_cells(const toml::value &toml) {
  size_t                            default_row_count    = 6;
  size_t                            default_column_count = 3;
  std::tuple<size_t, size_t, Cells> default_ret{default_row_count, default_column_count,
                                                default_cells};

  bool use_custom_rows = false;
  load_value(toml, use_custom_rows, "stats_tbl", "use_custom_rows");
  if (!use_custom_rows) {
    return default_ret;
  }

  Cells ret_cells{};

#define find_from_toml(_name, _type, _toml, _log, _callback)        \
  auto _name##_o = toml::find<std::optional<_type>>(_toml, #_name); \
  if (!_name##_o.has_value()) {                                     \
    logln("module statstable : load " #_log " fail");               \
    _callback;                                                      \
  }                                                                 \
  const auto &_name = _name##_o.value();

  auto custom_rows_o =
    toml::find<std::optional<std::vector<toml::value>>>(toml, "stats_tbl", "custom_rows");
  if (!custom_rows_o.has_value()) {
    logln("module statstable : load custom_rows fail, using default config");
    return default_ret;
  }
  const auto &custom_rows = custom_rows_o.value();

  for (const auto &custom_row : custom_rows) {
    find_from_toml(cells, std::vector<toml::value>, custom_row, custom_rows.cells, {
      ret_cells.push_back({});
      continue;
    });

    ret_cells.push_back({});
    for (const auto &cell : cells) {
      auto   span_o = toml::find<std::optional<size_t>>(cell, "span");
      size_t span   = span_o.has_value() ? span_o.value() : 1;

      find_from_toml(type, std::string, cell, custom_rows.cells.type, {
        ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
        continue;
      });

      if (type == "Text") {
        find_from_toml(text, std::string, cell, custom_rows.cells.text, {
          ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
          continue;
        });
        ret_cells.back().push_back(std::make_shared<TextCell>(span, text));
      } else if (type == "Expr") {
        find_from_toml(expr, std::string, cell, custom_rows.cells.expr, {
          ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
          continue;
        });
        auto        format_o = toml::find<std::optional<std::string>>(cell, "format");
        std::string format   = format_o.has_value() ? format_o.value() : "{}";

        auto opt_o = toml::find<std::optional<std::string>>(cell, "opt");
        if (!opt_o.has_value()) {
          ret_cells.back().push_back(std::make_shared<ExprCell>(span, expr, format));
        } else {
          const auto &opt = opt_o.value();
          find_from_toml(expr_opt, std::string, cell, custom_rows.cells.expr_opt, {
            ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
            continue;
          });
          find_from_toml(format_opt, std::string, cell, custom_rows.cells.format_opt, {
            ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
            continue;
          });
          ret_cells.back().push_back(
            std::make_shared<ExprCell>(span, expr, format, opt, expr_opt, format_opt));
        }
      } else if (type == "Preset") {
        find_from_toml(preset, std::string, cell, custom_rows.cells.preset, {
          ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
          continue;
        });
        ret_cells.back().push_back(std::make_shared<PresetCell>(span, preset));
      } else {
        logln(std::format("module statstable : unknown type '{}'", type));
        ret_cells.back().push_back(std::make_shared<DefaultCell>(span));
        continue;
      }
    }
  }

  // calculate proper row_count and column_count
  size_t ret_row_count    = 0;
  size_t ret_column_count = 0;
  ret_row_count           = ret_cells.size();
  for (const auto &row_cells : ret_cells) {
    size_t cur_row_column_count = 0;
    for (const auto &row_cell : row_cells) {
      cur_row_column_count += row_cell->span();
    }
    ret_column_count = std::max(ret_column_count, cur_row_column_count);
  }
  return std::tuple<size_t, size_t, Cells>{ret_row_count, ret_column_count, ret_cells};
}
static void calculate_span_text() {
  _spans.clear();
  _spans.reserve(_cells.size());
  for (const auto &row_cells : _cells) {
    _spans.push_back({});
    for (const auto &cell : row_cells) {
      _spans.back().push_back(cell->span());
    }
  }
  _stats_table_text.clear();
  _stats_table_text.reserve(_rowc);
  for (size_t r = 0; r < _rowc; ++r) {
    _stats_table_text.push_back(std::vector<std::string>(_colc));
  }
}
static void default_config() {
  _cells = default_cells;
  _rowc  = 6;
  _colc  = 3;
  calculate_span_text();
}
void reset() {
  _cols_width  = {150, 50, 50};
  _rows_height = 30;

  auto config_r = toml::try_parse(prog::env::config_filename);
  if (!config_r.is_ok()) {
    std::cerr << config_r.unwrap_err().at(0) << std::endl;
    logln("module statstable : load prog config fail, using default config");
    default_config();
    return;
  }
  auto config = config_r.unwrap();

  auto _load_cells = load_cells(config);
  _rowc            = std::get<0>(_load_cells);
  _colc            = std::get<1>(_load_cells);
  _cells           = std::get<2>(_load_cells);

  calculate_span_text();

  load_value(config, _cols_width, "stats_tbl", "column_width");
  _cols_width.resize(_colc, 50);

  load_value(config, _rows_height, "stats_tbl", "rows_height");

  update_stats_table_text({});
}
static AST::Env env;
void            update_stats_table_text(const EssentialData &data) {
  env["TOTAL"]   = AST::Value(int64_t(data.total));
  env["WFW"]     = AST::Value(int64_t(data.w_st_wins));
  env["WFL"]     = AST::Value(int64_t(data.w_st_loses));
  env["WFO"]     = AST::Value(int64_t(data.w_st_others));
  env["WSW"]     = AST::Value(int64_t(data.w_nd_wins));
  env["WSL"]     = AST::Value(int64_t(data.w_nd_loses));
  env["WSO"]     = AST::Value(int64_t(data.w_nd_others));
  env["LFW"]     = AST::Value(int64_t(data.l_st_wins));
  env["LFL"]     = AST::Value(int64_t(data.l_st_loses));
  env["LFO"]     = AST::Value(int64_t(data.l_st_others));
  env["LSW"]     = AST::Value(int64_t(data.l_nd_wins));
  env["LSL"]     = AST::Value(int64_t(data.l_nd_loses));
  env["LSO"]     = AST::Value(int64_t(data.l_nd_others));
  env["COINWIN"] = AST::Value(int64_t(data.w_st_wins + data.w_st_loses + data.w_st_others
                                                 + data.w_nd_wins + data.w_nd_loses + data.w_nd_others));
  env["COINLOSE"] = AST::Value(int64_t(data.l_st_wins + data.l_st_loses + data.l_st_others
                                                  + data.l_nd_wins + data.l_nd_loses + data.l_nd_others));
  env["WIN"] =
    AST::Value(int64_t(data.w_st_wins + data.w_nd_wins + data.l_st_wins + data.l_nd_wins));
  env["LOSE"] =
    AST::Value(int64_t(data.w_st_loses + data.w_nd_loses + data.l_st_loses + data.l_nd_loses));
  env["OTHER"] =
    AST::Value(int64_t(data.w_st_others + data.w_nd_others + data.l_st_others + data.l_nd_others));
  env["FIRST"]  = AST::Value(int64_t(data.w_st_wins + data.w_st_loses + data.w_st_others
                                                + data.l_st_wins + data.l_st_loses + data.l_st_others));
  env["SECOND"] = AST::Value(int64_t(data.w_nd_wins + data.w_nd_loses + data.w_nd_others
                                                + data.l_nd_wins + data.l_nd_loses + data.l_nd_others));
  for (size_t r = 0; r < _rowc; ++r) {
    size_t actual_col = 0;
    for (const auto &cell : _cells[r]) {
      _stats_table_text[r][actual_col]  = cell->display(data, env);
      actual_col                       += cell->span();
    }
  }
}
}  // namespace StatsTable
}  // namespace MyModule
