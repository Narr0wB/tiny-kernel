
project_root = meson.source_root()
nasm = find_program('nasm', required: true)
memory_asm = custom_target(
    'memory_asm.o',
    input: project_root / 'src/arch/x86_64/memory.asm',
    output: 'memory_asm.o',
    command: [nasm, '-f', 'elf64', '-Wall', '-o', '@OUTPUT@', '@INPUT@'],
)

sources = files('memory.c', 'palloc.c')
memory_obj = static_library('memory', sources, objects: memory_asm, include_directories: kernel_include)
memory_dep = declare_dependency(link_with: memory_obj, include_directories: kernel_include)