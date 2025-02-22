project(
  'osmium',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

gtkmm_dep = dependency('gtkmm-4.0')
httplib_dep = subproject('httplib').get_variable('cpp_httplib_dep')
libosmium_html_dep = subproject('osmium-html').get_variable('libosmium_html_dep')

executable(
  'osmium',
  'src/main.cc',
  install: true,
  dependencies: [gtkmm_dep, httplib_dep, libosmium_html_dep],
  cpp_args: ['-Wall', '-Wextra', '-Wpedantic', '-Wconversion'],
)