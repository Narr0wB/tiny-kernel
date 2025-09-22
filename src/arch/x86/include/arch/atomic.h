
#ifndef ARCH_X86_ATOMIC_H
#define ARCH_X86_ATOMIC_H

#include <tiny/types.h>
#include <tiny/compiler.h>

#define LOCK_PREFIX "lock "

typedef struct {
    int32_t counter;
} atomic_t;

static __force_inline int32_t atomic_get(const atomic_t *a) {
    return (*(volatile int32_t *)&(a)->counter);
}

static __force_inline void atomic_set(atomic_t *a, int32_t v) {
    *(volatile int32_t *)&(a)->counter = v;
}

static __force_inline void atomic_inc(atomic_t *a) {
    __asm__ volatile (
        LOCK_PREFIX "incl %0"
        : "+m" (a->counter)
    );
}

static __force_inline void atomic_dec(atomic_t *a) {
    __asm__ volatile (
        LOCK_PREFIX "decl %0"
        : "+m" (a->counter)
    );
}

static __force_inline void atomic_add(atomic_t *a, int32_t addend) {
    __asm__ volatile (
        LOCK_PREFIX "add %1, %0"
        : "+m" (a->counter) 
        : "ir" (addend)
    );
}

static __force_inline void atomic_sub(atomic_t *a, int32_t addend) {
    __asm__ volatile (
        LOCK_PREFIX "subl %1, %0"
        : "+m" (a->counter) 
        : "ir" (addend)
    );
}

static __force_inline int32_t atomic_xadd(atomic_t *a, int32_t addend) {
    __asm__ volatile (
        LOCK_PREFIX "xaddl %1, %0"
        : "+m" (a->counter), "+r" (addend)
    );

    return addend;
}

static __force_inline int32_t atomic_xsub(atomic_t *a, int32_t addend) {
    __asm__ volatile (
        LOCK_PREFIX "xsubl %1, %0"
        : "+m" (a->counter), "+r" (addend)
    );

    return addend;
}

#endif // ARCH_X86_ATOMIC_H