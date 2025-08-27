
sources = files('int.c', 'notifier.c', arch_x86 / 'int/isr.S')
int_obj = static_library('int', sources, include_directories: kernel_include)
int_dep = declare_dependency(link_with: int_obj, include_directories: kernel_include)