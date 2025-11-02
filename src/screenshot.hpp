#ifndef SCREENSHOT_HPP_
#define SCREENSHOT_HPP_

#include <expected>
#include <functional>
#include <opencv2/opencv.hpp>
#include <optional>
#include "utils.hpp"

class ScreenShot {
  // the screen device context
  HDC hdcScreen;
  // the memory device context
  HDC hdcMem;

public:
  ScreenShot() {
    hdcScreen = GetDC(NULL);
    hdcMem    = CreateCompatibleDC(hdcScreen);
  }
  ~ScreenShot() {
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
  }
  bool                                  is_ok() { return hdcScreen != NULL && hdcMem != NULL; }
  static std::expected<RECT, ErrorType> get_window_rect(HWND hwnd) {
    RECT rect;  // left, top, right, bottom
    if (!GetWindowRect(hwnd, &rect)) {
      return std::unexpected{ErrorType::ErrGetWindowRect};
    }
    return rect;
  }
  std::expected<HBITMAP, ErrorType> capture_window(HWND hwnd, double scale) {
    RECT rect;
    auto rect_e = get_window_rect(hwnd);
    if (!rect_e.has_value()) {
      return std::unexpected{rect_e.error()};
    } else {
      rect = rect_e.value();
    }

    HBITMAP hbitmap = CreateCompatibleBitmap(hdcScreen, long(scale * (rect.right - rect.left)),
                                             long(scale * (rect.bottom - rect.top)));
    SelectObject(hdcMem, hbitmap);
    if (!PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT)) {
      DeleteObject(hbitmap);
      return std::unexpected{ErrorType::ErrPrintWindow};
    }

    return hbitmap;
  }

  // the caller should call `DeleteObject` to delete the return value
  HBITMAP capture_region(int x1, int y1, int x2, int y2) {
    int c_width  = y2 - y1;
    int c_height = x2 - x1;

    // 创建兼容的位图
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, c_width, c_height);

    // 将屏幕内容复制到内存位图
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, c_width, c_height, hdcScreen, x1, y1, SRCCOPY);

    return hBitmap;
  }

  static cv::Mat HBITMAP_to_Mat(HBITMAP &hBitmap) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int     channels = bmp.bmBitsPixel == 32 ? 4 : 3;
    cv::Mat mat(bmp.bmHeight, bmp.bmWidth, CV_8UC(channels));

    // 获取位图数据
    GetBitmapBits(hBitmap, bmp.bmHeight * bmp.bmWidth * channels, mat.data);

    // 注意：这种方法可能需要调整通道顺序
    /* */
    if (channels == 4) {
      cv::Mat bgr;
      cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);
      return bgr;
    }
    /* */

    return mat;
  }

  double get_screen_scale(bool log) {
    // physical size
    int p_width  = GetDeviceCaps(hdcScreen, DESKTOPHORZRES);
    int p_height = GetDeviceCaps(hdcScreen, DESKTOPVERTRES);

    // scaled size
    int s_width  = GetSystemMetrics(SM_CXSCREEN);
    int s_height = GetSystemMetrics(SM_CYSCREEN);

    double scale = double(p_width) / double(s_width);

    if (log) {
      logln(std::format("physical size\t: {}x{}", p_width, p_height), LogLevel::ALWAYS);
      logln(std::format("scaled size\t: {}x{}", s_width, s_height), LogLevel::ALWAYS);
      logln(std::format("scale\t: {}", scale), LogLevel::ALWAYS);
    }

    return scale;
  }

  std::expected<cv::Mat, ErrorType> capture_window_mat(HWND hwnd, double scale) {
    auto e_hbitmap = capture_window(hwnd, scale);
    if (!e_hbitmap.has_value()) {
      return std::unexpected(e_hbitmap.error());
    }
    HBITMAP hbitmap = *e_hbitmap;
    cv::Mat image   = HBITMAP_to_Mat(hbitmap);
    DeleteObject(hbitmap);
    return image;
  }
};

inline std::function<std::optional<cv::Mat>()> capture_fn_generator(ScreenShot &ss,
                                                                    HWND        hwnd,
                                                                    double      scale,
                                                                    cv::Rect    crop) {
  return [&ss, hwnd, scale, crop]() {
    auto mat_e = ss.capture_window_mat(hwnd, scale);
    if (mat_e.has_value()) {
      auto mat    = mat_e.value();
      auto width  = mat.cols;
      auto height = mat.rows;
      if (width >= crop.x + crop.width && height >= crop.y + crop.height) {
        return std::optional(mat(crop));
      }
    }
    return std::optional<cv::Mat>{};
  };
}

#endif
