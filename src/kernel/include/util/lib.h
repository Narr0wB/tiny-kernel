
#ifndef STDLIB_H
#define STDLIB_H

#include <common.h>

#define get_bit(v, n) ((v >> n) & 1)
#define set_bit(v, n) ((v) |= (1 << n))
#define clr_bit(v, n) ((v) &= ~(1 << n))

#endif // STDLIB_H
