
sources = files('vfs.c')

fs_obj = static_library('fs', sources, include_directories: kernel_include)

fs_dep = declare_dependency(link_with: fs_obj, include_directories: kernel_include)