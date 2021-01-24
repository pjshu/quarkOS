//
// Created by pjs on 2021/1/15.
//

#ifndef QUARKOS_QLIB_H
#define QUARKOS_QLIB_H

#include <stddef.h> // size_t and NULL
#include <stdbool.h>
#include <stdarg.h>
#include "stdint.h"

/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

void terminal_initialize(void);

void terminal_setcolor(uint8_t color);

void print_str(const char *data);

void print_u(uint64_t num);

void print_hex(uint64_t x);

void print_pointer(void *p);

void print_char(char data);

// 10 进制转 16 进制,结果存在 str中
void hex(uint64_t n, char *str);


// 整数转字符,结果放在 str中
void q_itoa(uint64_t value, char *str);

#ifdef __i386__
#define assertk(condition) {\
    if (!(condition)) {     \
        printfk("\nassert error: %s: %s: %u\n",__FILE__,__FUNCTION__,__LINE__); \
    }\
}

#else

#include <assert.h>

#define assertk(condition) assert(condition);
#endif


#ifdef __i386__

void printfk(char *__restrict str, ...);

#else

#include <stdio.h>

#define printfk printf

#endif

// 生成掩码
#define BIT_MASK(__type__, n) (sizeof(__type__)*8==n)? \
        ((__type__)-1):\
        (((__type__)1<<n)-1)

#define MASK_U32(n) BIT_MASK(uint32_t, n)

#endif //QUARKOS_QLIB_H