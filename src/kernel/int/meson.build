
sources = files('int.c', 'notifier.c')

project_root = meson.source_root()
nasm = find_program('nasm', required: true)
int_asm = custom_target(
    'int_asm.o',
    input: project_root / 'src/arch/x86_64/isr.asm',
    output: 'int_asm.o',
    command: [nasm, '-f', 'elf64', '-g', '-o', '@OUTPUT@', '@INPUT@'],
    build_by_default: true
)

int_obj = static_library('int', sources, objects: int_asm, include_directories: kernel_include)

int_dep = declare_dependency(link_with: int_obj, include_directories: kernel_include)