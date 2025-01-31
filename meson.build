project('repro', 'cpp', default_options : ['cpp_std=c++23'])

pipewire_dep = dependency('libpipewire-0.3')
wireplumber_dep = dependency('wireplumber-0.5')

executable(
  'om-example',
  'main.cpp',
  dependencies: [pipewire_dep, wireplumber_dep])

