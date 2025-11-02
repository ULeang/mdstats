#ifndef STATSTABLE_HPP_
#define STATSTABLE_HPP_

#include <string>
#include <vector>

namespace MyModule {
namespace StatsTable {
struct RAII {
  RAII();
  ~RAII();
};

struct EssentialData {
  size_t total       = 0;
  size_t w_st_wins   = 0;
  size_t l_st_wins   = 0;
  size_t w_nd_wins   = 0;
  size_t l_nd_wins   = 0;
  size_t w_st_loses  = 0;
  size_t l_st_loses  = 0;
  size_t w_nd_loses  = 0;
  size_t l_nd_loses  = 0;
  size_t w_st_others = 0;
  size_t l_st_others = 0;
  size_t w_nd_others = 0;
  size_t l_nd_others = 0;
};

template<typename T>
using VV             = std::vector<std::vector<T>>;
using Spans          = VV<size_t>;
using StatsTableText = VV<std::string>;

size_t                rows_count();
size_t                cols_count();
const StatsTableText *stats_table_text();

void reset();
void update_stats_table_text(const EssentialData &data);

const std::vector<size_t> &cols_width();
size_t                     rows_height();
const Spans               &spans();
}  // namespace StatsTable
}  // namespace MyModule

#endif
