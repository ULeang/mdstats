#include "../opencv_matcher.hpp"
#include "../screenshot.hpp"
#include "../utils.hpp"

#define DIVISOR 100

template<typename F, typename... Args>
void _t_any(F f, const char *_name, double divisor, Args &&...args) {
  auto __s = std::chrono::steady_clock::now();
  f(std::forward<Args>(args)...);
  auto __e = std::chrono::steady_clock::now();
  logln(std::format("{1} : {0}", std::chrono::duration<double>((__e - __s) / divisor), _name));
}

void _t_capture_window(ScreenShot &ss, HWND hwnd, double scale) {
  for (int i = 0; i < DIVISOR; ++i) {
    auto s_o = ss.capture_window(hwnd, scale);
    if (s_o.has_value()) {
      DeleteObject(s_o.value());
    }
  }
}
void _t_capture_window_mat(ScreenShot &ss, HWND hwnd, double scale) {
  for (int i = 0; i < DIVISOR; ++i) {
    auto s_o = ss.capture_window_mat(hwnd, scale);
  }
}
void _t_capture_window_mat_crop(ScreenShot &ss, HWND hwnd, double scale) {
  auto f = capture_fn_generator(ss, hwnd, scale, {30, 50, 100, 100});
  for (int i = 0; i < DIVISOR; ++i) {
    f();
  }
}
void _t_capture_window_mat_crop_gray(ScreenShot &ss, HWND hwnd, double scale) {
  auto f = capture_fn_generator(ss, hwnd, scale, {30, 50, 100, 100});
  for (int i = 0; i < DIVISOR; ++i) {
    auto    img = f().value();
    cv::Mat img_gray;
    cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  }
}
void _t_templatematch(ScreenShot &ss, HWND hwnd, double scale) {
  auto        crop_result = cv::Rect{880, 450, 840, 600};
  auto        f_result    = capture_fn_generator(ss, hwnd, scale, crop_result);
  std::string path{prog::env::opencv_templ_directory + R"*(2560x1440\)*"};
  Matcher     match_result({(path + "victory.png").c_str(), (path + "defeat.png").c_str()},
                           prog::env::config::misc_opencv_template_match_threshold,
                           prog::env::debug::matcher_img_log, prog::env::debug::matcher_text_log);

  for (int i = 0; i < 100; ++i) {
    auto mr = match_result.try_once(f_result);
  }
}

#define _T_ANY(f, ...) _t_any(f, #f, __VA_ARGS__)

int main() {
  ScreenShot ss;
  assert(ss.is_ok());
  HWND hwnd = FindWindowA(NULL, "qq");
  assert(hwnd);
  double scale = ss.get_screen_scale(true);

#define T_ANY(f) _T_ANY(f, DIVISOR, ss, hwnd, scale);
  T_ANY(_t_capture_window);
  T_ANY(_t_capture_window_mat);
  T_ANY(_t_capture_window_mat_crop);
  T_ANY(_t_capture_window_mat_crop_gray);
  HWND hwnd2 = FindWindowA(NULL, "masterduel");
  assert(hwnd2);
  _T_ANY(_t_templatematch, 100, ss, hwnd2, scale);
  return 0;
}
