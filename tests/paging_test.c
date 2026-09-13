/* Run: cc -ffreestanding -ffunction-sections -fdata-sections -Isrc/include -Isrc/arch/include tests/paging_test.c -Wl,--gc-sections -o /tmp/paging_test && /tmp/paging_test */
#include <assert.h>
#include "../src/arch/x86/mm/paging.c"

static pgd_t root;
static struct page_table pool[16] __attribute__((aligned(4096)));
static unsigned int used, limit;

static int allocate(struct vm_pt_page *pg, void *ctx)
{
    assert(ctx == NULL);
    if (used == limit)
        return -ENOMEM;
    pg->virt = &pool[used++];
    pg->phys = v_to_p(pg->virt);
    return 0;
}

static void reset(void)
{
    memset(&root, 0, sizeof(root));
    memset(pool, 0xa5, sizeof(pool));
    used = 0;
    limit = 16;
}

static u64 lookup(vaddr_t va, int *leaf_shift)
{
    struct page_table *table = &root;
    for (int shift = 39; shift >= 12; shift -= 9) {
        u64 entry = table->entry[(va >> shift) & 511];
        if (!(entry & PAGE_FLAG_PRESENT))
            return 0;
        if (shift == 12 || (entry & PAGE_FLAG_PAGESIZE)) {
            *leaf_shift = shift;
            return entry;
        }
        assert(entry & PAGE_FLAG_READWRITE);
        assert(entry & PAGE_FLAG_USER);
        table = (struct page_table *)p_to_v(entry & PHYS_ADDR_MASK);
    }
    assert(0);
    return 0;
}

int main(void)
{
    int shift;
    u64 rw = PAGE_FLAG_READWRITE | PAGE_FLAG_USER;
    reset();
    size_t mixed = PAGE_SIZE_1G + PAGE_SIZE_2M + PAGE_SIZE_4K;
    assert(vm_map(&root, 0, 0, mixed, rw, allocate) == 0);
    assert(lookup(0, &shift) == (rw | PAGE_FLAG_PRESENT | PAGE_FLAG_PAGESIZE) && shift == 30);
    assert((lookup(PAGE_SIZE_1G, &shift) & PHYS_ADDR_MASK) == PAGE_SIZE_1G && shift == 21);
    assert((lookup(mixed - PAGE_SIZE, &shift) & PHYS_ADDR_MASK) == mixed - PAGE_SIZE && shift == 12);
    assert(!lookup(mixed, &shift));
    assert(vm_map(&root, 0, 0, PAGE_SIZE, rw, allocate) == -EEXIST);

    reset();
    assert(vm_map(&root, PAGE_SIZE, 0, PAGE_SIZE_2M, rw, allocate) == 0);
    assert((lookup(PAGE_SIZE_2M - PAGE_SIZE, &shift) & PHYS_ADDR_MASK) == PAGE_SIZE_2M && shift == 12);
    assert(!lookup(PAGE_SIZE_2M, &shift));

    reset();
    vaddr_t start = PAGE_SIZE_2M - PAGE_SIZE;
    assert(vm_map(&root, start, start, PAGE_SIZE_2M + 2 * PAGE_SIZE,
                  rw | PAGE_FLAG_GLOBAL | PAGE_FLAG_PAGESIZE, allocate) == 0);
    assert((lookup(start, &shift) & PAGE_FLAG_GLOBAL) && shift == 12);
    assert(!(lookup(start, &shift) & PAGE_FLAG_PAGESIZE));
    assert(lookup(PAGE_SIZE_2M, &shift) && shift == 21);
    assert(lookup(2 * PAGE_SIZE_2M, &shift) && shift == 12);

    reset();
    vaddr_t boundary = 1ULL << 39;
    assert(vm_map(&root, 0, boundary - PAGE_SIZE, 2 * PAGE_SIZE, 0, allocate) == 0);
    assert(lookup(boundary - PAGE_SIZE, &shift) == PAGE_FLAG_PRESENT && shift == 12);
    assert(lookup(boundary, &shift) == (PAGE_SIZE | PAGE_FLAG_PRESENT));

    reset();
    limit = 3;
    assert(vm_map(&root, 0, PAGE_SIZE_2M - PAGE_SIZE, 2 * PAGE_SIZE, rw, allocate) == -ENOMEM);
    assert(!lookup(PAGE_SIZE_2M - PAGE_SIZE, &shift));
    limit = 16;
    assert(vm_map(&root, 0, PAGE_SIZE_2M - PAGE_SIZE, 2 * PAGE_SIZE, rw, allocate) == 0);

    reset();
    assert(vm_map(&root, 0, PAGE_SIZE, PAGE_SIZE, rw, allocate) == 0);
    assert(vm_map(&root, 0, 0, 2 * PAGE_SIZE, rw, allocate) == -EEXIST);
    assert(!lookup(0, &shift));
    assert(lookup(PAGE_SIZE, &shift));

    root.entry[0] &= ~PAGE_FLAG_USER;
    assert(vm_map(&root, 0, 0, PAGE_SIZE, rw, allocate) == -EINVAL);
    root.entry[0] |= PAGE_FLAG_USER | (1ULL << 63);
    assert(vm_map(&root, 0, 0, PAGE_SIZE, rw, allocate) == -EINVAL);

    reset();
    assert(vm_map(&root, 1, 0, PAGE_SIZE, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 0, 1, PAGE_SIZE, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 0, 0, 1, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 0, 1ULL << 47, PAGE_SIZE, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 0, (1ULL << 47) - PAGE_SIZE, 2 * PAGE_SIZE, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 0, UINT64_MAX - PAGE_SIZE + 1, 2 * PAGE_SIZE, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 1ULL << 52, 0, PAGE_SIZE, rw, allocate) == -EINVAL);
    assert(vm_map(&root, 0, 0, 0, rw, allocate) == 0 && used == 0);
    assert(vm_map(&root, 0, VASL_VIRTUAL_BASE, PAGE_SIZE, rw, allocate) == 0);
    assert(lookup(VASL_VIRTUAL_BASE, &shift) == (rw | PAGE_FLAG_PRESENT));
    return 0;
}
