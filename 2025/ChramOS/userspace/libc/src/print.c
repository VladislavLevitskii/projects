// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <np/syscall.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

size_t print_count;

/** Print single character to console.
 *
 * @param c Character to print.
 * @return Character written as an unsigned char cast to an int.
 */
int putchar(int c) {
    __SYSCALL1(SYSCALL_PUTCHAR, c);
    print_count++;
    return (unsigned char)c;
}

/** Prints the given string to console, terminating it with a newline.
 *
 * @param str String to print.
 * @return Number of printed characters.
 */
int puts(const char* str) {
    int count = 1; // For new-line
    while (*str != 0) {
        putchar(*str);
        str++;
        count++;
    }
    putchar('\n');
    return count;
}

static char get_next_decimal_char(unsigned long i) {
    return '0' + i % 10;
}

static unsigned long get_next_ulong(unsigned long i) {
    return i / 10;
}

static void putuint_dec(unsigned int i) {
    char c = get_next_decimal_char(i);
    i = (unsigned int)get_next_ulong(i);

    if (i != 0)
        putuint_dec(i);

    putchar(c);
}

static void putint_dec(int i) {
    unsigned int next;

    if (i < 0) {
        putchar('-');
        next = ~(unsigned int)(i) + 1;
    } else {
        next = (unsigned int)i;
    }

    putuint_dec(next);
}

#define puthex(n, is_capital, min_length) \
    { \
        const char* characters = is_capital ? "0123456789ABCDEF" : "0123456789abcdef"; \
        bool leading_zero = true; \
        for (int nibble = sizeof(n) * 2 - 1; nibble >= 0; nibble--) { \
            char c = characters[(n >> (nibble * 4)) & 0xF]; \
            if (c != '0') \
                leading_zero = false; \
            if (nibble < min_length || !leading_zero) \
                putchar(c); \
        } \
    }

static void putuint_hex(unsigned int i) {
    puthex(i, false, 1);
}

static void putuint_hex_capital(unsigned int i) {
    puthex(i, true, 1);
}

static void putstring(char* s) {
    if (s == NULL) {
        putstring("(null)");
        return;
    }

    while (*s != '\0') {
        putchar(*s);
        s++;
    }
}

static void putpointer(void* p) {
    if (p == 0) {
        putstring("(nil)");
        return;
    }

    putchar('0');
    putchar('x');

    puthex((size_t)p, false, 1);
}

static void putuint_hex_whole(uint32_t i) {
    puthex(i, false, (int)sizeof(uint32_t) * 2);
}

/**
 * Prints the given formatted string to console.
 *
 * Supported format specifiers:
 *  <code>%c</code> - character,
 *  <code>%d</code> - signed decimal integer,
 *  <code>%x</code> - unsigned hexadecimal integer (lowercase),
 *  <code>%X</code> - unsigned hexadecimal integer (uppercase),
 *  <code>%y</code> - unsigned hexadecimal integer (lowercase, full 32 bits),
 *  <code>%s</code> - string,
 *  <code>%u</code> - unsigned decimal integer,
 *  <code>%p</code> - pointer,
 *  <code>%%</code> - percent sign
 *
 * @param format printf-style formatting string
 * @return Number of printed characters
 */
int printf(const char* format, ...) {
    va_list argp;
    va_start(argp, format);
    print_count = 0;

    const char* it = format;
    while (*it != '\0') {
        if (*it != '%') {
            putchar(*it);
            it++;
            continue;
        }

        it++; // eat '%'
        switch (*it) {
        case 'c':
            putchar(va_arg(argp, int));
            break;
        case 'd':
            putint_dec(va_arg(argp, int));
            break;
        case 'x':
            putuint_hex(va_arg(argp, unsigned int));
            break;
        case 'X':
            putuint_hex_capital(va_arg(argp, unsigned int));
            break;
        // whole 32 bits in hex
        case 'y':
            putuint_hex_whole(va_arg(argp, uint32_t));
            break;
        case 's':
            putstring(va_arg(argp, char*));
            break;
        case 'u':
            putuint_dec(va_arg(argp, unsigned int));
            break;
        case 'p':
            putpointer(va_arg(argp, void*));
            break;
        case '%':
            putchar('%');
            break;
        default:
            putchar('%');
            putchar(*it);
            break;
        }
        it++;
    }

    va_end(argp);
    return print_count;
}
