subdir('util')
subdir('spirv')
subdir('vulkan')
subdir('dxvk')

if get_option('enable_d3d9')
  subdir('dxso')
  subdir('d3d9')
endif

# Nothing selected
if not get_option('enable_d3d9') and not get_option('enable_d3d10') and not get_option('enable_d3d11') and not get_option('enable_tests')
  warning('Nothing selected to be built. Are you missing a frontend or tests?')
endif
