#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <windows.h>
#include <opencv2/opencv.hpp>
#include <functional>
#include <expected>
#include <ctime>
#include <chrono>
#include <sstream>
#include <print>
#include <filesystem>
#include <fstream>

#include <rapidcsv.h>

#include "prog.hpp"

enum class ErrorType
{
    OK = 0,
    ErrDeviceContext,
    ErrFindWindow,
    ErrGetWindowRect,
    ErrDeterRes,
    ErrPrintWindow,
    ErrEnsureCSV,
    ErrCheckResources,
    ErrStopRequested,
};

enum class MatcherFailT
{
    OK = 0,
    CannotGetImage,
    TryNExceed,
    Other,
};

// log

// only logs with level bigger than or equal to current threshold will be logged
enum LogLevel
{
    NEVER = 0,
    SELDOM,
    USUALLY,
    ALWAYS,
};
// c++17
inline LogLevel globl_log_level = LogLevel::USUALLY;

bool log(const std::string &l, LogLevel log_lv = LogLevel::USUALLY);
bool logln(const std::string &l, LogLevel log_lv = LogLevel::USUALLY);

// screenshot

class ScreenShot
{
    // the screen device context
    HDC hdcScreen;
    // the memory device context
    HDC hdcMem;

public:
    ScreenShot()
    {

        hdcScreen = GetDC(NULL);
        hdcMem = CreateCompatibleDC(hdcScreen);
    }
    ~ScreenShot()
    {
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
    }
    bool is_ok()
    {
        return hdcScreen != NULL && hdcMem != NULL;
    }
    std::expected<RECT, ErrorType> get_window_rect(HWND hwnd)
    {
        RECT rect; // left, top, right, bottom
        if (!GetWindowRect(hwnd, &rect))
        {
            return std::unexpected{ErrorType::ErrGetWindowRect};
        }
        return rect;
    }
    std::expected<HBITMAP, ErrorType> capture_window(HWND hwnd, double scale)
    {
        RECT rect;
        auto rect_e = get_window_rect(hwnd);
        if (!rect_e.has_value())
        {
            return std::unexpected{rect_e.error()};
        }
        else
        {
            rect = rect_e.value();
        }

        HBITMAP hbitmap = CreateCompatibleBitmap(hdcScreen,
                                                 long(scale * (rect.right - rect.left)), long(scale * (rect.bottom - rect.top)));
        SelectObject(hdcMem, hbitmap);
        if (!PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT))
        {
            DeleteObject(hbitmap);
            return std::unexpected{ErrorType::ErrPrintWindow};
        }

        return hbitmap;
    }

    // the caller should call `DeleteObject` to delete the return value
    HBITMAP capture_region(int x1, int y1, int x2, int y2)
    {
        int c_width = y2 - y1;
        int c_height = x2 - x1;

        // 创建兼容的位图
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, c_width, c_height);

        // 将屏幕内容复制到内存位图
        SelectObject(hdcMem, hBitmap);
        BitBlt(hdcMem, 0, 0, c_width, c_height, hdcScreen, x1, y1, SRCCOPY);

        return hBitmap;
    }

    static cv::Mat HBITMAP_to_Mat(HBITMAP &hBitmap)
    {
        BITMAP bmp;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        int channels = bmp.bmBitsPixel == 32 ? 4 : 3;
        cv::Mat mat(bmp.bmHeight, bmp.bmWidth, CV_8UC(channels));

        // 获取位图数据
        GetBitmapBits(hBitmap, bmp.bmHeight * bmp.bmWidth * channels, mat.data);

        // 注意：这种方法可能需要调整通道顺序
        /* */
        if (channels == 4)
        {
            cv::Mat bgr;
            cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);
            return bgr;
        }
        /* */

        return mat;
    }

    double get_screen_scale(bool log)
    {
        // physical size
        int p_width = GetDeviceCaps(hdcScreen, DESKTOPHORZRES);
        int p_height = GetDeviceCaps(hdcScreen, DESKTOPVERTRES);

        // scaled size
        int s_width = GetSystemMetrics(SM_CXSCREEN);
        int s_height = GetSystemMetrics(SM_CYSCREEN);

        double scale = double(p_width) / double(s_width);

        if (log)
        {
            logln(std::format("physical size\t: {}x{}", p_width, p_height), LogLevel::ALWAYS);
            logln(std::format("scaled size\t: {}x{}", s_width, s_height), LogLevel::ALWAYS);
            logln(std::format("scale\t: {}", scale), LogLevel::ALWAYS);
        }

        return scale;
    }

    std::expected<cv::Mat, ErrorType> capture_window_mat(HWND hwnd, double scale)
    {
        auto e_hbitmap = capture_window(hwnd, scale);
        if (!e_hbitmap.has_value())
        {
            return std::unexpected(e_hbitmap.error());
        }
        HBITMAP hbitmap = *e_hbitmap;
        cv::Mat image = HBITMAP_to_Mat(hbitmap);
        DeleteObject(hbitmap);
        return image;
    }
};

std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss, HWND hwnd, double scale);
std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss, HWND hwnd, double scale, cv::Rect crop);

std::tm *get_local_time();

// csv
bool ensure_csv(std::filesystem::path csv);
bool append_to_csv(const std::filesystem::path &csv, const std::string &record);
std::string format_csv_record(size_t no, size_t coin, size_t st_nd, size_t result,
                              const std::string &deck, const std::tm *t);

bool check_resources(const std::vector<std::filesystem::path> &files);

// Matcher
template <int CV_FORMAT, int CV_MATCH_METHOD>
class T_Matcher;

template <>
class T_Matcher<CV_8UC3, cv::TM_CCOEFF_NORMED>
{
    std::vector<cv::Mat> templ;
    std::ofstream flog;

public:
    const size_t size;
    double threshold;
    bool log;
    bool text_log;

    explicit T_Matcher(std::initializer_list<const char *> img_filepath,
                       double threshold = 0.9, bool log = false, bool text_log = true)
        : size(img_filepath.size()),
          threshold(threshold),
          log(log),
          text_log(text_log),
          flog("log.txt", std::ios::app)
    {
        templ.resize(size);
        size_t i = 0;
        std::ranges::for_each(img_filepath.begin(), img_filepath.end(),
                              [this, &i](const char *p)
                              {
                                  templ[i] = cv::imread(p, cv::IMREAD_COLOR);
                                  ++i;
                              });
    }

    std::expected<size_t, MatcherFailT> try_once(const std::function<std::optional<cv::Mat>()> &f) const
    {
        auto finput = f();
        if (!finput.has_value())
        {
            logln("cannot get image");
            return std::unexpected(MatcherFailT::CannotGetImage);
        }
        cv::Mat image = finput.value();

        std::vector<std::tuple<size_t, double, cv::Point>> all_res;

        for (size_t i = 0; i < size; ++i)
        {
            cv::Mat result;
            result.create(image.rows - templ[i].rows + 1, image.cols - templ[i].cols + 1, CV_8UC3);

            cv::matchTemplate(image, templ[i], result, cv::TM_CCOEFF_NORMED);

            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

#ifndef NDEBUG
            logln(std::format("maxVal:{:5.2f}", 100 * maxVal), LogLevel::SELDOM);
#endif
            if (text_log)
            {
                flog << std::format("maxVal:{:5.2f}\n", 100 * maxVal);
            }
            if (maxVal > threshold)
            {
#ifndef NDEBUG
                logln(std::format("x:{},y:{}", maxLoc.x, maxLoc.y), LogLevel::SELDOM);
#endif
                all_res.push_back({i, maxVal, maxLoc});
            }
        }
        if (all_res.empty())
        {
            return std::unexpected(MatcherFailT::TryNExceed);
        }

        auto [c_i, c_maxVal, c_maxLoc] = *std::max_element(all_res.begin(), all_res.end(),
                                                           [](const auto &_a, const auto &_b)
                                                           { return std::get<1>(_a) < std::get<1>(_b); });
        if (log)
        {
            cv::rectangle(image, c_maxLoc,
                          cv::Point(c_maxLoc.x + templ[c_i].cols, c_maxLoc.y + templ[c_i].rows),
                          cv::Scalar(0, 0, 255), 4);

            auto local_time = get_local_time();

            // the filename may not contain colon ':', otherwise imwrite fails
            cv::imwrite(std::format("{}\\pic\\{:04}{:02}{:02}-{:02}{:02}{:02}-{:.5f}.png",
                                    prog::env::opencv_log_directory,
                                    local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
                                    local_time->tm_hour, local_time->tm_min, local_time->tm_sec, c_maxVal)
                            .c_str(),
                        image);
        }
        return c_i;
    }

    std::expected<size_t, MatcherFailT> keep_try(const std::function<std::optional<cv::Mat>()> &f,
                                                 DWORD sleep = 1 * 1000) const
    {
        while (true)
        {
            auto res = try_once(f);
            if (res.has_value())
            {
                return res.value();
            }
            if (res.error() == MatcherFailT::TryNExceed)
            {
                Sleep(sleep);
                continue;
            }
            return res;
        }
    }

    std::expected<size_t, MatcherFailT> try_n(const std::function<std::optional<cv::Mat>()> &f,
                                              size_t n, DWORD sleep = 1 * 1000) const
    {
        for (size_t i = 0; i < n; ++i)
        {
            auto res = try_once(f);
            if (res.has_value())
            {
                return res.value();
            }
            if (res.error() == MatcherFailT::TryNExceed)
            {
                Sleep(sleep);
                continue;
            }
            return res;
        }
        return std::unexpected(MatcherFailT::TryNExceed);
    }
};

using Matcher = T_Matcher<CV_8UC3, cv::TM_CCOEFF_NORMED>;

// csv data
struct DataBase
{
    using Data = std::vector<std::string>;
    size_t rowc;
    Data coin_col;
    Data st_nd_col;
    Data result_col;
    Data deck_col;
    Data time_col;

    size_t w_st_wins;
    size_t l_st_wins;
    size_t w_nd_wins;
    size_t l_nd_wins;
    size_t w_st_loses;
    size_t l_st_loses;
    size_t w_nd_loses;
    size_t l_nd_loses;
    size_t w_st_others;
    size_t l_st_others;
    size_t w_nd_others;
    size_t l_nd_others;

    DataBase(const std::string &csv_filename);
    DataBase();
    void append_record(std::string coin, std::string st_nd, std::string result,
                       std::string deck, std::string t);
    size_t trunc_last(size_t n = 1);
    void load_csv(const std::string &csv_filename);

private:
    void update_stats_by(bool inc, const std::string &coin, const std::string &st_nd, const std::string &result);
    void clear_stats();
};

// clipboard
void CopyToClipboard(const char *text);

#endif