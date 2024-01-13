project(
    'fast_unique',
    ['cpp'],
    version: '0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
        'backend=ninja',
        'buildtype=custom',
        'optimization=g',
        'debug=true',
        'b_ndebug=false',
        'b_lto=true'
    ]
)

#add_global_link_arguments('-static-libstdc++', language: ['c', 'cpp'])

args = [
  '-march=native',
  '-mtune=native',
]

add_global_arguments(cpp_args, language: ['c', 'cpp'])

cmake = import('cmake')

hh_proj = cmake.subproject('highwayhash')
hh_dep = hh_proj.dependency('highwayhash')

executable(
  'fast_unique',
  ['main.cpp'],
  dependencies : [hh_dep]
)
