
sources = files('io.c', 'lib.c', 'panic.c', 'string.c', arch_x86 / 'util/panic.S')
util_obj = static_library('util', sources, include_directories: kernel_include)
util_dep = declare_dependency(link_with: util_obj, include_directories: kernel_include)