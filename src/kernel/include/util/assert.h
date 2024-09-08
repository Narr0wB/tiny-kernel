
#ifndef ASSERT_H
#define ASSERT_H

#include <util/io.h>
#include <util/panic.h>

#define kassert(EX) do { if (!EX) {panic("Assertion failed: "#EX);} } while (0); 

#endif // ASSERT_H
