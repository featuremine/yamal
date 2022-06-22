pyytpbase_srcs = files('ytp_base.cpp')
pyytp_srcs = files('ytp.cpp')
pymodytp_srcs = files('py_ytp.cpp')
py_ibase = files(
    'ytp.pyi',
    'ytp_base.pyi',
)

py_srcs = files([
   '__init__.py',
])

py_tools = files(
  'tools/cli.py',
  'tools/__init__.py',
  'tools/commands/get_mod.py',
  'tools/commands/tail_mod.py',
  'tools/commands/__init__.py'
)

root_src = files(
  'ytp_base.py',
)

subdir('include')

pyshared = shared_library('pyytp', [pyytp_srcs],
                      include_directories : [inc, pyinc],
                      dependencies : [fmc_dep, py3_dep, static_libstdcxx_dep, shared_dep],
                      cpp_args : static_cpp_args,
                      link_args: static_link_args,
                      link_language: 'c',
                      install: true,
                      build_rpath: '$ORIGIN',
                      install_rpath: '$ORIGIN',
                      install_dir: 'lib')

pyshared_dep = declare_dependency(link_with: pyshared,
                                include_directories : [inc, pyinc],
                                version : meson.project_version(),
                                dependencies : [fmc_dep])

pybase_mod = python3.extension_module('ytp_base', pyytpbase_srcs,
                      include_directories : [inc],
                      dependencies : [fmc_dep, py3_dep, static_libstdcxx_dep, shared_dep],
                      cpp_args : static_cpp_args,
                      link_args: static_link_args,
                      link_language: 'c')

pysequence_mod = python3.extension_module('ytp', pymodytp_srcs,
                      include_directories : [inc],
                      dependencies : [fmc_dep, py3_dep, static_libstdcxx_dep, shared_dep, pyshared_dep],
                      cpp_args : static_cpp_args,
                      link_args: static_link_args,
                      link_language: 'c')

pybase_dep = declare_dependency(link_with: pybase_mod,
                               include_directories : [inc],
                               version : meson.project_version(),
                               dependencies : [fmc_dep, py3_dep])

pysequence_dep = declare_dependency(link_with: pysequence_mod,
                               include_directories : [inc],
                               version : meson.project_version(),
                               dependencies : [fmc_dep, py3_dep])

extensions = [pybase_mod, pysequence_mod, shared_fmc, shared, pyshared]

hadron = import('hadron')
ytp_pkg = hadron.package('ytp',
                         version: meson.project_version(),
                         py_sources: py_srcs + py_ibase + py_tools,
                         extensions: extensions,
                         root_files: root_src,
                         install: true,
                         verify: true,
                         documentation: true,
                         style: true,
                         python: python3)

ytp_pymod_path = 'package/' + meson.project_version()
