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
        'b_lto=true',
        'b_pie=false'
    ]
)

# cpp_args = [
#   '-march=native',
#   '-mtune=native'
# ]

# add_global_arguments(cpp_args, language: ['c', 'cpp'])

hh_proj = subproject('hh_c', default_options: [
  'b_lto=true',
  'b_pie=false',
  'simd=' + get_option('simd')
])
hh_dep = hh_proj.get_variable('hh_c')

executable(
  'fast_unique',
  ['main.cpp'],
  dependencies : [hh_dep]
)
