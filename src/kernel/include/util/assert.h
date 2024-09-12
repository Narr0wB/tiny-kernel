
#ifndef ASSERT_H
#define ASSERT_H

#include <util/io.h>
#include <util/panic.h>

#define kassert(EX) do { if (!(EX)) {panic("Assertion failed: "#EX" in "__FILE__);} } while (0); 

#endif // ASSERT_H
