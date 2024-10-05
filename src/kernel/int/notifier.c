
#include <int/notifier.h>

struct notifier_block *head = NULL;

void register_notifier_block(struct notifier_block *block) {
    if (!head) {
        head = block;
        return;
    }

    struct notifier_block *current = head;

    while (current->next) {
        current = current->next;
    }
    current->next = block;
}

void unregister_notifier_block(struct notifier_block *block) {
    if (!head) {
        return;
    }

    struct notifier_block *current = head;

    while (current->next && current->next != block) {
        current = current->next;
    }

    if (current->next) {
        current->next = current->next->next;
    }
}