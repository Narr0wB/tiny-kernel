
project_root = meson.source_root()
nasm = find_program('nasm', required: true)
util_asm = custom_target(
    'util_asm.o',
    input: project_root / 'src/arch/x86_64/panic.asm',
    output: 'util_asm.o',
    command: [nasm, '-f', 'elf64', '-Wall', '-o', '@OUTPUT@', '@INPUT@'],
    build_by_default: true
)

sources = files('io.c', 'lib.c', 'panic.c', 'string.c')
util_obj = static_library('util', sources, objects: util_asm, include_directories: kernel_include)
util_dep = declare_dependency(link_with: util_obj, include_directories: kernel_include)