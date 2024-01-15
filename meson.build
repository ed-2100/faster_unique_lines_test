project(
    'fast_unique',
    ['cpp'],
    version: '0.1',
    default_options: [
        'warning_level=3',
        'cpp_std=c++20',
        'backend=ninja',
        'buildtype=custom',
        'optimization=3',
        'debug=false',
        'b_ndebug=true',
        'b_staticpic=false',
        'b_pie=false',
        'b_lto=true'
    ]
)

cpp_args = [
  '-march=native',
  '-mtune=native'
]

add_global_arguments(cpp_args, language: ['c', 'cpp'])

hh_proj = subproject('hh_c', default_options: [
  'simd=' + get_option('simd')
])
hh_dep = hh_proj.get_variable('hh_c')

cpp_args = []

if get_option('simd') == 'avx2'
  cpp_args = ['-mavx2']
endif

executable(
  'fast_unique',
  ['main.cpp'],
  cpp_args : cpp_args,
  link_args : cpp_args,
  dependencies : [hh_dep]
)
