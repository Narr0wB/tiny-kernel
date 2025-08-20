project('tiny-kernel', 'c', version: '1.0')

c_flags = [
  '-ffreestanding', 
  '-fno-stack-protector', 
  '-fno-stack-check', 
  '-fshort-wchar', 
  '-Wall', 
  '-Wpedantic', 
  '-g', 
  '-O1',
]

add_project_arguments(c_flags, language: 'c')
kernel_include = include_directories('src/kernel/include')
subdir('src')
