#ifndef TOML_READER_HPP_
#define TOML_READER_HPP_

#include <toml.hpp>
#include "evil.h"

template<typename T, typename V, typename... Ks>
concept TOML = requires(T t, V v, V _v, Ks... ks) {
  { toml::find<std::optional<V>>(t, ks...) } -> std::same_as<std::optional<V>>;
  { v = _v };
};

template<typename T, typename V, typename... Ks>
  requires TOML<T, V, Ks...>
inline bool load_value(T &toml, V &value, const Ks &...ks) {
  std::optional<V> v_o = toml::find<std::optional<V>>(toml, ks...);
  if (!v_o.has_value()) {
    return false;
  }
  value = v_o.value();
  return true;
}

#define LOAD(toml, ...) load_value(toml, VARLOOK(__VA_ARGS__))
#define DEFTHENLOAD(type, init, toml, ...) \
  type VARNAME(__VA_ARGS__) = init;        \
  LOAD(toml, __VA_ARGS__);

#endif
