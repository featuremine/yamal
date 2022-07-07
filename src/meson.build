srcs_c = files(
    'fmc/cmp/cmp.c',
    'fmc/math.c',
    'fmc/murmur3.c',
    'fmc/signals.c',
    'fmc/error.cpp',
    'fmc/files.cpp',
    'fmc/test.cpp',
    'fmc/time.cpp',
    'fmc/sockets.cpp',
    'fmc/process.cpp',
    'fmc/extension.cpp'
)

srcs_cpp = files(
    'fmc++/stackdump.cpp',
    'fmc++/config/config.cpp',
    'fmc++/python/config.cpp'
)

if is_linux
    extr_dep_deps = [dl, dw, unwind, unwind_lib]
else
    extr_dep_deps = []
endif

cc = meson.get_compiler('cpp')

shared_fmc = shared_library('fmc', [srcs_c],
                            include_directories : [inc],
                            dependencies : [static_libstdcxx_dep, thread_dep, extr_dep_deps],
                            cpp_args : static_cpp_args,
                            link_args: static_link_args,
                            link_language: 'c',
                            install: true,
                            install_dir: 'lib')

static_fmcpp = static_library('fmc++', [srcs_cpp],
                              include_directories : [inc],
                              dependencies : [thread_dep, extr_dep_deps, py3_dep],
                              cpp_args : static_cpp_args,
                              install: true,
                              install_dir: 'lib')

shared_fmc_dep = declare_dependency(link_with: [shared_fmc],
                                    include_directories : [inc],
                                    version : meson.project_version())

static_fmcpp_dep = declare_dependency(link_with: [static_fmcpp],
                                      include_directories : [inc],
                                      version : meson.project_version())

fmc_dep = declare_dependency(link_with: [static_fmcpp, shared_fmc],
                             include_directories : [inc],
                             version : meson.project_version())
