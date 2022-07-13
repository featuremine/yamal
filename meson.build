project('yamal', 'cpp', 'c', version:'7.2.6')

version = meson.project_version()

is_windows = build_machine.system() == 'windows'
is_macos = build_machine.system() == 'darwin'
is_linux = build_machine.system() == 'linux'
is_unix = is_linux or is_macos

if not is_windows
  add_project_arguments('-Wno-unused-variable', language : 'cpp')
  add_project_arguments('-Wno-deprecated-declarations', language : 'cpp')
  # Commented out since this option is not available in gcc 7.2.1
  # add_project_arguments('-Wclass-memaccess', language : 'cpp')
endif

if is_windows
  add_project_arguments('/std:c++17', language : 'cpp')
  add_project_arguments('-D_WIN32_WINNT=0x0A00', language : 'cpp')
  add_project_arguments('-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING', language : 'cpp')
elif is_linux
  add_project_arguments('-std=c++17', language : 'cpp')
  add_project_link_arguments('-static-libstdc++', language : 'cpp')
  add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
elif is_macos
  add_project_arguments('-std=c++17', language : 'cpp')
endif

if is_linux
  add_project_arguments('-fconcepts', language : 'cpp')
endif

is_debug = false

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DNBUILD', language : 'cpp')
  is_debug = true
endif

cpp_compiler = meson.get_compiler('cpp')
c_compiler = meson.get_compiler('c')

if is_windows
  path_separator = ';'
else
  path_separator = ':'
endif

if is_macos
  static_libstdcxx = cpp_compiler.find_library('c++')
  static_link_args = ['-fvisibility-inlines-hidden',
                      '-fvisibility=hidden']
elif is_linux
  add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
  static_libstdcxx = [cpp_compiler.find_library('stdc++', static: true), cpp_compiler.find_library('m', required : true)]
  static_link_args = ['-fvisibility-inlines-hidden',
                      '-fvisibility=hidden',
                      '-Wl,--exclude-libs=ALL']
elif is_windows
  static_link_args = []
  static_libstdcxx = []
  static_libstdcxx_dep = declare_dependency(dependencies : static_libstdcxx)
endif

if is_unix
  static_libstdcxx_dep = declare_dependency(dependencies : static_libstdcxx)
  static_cpp_args = ['-fvisibility=hidden',
                    '-fvisibility-inlines-hidden']

  strip = find_program('strip', required : true)
elif is_windows
  static_cpp_args = []
endif

dl = []
if is_linux
  dl = cpp_compiler.find_library('dl', required : true)
endif

is_subproject = meson.is_subproject()

if is_windows
base = 'C:/vcpkg/installed/x64-windows'
if is_debug
  vcpkg_dir =base + '/debug'
  gtest_lib = 'gtestd'
else
  vcpkg_dir =base
   gtest_lib = 'gtest'
endif

gtest_lib = meson.get_compiler('cpp').find_library(gtest_lib, dirs: vcpkg_dir+'/lib', required: true)
gtest_inc = include_directories( base+'/include')
gtest = declare_dependency(dependencies : [gtest_lib],
  include_directories : gtest_inc)
else
gtest = dependency('gtest')
endif

tclap_dep = dependency('tclap')

if is_unix
  thread_dep = dependency('threads')
elif is_windows
  thread = []
  thread_dep = declare_dependency(dependencies : thread)
endif

install_samples = get_option('install_samples')

subdir('include')
subdir('src')
subdir('tests')

if not is_subproject
  install_subdir('doc', install_dir : '.')
  install_data('README.md', install_dir : '.')
  install_data('LICENSE', install_dir : '.')
endif
