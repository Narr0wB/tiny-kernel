project('tiny-kernel', 'c', 
        version: '1.0')

# Global compiler flags (defined in root)

c_flags = [
  '-ffreestanding', '-fno-stack-protector', '-fno-stack-check', '-fshort-wchar', '-Wall', '-Wpedantic', '-g', '-O1',
  '-I' + meson.current_source_dir() / 'src/include',
  '-I' + meson.current_source_dir() / 'src/boot/include',
]
add_project_arguments(c_flags, language: 'c')

# Include subdirectories
kernel_include = include_directories('src/kernel/include')

subdir('src')