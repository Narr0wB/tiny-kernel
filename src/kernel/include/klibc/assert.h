
#ifndef ASSERT_H
#define ASSERT_H

#include <klibc/io.h>
#include <panic.h>

#define kassert(EX) do { if (!EX) {panic("Assertion failed: "#EX);} } while (0); 

#endif // ASSERT_H
