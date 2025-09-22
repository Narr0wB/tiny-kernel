
#ifndef KERNEL_H
#define KERNEL_H

#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)((char *)__mptr - offsetof(type,member));})

#define ARRAY_SIZE(x) ((sizeof(x)/sizeof(0[x])))

#endif // KERNEL_H