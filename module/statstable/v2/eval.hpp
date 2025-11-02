#ifndef EVAL_HPP_
#define EVAL_HPP_

#include <parser_.h>

#include <stdint.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include "utils.hpp"

class AST {
public:
  enum DataType {
    integral,
    floating,
  };
  enum NodeType {
    number,
    id,
    add,
    sub,
    mul,
    div,
    branch,
    def,
    gt,
    lt,
    ge,
    le,
    eq,
    ne,
  };
  using pAST  = std::shared_ptr<AST>;
  using Data  = std::variant<int64_t,
                             std::string,
                             std::tuple<pAST, pAST, pAST>,
                             std::tuple<std::string, pAST, pAST>>;
  using Value = std::variant<int64_t, double, char>;
  using Env   = std::unordered_map<std::string, Value>;

  DataType datatype;
  NodeType nodetype;
  Data     data;
  AST(DataType _datatype = integral, NodeType _nodetype = number, Data _data = 0)
    : datatype(_datatype), nodetype(_nodetype), data(_data) {}

  Value eval(Env &env) const {
    if (datatype == integral) {
      return _get<int64_t>(_eval<int64_t>(env));
    } else if (datatype == floating) {
      return _get<double>(_eval<double>(env));
    } else {
      logln("AST : unknown datatype");
      return '-';
    }
  }

  template<typename... Args>
  static pAST make(Args &&...args) {
    return std::make_shared<AST>(AST::integral, std::forward<Args>(args)...);
  }
  template<typename... Args>
  static pAST make_floating(Args &&...args) {
    return std::make_shared<AST>(AST::floating, std::forward<Args>(args)...);
  }
  // this function just simply transform the boxed type to T, i.e. Value<anytype except char type>
  // -> Value<T>
  template<typename T>
  static Value _get(Value raw) {
    if (raw.index() == 0) {
      return Value(T(std::get<int64_t>(raw)));
    } else if (raw.index() == 1) {
      return Value(T(std::get<double>(raw)));
    } else {
      return raw;
    }
  }

private:
  template<typename T>
  Value map2(Env &env, std::function<T(T, T)> fn) const {
    auto [opl, opr, _] = std::get<std::tuple<pAST, pAST, pAST>>(data);
    auto vl            = opl->_eval<T>(env);
    if (vl.index() == 2) return '-';
    auto vr = opr->_eval<T>(env);
    if (vr.index() == 2) return '-';
    return fn(std::get<T>(vl), std::get<T>(vr));
  }
  template<typename T>
  Value _eval(Env &env) const {
    switch (nodetype) {
      case number: return T(std::get<int64_t>(data));
      case id:
        try {
          auto v = env.at(std::get<std::string>(data));
          return _get<T>(v);
        } catch (...) {
          logln(std::format("AST : cannot find id '{}'", std::get<std::string>(data)));
          return '-';
        }
      case add: return map2<T>(env, [](T vl, T vr) { return vl + vr; });
      case sub: return map2<T>(env, [](T vl, T vr) { return vl - vr; });
      case mul: return map2<T>(env, [](T vl, T vr) { return vl * vr; });
      case div: {
        if constexpr (std::is_same_v<T, double>) {
          return map2<T>(env, [](T vl, T vr) { return vl / vr; });
        } else {
          auto [opl, opr, _] = std::get<std::tuple<pAST, pAST, pAST>>(data);
          auto vl            = opl->_eval<T>(env);
          if (vl.index() == 2) return '-';
          auto vr = opr->_eval<T>(env);
          if (vr.index() == 2) return '-';
          auto vlv = std::get<T>(vl);
          auto vrv = std::get<T>(vr);
          if (vrv == 0) return '-';
          return vlv / vrv;
        }
      }
      case branch: {
        auto [predicate, iftrue, iffalse] = std::get<std::tuple<pAST, pAST, pAST>>(data);
        auto vp                           = predicate->_eval<T>(env);
        if ((vp.index() == 0 && std::get<int64_t>(vp) == 0)
            || (vp.index() == 1 && std::get<double>(vp) == 0)) {
          return iffalse->_eval<T>(env);
        } else if (vp.index() == 0 || vp.index() == 1) {
          return iftrue->_eval<T>(env);
        } else {
          return '-';
        }
      }
      case def: {
        auto [id, assign, tocal] = std::get<std::tuple<std::string, pAST, pAST>>(data);
        if (env.contains(id)) {
          logln(std::format("AST : id '{}' already exists", id));
          return '-';
        }
        auto va            = assign->_eval<T>(env);
        auto [i, inserted] = env.insert({id, va});
        auto ret           = tocal->_eval<T>(env);
        env.erase(id);
        return ret;
      }
      case gt: return map2<T>(env, [](T vl, T vr) { return vl > vr ? T(1) : T(0); });
      case lt: return map2<T>(env, [](T vl, T vr) { return vl < vr ? T(1) : T(0); });
      case ge: return map2<T>(env, [](T vl, T vr) { return vl >= vr ? T(1) : T(0); });
      case le: return map2<T>(env, [](T vl, T vr) { return vl <= vr ? T(1) : T(0); });
      case eq: return map2<T>(env, [](T vl, T vr) { return vl == vr ? T(1) : T(0); });
      case ne: return map2<T>(env, [](T vl, T vr) { return vl != vr ? T(1) : T(0); });
      default: logln("AST : unknown nodetype"); return '-';
    }
  }
};

namespace PARSER {
template<typename T>
ParserT<T> filter(std::function<bool(const T &)> f, const ParserT<T> &p) {
  return [p, f](std::string_view input) {
    auto p_r = p(input);
    if (!p_r.has_value()) return p_r;
    const auto &[p_v, rest] = p_r.value();
    if (!f(p_v)) {
      return std::optional<std::tuple<T, std::string_view>>();
    }
    return p_r;
  };
}

inline std::string keywords[]   = {"if", "then", "else", "let", "in"};
inline auto        Leading_id   = Alpha | Char('_');
inline auto        Following_id = Leading_id | DigitC;
inline auto        Id = filter(std::function<bool(const std::string &)>([](const std::string &s) {
                          for (const auto &k : keywords) {
                            if (s == k) return false;
                          }
                          return true;
                        }),
                               fmap<std::tuple<char, Many<char>>, std::string>(
                          [](std::tuple<char, Many<char>> arg) {
                            std::string ret;
                            ret.reserve(1 + std::get<1>(arg).size());
                            ret.push_back(std::get<0>(arg));
                            for (auto c : std::get<1>(arg)) {
                              ret.push_back(c);
                            }
                            return ret;
                          },
                          Leading_id &many(Following_id)));

inline auto char2Int64 = [](char c) { return int64_t(c - '0'); };

inline auto Digit64 = fmap<char, int64_t>(char2Int64, DigitC);
inline auto mkInt64 = [](Many<int64_t> ints) {
  int64_t rec = 0;
  for (int64_t i : ints) {
    rec = (10 * rec) + i;
  }
  return rec;
};

inline auto Natural64 = fmap<Many<int64_t>, int64_t>(mkInt64, many1(Digit64));

inline auto Integer64 = Char('-')
                       >> fmap<int64_t, int64_t>([](int64_t i) { return int64_t(-1) * i; },
                                                 Natural64)
                      | Natural64;
inline auto Number = Integer64;

using pAST = std::shared_ptr<AST>;

inline auto Id_AST =
  fmap<std::string, pAST>([](std::string t) { return AST::make(AST::id, t); }, Id);
inline auto Number_AST =
  fmap<int64_t, pAST>([](int64_t t) { return AST::make(AST::number, t); }, Number);

#define DEFPARSER(name, type)                      \
  ParserRet<type> _##name(std::string_view input); \
  inline auto     name = std::function(_##name);

// we need recursive parser, this is significant, a forward declaration of a function and a function
// wrapper a forward declaration of a variable would not work, because combinators capture their
// parameters parser by value at the time we call a combinator, the variable is still a invalid
// parser result in bad function call
DEFPARSER(Expression, pAST)

inline auto Spaces  = TakeWhile([](char c) { return isspace(c); });
inline auto Spaces1 = Take1While([](char c) { return isspace(c); });

inline auto Parenthetical_Expression = Char('(') >> Spaces >> Expression << Spaces << Char(')');

#define DEFBINARY(name, leading, nodetype)                                                      \
  inline auto name       = (leading >> Spaces >> Expression) & (Spaces >> Expression);          \
  inline auto name##_AST = fmap<std::tuple<pAST, pAST>, pAST>(                                  \
    [](std::tuple<pAST, pAST> t) {                                                              \
      return AST::make(nodetype, std::make_tuple(std::get<0>(t), std::get<1>(t), AST::make())); \
    },                                                                                          \
    name);

DEFBINARY(Add, Char('+'), AST::add)
DEFBINARY(Sub, Char('-'), AST::sub)
DEFBINARY(Mul, Char('*'), AST::mul)
DEFBINARY(Div, Char('/'), AST::div)
DEFBINARY(Gt, Char('>'), AST::gt)
DEFBINARY(Lt, Char('<'), AST::lt)
DEFBINARY(Ge, Lit(">="), AST::ge)
DEFBINARY(Le, Lit("<="), AST::le)
DEFBINARY(Eq, Lit("=="), AST::eq)
DEFBINARY(Ne, Lit("!="), AST::ne)

inline auto Branch = (Lit("if") >> Spaces1 >> Expression)
                   & (Spaces1 >> Lit("then") >> Spaces1 >> Expression)
                   & (Spaces1 >> Lit("else") >> Spaces1 >> Expression);
inline auto Branch_AST = fmap<std::tuple<std::tuple<pAST, pAST>, pAST>, pAST>(
  [](std::tuple<std::tuple<pAST, pAST>, pAST> t) {
    return AST::make(AST::branch, std::make_tuple(std::get<0>(std::get<0>(t)),
                                                  std::get<1>(std::get<0>(t)), std::get<1>(t)));
  },
  Branch);

inline auto Def = (Lit("let") >> Spaces1 >> Id) & (Spaces >> Char('=') >> Spaces >> Expression)
                & (Spaces1 >> Lit("in") >> Spaces1 >> Expression);
inline auto Def_AST = fmap<std::tuple<std::tuple<std::string, pAST>, pAST>, pAST>(
  [](std::tuple<std::tuple<std::string, pAST>, pAST> t) {
    return AST::make(AST::def, std::make_tuple(std::get<0>(std::get<0>(t)),
                                               std::get<1>(std::get<0>(t)), std::get<1>(t)));
  },
  Def);

inline ParserT<char> Eof = [](std::string_view input) {
  if (input.empty()) {
    return std::optional(std::make_tuple(' ', input));
  }
  return std::optional<std::tuple<char, std::string_view>>{};
};

inline ParserRet<pAST> _Expression(std::string_view input) {
  auto __Expression = Parenthetical_Expression | Add_AST | Sub_AST | Mul_AST | Div_AST | Gt_AST
                    | Lt_AST | Ge_AST | Le_AST | Eq_AST | Ne_AST | Branch_AST | Def_AST | Id_AST
                    | Number_AST;
  return __Expression(input);
}

inline auto Type                = Char('%') >> (Char('i') | Char('f'));
inline auto Type_expression     = Type & (Spaces1 >> Expression);
inline auto Type_expression_AST = fmap<std::tuple<char, pAST>, pAST>(
  [](std::tuple<char, pAST> t) {
    auto type  = std::get<0>(t);
    auto p_ast = std::get<1>(t);
    if (type == 'f') {
      p_ast->datatype = AST::floating;
    } else {
      p_ast->datatype = AST::integral;
    }
    return p_ast;
  },
  Type_expression);
inline auto Single_expression = Spaces >> Type_expression_AST << Spaces << Eof;
}  // namespace PARSER

#endif
