#include "../screenshot.hpp"
#include "../utils.hpp"

#define DIVISOR 100

template<typename F, typename... Args>
void _t_any(F f, double divisor, const char *_name, Args &&...args) {
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

int main() {
  ScreenShot ss;
  assert(ss.is_ok());
  HWND hwnd = FindWindowA(NULL, "qq");
  assert(hwnd);
  double scale = ss.get_screen_scale(true);

  _t_any(_t_capture_window, DIVISOR, "_t_capture_window", ss, hwnd, scale);
  _t_any(_t_capture_window_mat, DIVISOR, "_t_capture_window_mat", ss, hwnd, scale);
  _t_any(_t_capture_window_mat_crop, DIVISOR, "_t_capture_window_mat_crop", ss, hwnd, scale);
  return 0;
}
