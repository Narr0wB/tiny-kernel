

# __allocator_free_block
All blocks are freed here. Given a block, we check its buddy, if buddy is free (PG_FREE is SET), then merge into 
bigger buddy (order + 1) and repeat until we cannot merge any more.