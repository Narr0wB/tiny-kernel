# Include kernel submodules

subdir('device')
subdir('fs')
subdir('int')
subdir('memory')
subdir('sys')
subdir('tty')
subdir('util')
subdir('video')

# Aggregate all kernel dependencies


kernel_core = static_library('kernel', 'kernel.c', include_directories: kernel_include)

link_script = meson.source_root() / 'src' / 'arch' / 'x86_64' / 'kernel.ld'
link_args = [
  '-nostdlib', '-T', link_script,
]

executable('kernel.elf',
  # Instead of listing your dependencies as positional arguments, use link_with:
  link_with: [
    kernel_core,
    device_obj,
    fs_obj,
    int_obj,
    memory_obj,
    sys_obj,
    tty_obj,
    util_obj,
    video_obj
  ],
  link_args: link_args,
  install: false
)