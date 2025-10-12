#ifndef PROG_HPP_
#define PROG_HPP_

#include <string>

namespace prog {
    namespace env {
        using std::string;

        inline const string data_csv_filename{"resource\\csv\\data.csv"};
        inline const string opencv_templ_directory{"resource\\template\\"};
        inline const string opencv_log_directory{"log\\opencv\\"};
        inline const string capture_window_title{"masterduel"};

        inline const string default_font_filename{"resource\\font\\SourceHanSansCN-Bold.otf"};
        
        namespace debug {
            inline bool test_capture_flag{true};
        }
    }
    namespace global {
        inline int qt_font_id;
    }
}

#endif