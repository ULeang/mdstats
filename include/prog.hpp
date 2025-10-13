#ifndef PROG_HPP_
#define PROG_HPP_

#include <string>
#include <windows.h>

#include <toml.hpp>
#include <array>

namespace prog
{
    // c++17
    namespace env
    {
        using std::string;

        inline const string data_csv_filename{"resource\\csv\\data.csv"};
        inline const string opencv_templ_directory{"resource\\template\\"};
        inline const string opencv_log_directory{"log\\opencv\\"};
        inline const string capture_window_title{"masterduel"};
        inline const string config_filename{"config.toml"};

        inline const string default_font_filename{"resource\\font\\SourceHanSansCN-Bold.otf"};
        inline const double matcher_threshold{0.9};

        namespace config
        {
            inline string stats_tbl_background_color{"#000000"};
            inline string stats_tbl_foreground_color{"#ffffff"};
            inline std::array<size_t, 4> prog_window_init_x_y_width_height{800, 400, 900, 400};
            inline DWORD matcher_sleep_ms{1 * 1000};

            bool load_prog_config();
        }

        namespace debug
        {
            inline const bool test_capture_flag{false};
            inline const bool matcher_text_log{false};
            inline const bool matcher_img_log{true};
        }
    }
    namespace global
    {
        inline int qt_font_id;
    }
}

#endif