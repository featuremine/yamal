project('yamal', 'cpp', 'c', version:'7.2.4')

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

pymod = import('python')

if meson.is_subproject()
  py_version = get_option('py_version')
  python3 = pymod.find_installation(py_version)
else
  python3 = pymod.find_installation()
endif


if is_windows and is_debug
  pyv =  python3.language_version().split('.')[1]
  py_base = 'C:/Program Files/Python3'+ pyv
  lib = [c_compiler.find_library( 'python3' +pyv + '_d',dirs:[py_base+'/libs/']) ]
  inc_dir = include_directories(py_base+'/include')
  py3_dep = declare_dependency(include_directories: inc_dir,
                                    dependencies:[lib])
else
  py3_dep = python3.dependency(embed:true)
endif


if (python3.language_version().split('.')[1].to_int() < 6)
  error('support python version 3.6 and higher')
endif

pp = run_command(python3, '-c', 'import sys;print(sys.executable)')
if pp.returncode() != 0
  error('failed command')
endif
python_path = pp.stdout().strip()



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

#add common subproject
common_sp = subproject('common')
fmc_dep = common_sp.get_variable('fmc_dep')
shared_fmc_dep = common_sp.get_variable('shared_fmc_dep')
shared_fmc = common_sp.get_variable('shared_fmc')
static_fmcpp_dep = common_sp.get_variable('shared_fmc_dep')
static_fmcpp = common_sp.get_variable('static_fmcpp')

if is_unix
  thread_dep = dependency('threads')
elif is_windows
  thread = []
  thread_dep = declare_dependency(dependencies : thread)
endif

src = include_directories('.')
install_samples = get_option('install_samples')

tclap_dep = dependency('tclap')

subdir('include')
subdir('src')
subdir('python')
subdir('bin')
subdir('tests')

if not is_subproject
  install_subdir('doc', install_dir : '.')
  install_data('README', install_dir : '.')
  install_data('LICENSE', install_dir : '.')
endif
