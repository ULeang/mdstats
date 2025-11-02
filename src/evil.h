// DONT include this header unless you are EVIL
#ifndef EVIL_H_
#define EVIL_H_

#define PP_ARG_X(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, a, b, c, d, e, f, g, h, i, j, k, l, m, n, \
                 o, p, q, r, s, t, u, v, w, x, y, z, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O,  \
                 P, Q, R, S, T, U, V, W, X, Y, Z, XX, ...)                                         \
  XX
#define PP_ARG_N(...)                                                                             \
  PP_ARG_X("ignored", ##__VA_ARGS__, Z, Y, X, W, V, U, T, S, R, Q, P, O, N, M, L, K, J, I, H, G,  \
           F, E, D, C, B, A, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, \
           c, b, a, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define PP_VA_NAME(prefix, ...) PP_CAT2(prefix, PP_ARG_N(__VA_ARGS__))
#define PP_CAT2(a, b)           PP_CAT2_1(a, b)
#define PP_CAT2_1(a, b)         a##b

#define PP_REV_VA1(k1)      k1
#define PP_REV_VA2(k2, ...) PP_REV_VA1(__VA_ARGS__), k2
#define PP_REV_VA3(k3, ...) PP_REV_VA2(__VA_ARGS__), k3
#define PP_REV_VA4(k4, ...) PP_REV_VA3(__VA_ARGS__), k4
#define PP_REV_VA5(k5, ...) PP_REV_VA4(__VA_ARGS__), k5
#define PP_REV_VA6(k6, ...) PP_REV_VA5(__VA_ARGS__), k6

#define PP_REV_VA(...) PP_VA_NAME(PP_REV_VA, __VA_ARGS__)(__VA_ARGS__)

//
#define _VARNAME1(k1)                     k1
#define _VARNAME2(k1, k2)                 k1##_##k2
#define _VARNAME3(k1, k2, k3)             k1##_##k2##_##k3
#define _VARNAME4(k1, k2, k3, k4)         k1##_##k2##_##k3##_##k3
#define _VARNAME5(k1, k2, k3, k4, k5)     k1##_##k2##_##k3##_##k4##_##k5
#define _VARNAME6(k1, k2, k3, k4, k5, k6) k1##_##k2##_##k3##_##k4##_##k5##_##k6

#define VARNAME(...) PP_VA_NAME(_VARNAME, __VA_ARGS__)(__VA_ARGS__)

#define _LOOKUP1(k1)      #k1
#define _LOOKUP2(k1, ...) #k1, _LOOKUP1(__VA_ARGS__)
#define _LOOKUP3(k1, ...) #k1, _LOOKUP2(__VA_ARGS__)
#define _LOOKUP4(k1, ...) #k1, _LOOKUP3(__VA_ARGS__)
#define _LOOKUP5(k1, ...) #k1, _LOOKUP4(__VA_ARGS__)
#define _LOOKUP6(k1, ...) #k1, _LOOKUP5(__VA_ARGS__)

#define LOOKUP(...) PP_VA_NAME(_LOOKUP, __VA_ARGS__)(__VA_ARGS__)

#define VARLOOK(...) VARNAME(__VA_ARGS__), LOOKUP(__VA_ARGS__)

#endif
