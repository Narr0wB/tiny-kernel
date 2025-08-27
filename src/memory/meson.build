
sources = files('memory.c', 'pmm.c', arch_x86 / 'memory/memory.S')
memory_obj = static_library('memory', sources, include_directories: kernel_include)
memory_dep = declare_dependency(link_with: memory_obj, include_directories: kernel_include)