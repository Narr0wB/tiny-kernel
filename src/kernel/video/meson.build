
sources = files('video.c')

video_obj = static_library('video', sources, include_directories: kernel_include)

video_dep = declare_dependency(link_with: video_obj, include_directories: kernel_include)