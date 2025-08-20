
boot_sources = files('boot.c')

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

gnu_efi = meson.source_root() / 'gnu-efi'
efi_ldscript = gnu_efi / 'elf_x86_64_efi.lds'  # GNU-EFI linker script
efi_libs = ['-lgnuefi', '-lefi']  # UEFI libraries

boot_ldflags = [
    '-shared',
    '-Bsymbolic',
    '-L' + gnu_efi,
    '-nostdlib',
    '-T' + efi_ldscript,
  ] + efi_libs

# Build shared object (intermediate step)

boot_obj = custom_target('boot.o',
  input: boot_sources,
  output: 'boot.o',
  command: [
    'x86_64-elf-gcc' ]
    + 
    boot_cflags 
    + ['-o', '@OUTPUT@',
    '-c',
    '@INPUT@'
  ],
  depend_files: [boot_sources]
)

boot_elf = custom_target('boot.elf',
  input: [
    boot_obj,
    gnu_efi / 'crt0-efi-x86_64.o'
  ],
  output: 'boot.so',
  command: [
    'x86_64-elf-ld']
    + boot_ldflags
    + ['@INPUT1@',  # crt0-efi-x86_64.o
    '@INPUT0@',  # boot.o
    '-o', '@OUTPUT@'
  ]
)

# Custom target to run objcopy
boot_efi = custom_target('boot.efi',
  input: boot_elf,
  output: 'bootx64.efi',
  command: [
    'objcopy',
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
    '--target=efi-app-x86_64',
    '--subsystem=10',
    '@INPUT@',
    '@OUTPUT@',
  ],
  build_by_default: true,
)