
sources = files(
    'idt.S', 
    'panic.S',
    'cpu.c',
    'idt.c',
    'backtrace.c'
)

arch_x86_kernel = static_library('arch_x86_kernel', sources, include_directories: kernel_include)