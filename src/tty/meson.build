
sources = files('tty.c')
tty_obj = static_library('tty', sources, include_directories: kernel_include)
tty_dep = declare_dependency(link_with: tty_obj, include_directories: kernel_include)