
sources = files('syscall.c')
sys_obj = static_library('sys', sources, include_directories: kernel_include)
sys_dep = declare_dependency(link_with: sys_obj, include_directories: kernel_include)