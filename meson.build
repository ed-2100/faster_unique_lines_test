project(
    'proj',
    ['cpp'],
    version: '0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
        'backend=ninja'
    ]
)

cmake = import('cmake')

hh_proj = cmake.subproject('highwayhash')
hh_dep = hh_proj.dependency('highwayhash')

executable(
  'fast_unique',
  ['main.cpp'],
  cpp_args : ['-march=native', '-mtune=native', '-Ofast', '-fno-exceptions', '-fno-rtti', '-g'],
  dependencies : [hh_dep],
  install : true
)

# meson build --wipe -Ddebug=true -Doptimization=3
# meson build --wipe -Dbuildtype=release -Doptimization=3
