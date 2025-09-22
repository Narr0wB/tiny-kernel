# Include kernel submodules

subdir('arch/x86')
subdir('device')
subdir('fs')
subdir('kernel')
subdir('mm')

link_script = meson.source_root() / 'src' / 'arch' / 'x86' / 'kernel.ld'
ksymtable_script = meson.source_root() /'scripts' / 'ksymtable.py' 
link_args = [
  '-no-pie', 
  '-nostdlib', 
  '-T', link_script,
]

link_modules = [
  arch_x86_boot,
  arch_x86_drivers,
  arch_x86_kernel,
  arch_x86_mm,
  fs,
  kernel,
  mm
]

# To generate a symtable, we need to:
#   1. Link the kernel 
#   2. Generate a symtable based on the first iteration of the kernel
#   3. Link the kernel with the symtable 
#   4. Generate a symtable based on the second iteration of the kernel
#   5. Link the third and (hopefully) final version of the kernel 

vmtiny1 = executable(
  'vmtiny1.elf',
  link_with: link_modules,
  link_args: link_args,
  install: false
)

ksyms1 = custom_target(
  'ksyms1',
  input: vmtiny1,
  output: 'ksymtable1.S',
  command: [
    'python3', ksymtable_script, '@INPUT@', '@OUTPUT@'
  ]
)

vmtiny2 = executable(
  'vmtiny2.elf',
  sources: [ksyms1],
  link_with: link_modules,
  link_args: link_args,
  install: false
)

ksyms2 = custom_target(
  'ksyms2',
  input: vmtiny2,
  output: 'ksymtable2.S',
  command: [
    'python3', ksymtable_script, '@INPUT@', '@OUTPUT@'
  ]
)

vmtiny = executable(
  'vmtiny.elf',
  sources: [ksyms2],
  link_with: link_modules,
  link_args: link_args,
  install: false
)