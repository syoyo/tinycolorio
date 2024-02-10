project('fuzz_tcio', 'c', 'cpp', default_options : ['cpp_std=c++14'])

tcio_sources=[
  ]

incdirs = include_directories(['../',
  ])

executable('fuzz_tcio',
  'tcio_fuzzmain.cc',
  tcio_sources,
  install: true,
  include_directories : incdirs,
  cpp_args : ['-fsanitize=address,fuzzer', '-DTINY_COLOR_IO_FUZZER_BUILD=1'],
  link_args : '-fsanitize=address,fuzzer' )

