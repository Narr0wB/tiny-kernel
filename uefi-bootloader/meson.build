
gnu_efi_dir = meson.source_root() / 'gnu-efi'
efi_ldscript = gnu_efi_dir / 'elf_x86_64_efi.lds' 
efi_libs = ['-lgnuefi', '-lefi']

boot_sources = files('boot.c')

boot_cflags = [
  '-mno-red-zone',
  '-fno-stack-protector',
  '-ffreestanding',
  '-fno-stack-check',
  '-fPIC',
  '-Wall',
  '-fshort-wchar',
  '-g3'
]


tmp = static_library(
  'tmp',
  boot_sources,
  install: false,
  c_args: boot_cflags,
  include_directories: [boot_include, kernel_include]
)

boot_objs = tmp.extract_all_objects()

boot_ldflags = [
  '-shared',
  '-Bsymbolic',
  '-L' + gnu_efi_dir,
  '-T' + efi_ldscript,
  '-nostdlib',
] 

ld = find_program('ld')
boot_elf = custom_target('bootx64.elf',
  input: [
    gnu_efi_dir / 'crt0-efi-x86_64.o',
    boot_objs
  ],
  output: 'bootx64.elf',
  command: [
    ld, 
    boot_ldflags,
    '@INPUT@', '-o', '@OUTPUT@',
    efi_libs
  ]
)


objcopy = find_program('objcopy')
boot_efi = custom_target('boot.efi',
  input: boot_elf,
  output: 'bootx64.efi',
  command: [
    objcopy,
    '-j', '.text',
    '-j', '.sdata',
    '-j', '.data',
    '-j', '.rodata',
    '-j', '.dynamic',
    '-j', '.dynsym',
    '-j', '.rel',
    '-j', '.rela',
    '-j', '.rel.*',
    '-j', '.rela.*',
    '-j', '.reloc',
    '--target', 'efi-app-x86_64',
    '--subsystem=10',
    '@INPUT@',
    '@OUTPUT@',
  ],
  build_by_default: true,
)