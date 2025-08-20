
boot_sources = files('boot.c')

gnu_efi_dir = meson.source_root() / 'gnu-efi'
efi_ldscript = gnu_efi_dir / 'elf_x86_64_efi.lds' 
efi_libs = ['-lgnuefi', '-lefi']

cc = meson.get_compiler('c')
ld = find_program('ld')
objcopy = find_program('objcopy')

boot_cflags = [
  '-mno-red-zone',
  '-fno-stack-protector',
  '-fpic',
  '-ffreestanding',
  '-fno-stack-check',
  '-Wall',
  '-fshort-wchar',
  '-I' + meson.current_source_dir() / 'include',
]

boot_ldflags = [
  '-shared',
  '-Bsymbolic',
  '-L' + gnu_efi_dir,
  '-T' + efi_ldscript,
  '-nostdlib'
] 

boot_obj = custom_target('boot.o',
  input: boot_sources,
  output: 'boot.o',
  command: [
    cc.cmd_array(),
    boot_cflags,
    '-c', '@INPUT@',
    '-o', '@OUTPUT@'
  ],
  depend_files: [boot_sources]
)

boot_elf = custom_target('boot.so',
  input: [
    gnu_efi_dir / 'crt0-efi-x86_64.o',
    boot_obj
  ],
  output: 'boot.so',
  command: [
    ld, 
    boot_ldflags,
    '@INPUT@', '-o', '@OUTPUT@',
    efi_libs
  ]
)

# Custom target to run objcopy
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