// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <debug.h>
#include <drivers/printer.h>
#include <lib/print.h>
#include <lib/stdarg.h>
#include <types.h>

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

    printer_putchar(c);
}

static void putint_dec(int i) {
    unsigned int next;

    if (i < 0) {
        printer_putchar('-');
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
                printer_putchar(c); \
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
        printer_putchar(*s);
        s++;
    }
}

static void putpointer(void* p) {
    if (p == 0) {
        putstring("(nil)");
        return;
    }

    printer_putchar('0');
    printer_putchar('x');

    puthex((size_t)p, false, 1);
}

static void print_linked_list(link_t* link, link_t* head, int length) {
    assert(link != NULL);
    length++;

    if (link->prev == head) {
        putint_dec(length);
        putstring(": ");
        putpointer(link);
    } else {
        print_linked_list(link->prev, head, length);
        printer_putchar('-');
        putpointer(link);
    }

    return;
}

static void putlist(list_t* list) {
    assert(list != NULL);
    putpointer((void*)list);

    if (list->head.next == &list->head) {
        putstring("[empty]");
        return;
    }

    printer_putchar('[');

    print_linked_list(list->head.prev, &list->head, 0);

    printer_putchar(']');

    return;
}

static void putfunction(void* address) {
    putpointer(address);
    printer_putchar('[');

    int instr_count = 4;

    uint32_t* it = (uint32_t*)address;
    for (int i = 0; i < instr_count; i++) {
        if (i != 0) {
            printer_putchar(' ');
        }
        puthex(*it, false, 8);
        it++;
    }

    printer_putchar(']');
    return;
}

static void putuint_hex_whole(uint32_t i) {
    puthex(i, false, (int)sizeof(uint32_t) * 2);
}

/** Prints given formatted string to console.
 *
 * @param format printf-style formatting string.
 */
void printk(const char* format, ...) {
    va_list argp;
    va_start(argp, format);

    const char* it = format;
    while (*it != '\0') {
        if (*it != '%') {
            printer_putchar(*it);
            it++;
            continue;
        }

        it++; // eat '%'
        switch (*it) {
        case 'c':
            printer_putchar(va_arg(argp, int));
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
            it++;

            switch (*it) {
            case 'L':
                putlist(va_arg(argp, list_t*));
                break;
            case 'F':
                putfunction(va_arg(argp, void*));
                break;
            default:
                it--;
                putpointer(va_arg(argp, void*));
                break;
            }

            break;
        case '%':
            printer_putchar('%');
            break;
        default:
            printer_putchar('%');
            printer_putchar(*it);
            break;
        }
        it++;
    }

    va_end(argp);
}

/** Prints given string to console, terminating it with newline.
 *
 * @param s String to print.
 */
void puts(const char* s) {
    while (*s != '\0') {
        printer_putchar(*s);
        s++;
    }
    printer_putchar('\n');
}
