// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _LIB_PRINT_H
#define _LIB_PRINT_H

#define printkl(fmt, args...) printk(fmt "\n", ##args)
#define tm printk("tvoje mama\n");
#define tmc(int) printk("tvoje mama: %d\n", int);

void printk(const char* format, ...);
void puts(const char* s);

#endif
