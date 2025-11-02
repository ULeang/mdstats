#include <windows.h>
#include <optional>
#include <toml.hpp>

int main() {
  auto config_r = toml::try_parse("config.toml");
  if (!config_r.is_ok()) {
    ShellExecuteA(0, "open", "mdstats_with_console.exe", 0, 0, SW_SHOWNORMAL);
    return -1;
  }
  auto config = config_r.unwrap();

  auto hide_console = toml::find<std::optional<bool>>(config, "misc", "hide_console");
  if (!hide_console.has_value() || hide_console.value() == true) {
    ShellExecuteA(0, "open", "mdstats_without_console.exe", 0, 0, SW_SHOWNORMAL);
  } else {
    ShellExecuteA(0, "open", "mdstats_with_console.exe", 0, 0, SW_SHOWNORMAL);
  }

  return 0;
}
