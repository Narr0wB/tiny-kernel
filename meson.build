project('tiny-kernel', 'c', version: '1.0')

c_flags = [
  '-ffreestanding', 
  '-fno-omit-frame-pointer', 
  '-fno-stack-protector', 
  '-fno-stack-check', 
  '-mno-red-zone', 
  '-fshort-wchar', 
  '-fno-pie',
  '-fno-pic',
  '-mcmodel=large',
  '-Wall', 
  '-Wpedantic', 
  '-g', 
  '-ggdb',
  '-O0',
]

add_project_arguments(c_flags, language: 'c')

kernel_include = include_directories('src/include', 'src/arch/x86/include')
boot_include = include_directories('uefi-bootloader/include') 

arch_x86 = meson.source_root() / 'src/arch/x86'

subdir('src')
subdir('uefi-bootloader')

