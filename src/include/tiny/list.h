
#ifndef LIST_H
#define LIST_H

#include <tiny/compiler.h>
#include <tiny/kernel.h>

/* 
 * Circular doubly-linked list, similarly defined as in the list.h header in the Linux Kernel.
 * Some functionality might be missing, but that is to be expected.
 */

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_NODE_INIT(node) { &(node), &(node) }

static __force_inline void list_head_init(struct list_head *head)
{
    *head = (struct list_head)LIST_NODE_INIT(*head);
}

static __force_inline bool _list_add_valid(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    if (next->prev != prev ||
        prev->next != next)
        return false;
    
    return true;
}

static __force_inline void _list_add(struct list_head *new, struct list_head *prev, struct list_head *next) 
{
    if (!_list_add_valid(new, prev, next))
        return; 

    prev->next = new;
    new->prev = prev;
    new->next = next;
    WRITE_ONCE(next->prev, new);
}

static __force_inline void _list_del(struct list_head *prev, struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static __force_inline struct list_head *_list_last(struct list_head *head)
{
    return head->prev;
}

/*
 * Insert a new node after the head of the list
 * @new: the new node to insert
 * @head: the head of the list
 */
static __force_inline void list_add(struct list_head *new, struct list_head *head)
{
    _list_add(new, head, head->next);
}

/*
 * Insert a new node at the tail of the list 
 * @new: new node to insert
 * @head: the head of the list 
 */
static __force_inline void list_add_tail(struct list_head *new, struct list_head *head) 
{
    _list_add(new, head->prev, head);
}

/*
 * Delete a node from the list 
 * @node: the node to delete 
 */
static __force_inline void list_del(struct list_head *node) 
{
    _list_del(node->prev, node->next);
}

/*
 * Replace a node from the list 
 * @old: the node to replace
 * @new: the new node to replace the old with
 */
static __force_inline void list_replace(struct list_head *old, struct list_head *new)
{
    old->prev->next = new;
    old->next->prev = new;

    new->prev = old->prev;
    new->next = old->next;
}

static __force_inline bool list_empty(struct list_head *head)
{
    return head == head->next;
}

static __force_inline struct list_head *_list_take_last(struct list_head *head)
{
    struct list_head *last = _list_last(head);
    list_del(last);
    return last;
}

/*
 * Get the containing structure of the given list entry 
 * @ptr: pointer to the struct list_head in the container 
 * @type: type of the container (eg. struct dev, struct idt_info, ...) 
 * @member: name of the list entry in the struct (eg. list, list_node, ...)
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/*
 * Get the containing structure of the first list entry 
 * @head: pointer to the head of the list 
 * @type: type of the container (eg. struct dev, struct idt_info, ...) 
 * @member: name of the list entry in the struct (eg. list, list_node, ...)
 */
#define list_first_entry(head, type, member) \
    list_entry((head)->next, type, member)

#define list_next_entry(entry, member) \
    list_entry((entry)->member.next, typeof(*(entry)), member)

#define list_take_last(head, type, member) \
    container_of(_list_take_last(head), type, member)

#define list_foreach_entry(head, pos, member) \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_next_entry(pos, member))

#endif // LIST_H 