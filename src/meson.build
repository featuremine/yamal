srcs_c = files(
    'fmc/cmp.c',
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
    'fmc++/config.cpp'
)

cc = meson.get_compiler('cpp')

if is_linux
    extr_dep_deps = [dl]
else
    extr_dep_deps = []
endif

shared_fmc = shared_library('fmc', [srcs_c],
                            include_directories : [inc],
                            dependencies : [static_libstdcxx_dep, thread_dep, extr_dep_deps],
                            cpp_args : static_cpp_args,
                            link_args: static_link_args,
                            link_language: 'c',
                            install: true,
                            install_dir: 'lib')

shared_fmc_dep = declare_dependency(link_with: [shared_fmc],
                                    include_directories : [inc],
                                    version : meson.project_version())

static_fmcpp = static_library('fmc++', [srcs_cpp],
                              include_directories : [inc],
                              dependencies : [shared_fmc_dep, thread_dep, extr_dep_deps],
                              cpp_args : static_cpp_args,
                              install: true,
                              install_dir: 'lib')

static_fmcpp_dep = declare_dependency(link_with: [static_fmcpp],
                                      include_directories : [inc],
                                      dependencies : [shared_fmc_dep, thread_dep, extr_dep_deps],
                                      version : meson.project_version())

fmc_dep = declare_dependency(dependencies : [shared_fmc_dep, static_fmcpp_dep],
                             version : meson.project_version())

ytp_srcs = files(
	'ytp/yamal.cpp',
	'ytp/peer.cpp',
	'ytp/channel.cpp',
	'ytp/time.cpp',
	'ytp/control.cpp',
	'ytp/sequence.cpp',
	'ytp/timeline.cpp'
)

yamal_srcs = files(
	'ytp/yamal.cpp'
)

shared_ytp = shared_library('ytp', [ytp_srcs],
                            include_directories : [inc],
                            dependencies : [fmc_dep, static_libstdcxx_dep, thread_dep],
                            cpp_args : static_cpp_args,
                            link_args: static_link_args,
                            link_language: 'c',
                            install: true,
                            build_rpath: '$ORIGIN',
                            install_rpath: '$ORIGIN',
                            install_dir: 'lib')

shared_ytp_dep = declare_dependency(link_with: shared_ytp,
                                    include_directories : [inc],
                                    version : meson.project_version(),
                                    dependencies : [fmc_dep])


subdir('tools')