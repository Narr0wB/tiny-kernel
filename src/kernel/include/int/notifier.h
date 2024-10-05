
#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <common.h>

#define NOTIFY_DONE     0x0000
#define NOTIFY_OK       0x0001
#define NOTIFY_BAD      0x0002
#define NOTIFY_STOP     0x0003 

struct notifier_block;

typedef int (*notifier_fn_t) (struct notifier_block *nb, uint64_t action, void *data);

struct notifier_block {
    notifier_fn_t notifier_call;
    struct notifier_block *next;
    int irq;
};

void register_notifier_block(struct notifier_block *block);
void unregister_notifier_block(struct notifier_block *block);

#endif // NOTIFIER_H