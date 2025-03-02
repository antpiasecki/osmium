project(
  'osmium',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++17'],
)

cmake = import('cmake')

qt_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])
httplib_dep = subproject('httplib').get_variable('cpp_httplib_dep')
libosmium_html_dep = subproject('osmium-html').get_variable('libosmium_html_dep')

executable(
  'osmium',
  'src/main.cc',
  'src/mainwindow.cc',
  install: true,
  dependencies: [qt_dep, httplib_dep, libosmium_html_dep],
  cpp_args: ['-Wall', '-Wextra', '-Wpedantic'],
)