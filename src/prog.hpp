#ifndef PROG_HPP_
#define PROG_HPP_

#include "prog_config.hpp"
#include <windows.h>

#include <QStringList>
#include <QVariant>
#include <QFont>

namespace prog
{
    // c++17
    namespace env
    {
        namespace config
        {
            namespace preprocessed
            {
                inline QStringList custom_list_deck{};
                inline QStringList custom_list_note{};
                inline QVariant stats_tbl_color_background{};
                inline QVariant stats_tbl_color_foreground{};

                inline QVariant record_tbl_color_coin_win_background{};
                inline QVariant record_tbl_color_coin_win_foreground{};
                inline QVariant record_tbl_color_coin_lose_background{};
                inline QVariant record_tbl_color_coin_lose_foreground{};

                inline QVariant record_tbl_color_st_nd_first_background{};
                inline QVariant record_tbl_color_st_nd_first_foreground{};
                inline QVariant record_tbl_color_st_nd_second_background{};
                inline QVariant record_tbl_color_st_nd_second_foreground{};

                inline QVariant record_tbl_color_result_victory_background{};
                inline QVariant record_tbl_color_result_victory_foreground{};
                inline QVariant record_tbl_color_result_defeat_background{};
                inline QVariant record_tbl_color_result_defeat_foreground{};
                inline QVariant record_tbl_color_result_other_background{};
                inline QVariant record_tbl_color_result_other_foreground{};

                inline QString button_color_start_enabled_background{};
                inline QString button_color_start_enabled_foreground{};
                inline QString button_color_start_disabled_background{};
                inline QString button_color_start_disabled_foreground{};

                inline QString button_color_stop_enabled_background{};
                inline QString button_color_stop_enabled_foreground{};
                inline QString button_color_stop_disabled_background{};
                inline QString button_color_stop_disabled_foreground{};

                inline QString button_color_manual0_enabled_background{};
                inline QString button_color_manual0_enabled_foreground{};
                inline QString button_color_manual0_disabled_background{};
                inline QString button_color_manual0_disabled_foreground{};

                inline QString button_color_manual1_enabled_background{};
                inline QString button_color_manual1_enabled_foreground{};
                inline QString button_color_manual1_disabled_background{};
                inline QString button_color_manual1_disabled_foreground{};

                bool preprocess();
            }
        }
    }
    namespace global
    {
        inline QFont font{};
    }
}

#endif