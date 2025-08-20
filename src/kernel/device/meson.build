sources = files('device.c', 'keyboard.c', 'ps2.c', 'serial.c')

device_obj = static_library('device', sources, include_directories: kernel_include)

device_dep = declare_dependency(link_with: device_obj, include_directories: kernel_include)