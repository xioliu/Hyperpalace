#ifndef _STDINT_H
#define _STDINT_H

/* 精确宽度整数类型 */
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

/* 指针整数类型 */
typedef long long          intptr_t;
typedef unsigned long long uintptr_t;

/* 最小宽度类型 */
typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;

typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

/* 快速宽度类型 */
typedef int                int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;

typedef unsigned int       uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

/* 最大值宏 */
#define INT8_MAX   (127)
#define INT16_MAX  (32767)
#define INT32_MAX  (2147483647)
#define INT64_MAX  (9223372036854775807LL)

#define UINT8_MAX  (255U)
#define UINT16_MAX (65535U)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (18446744073709551615ULL)

/* 其他常用定义 */
#define SIZE_MAX   UINT64_MAX
#define NULL       ((void *)0)

#endif /* _STDINT_H */