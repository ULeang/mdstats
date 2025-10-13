#ifndef PROG_HPP_
#define PROG_HPP_

#include <string>
#include <windows.h>

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

        inline const string default_font_filename{"resource\\font\\SourceHanSansCN-Bold.otf"};
        inline const double matcher_threshold{0.9};
        inline const DWORD matcher_sleep_ms{1 * 1000};

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