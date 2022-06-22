ytp_srcs = files(
	'yamal.cpp',
	'peer.cpp',
	'channel.cpp',
	'time.cpp',
	'control.cpp',
	'sequence.cpp',
	'timeline.cpp'
)

yamal_srcs = files(
	'yamal.cpp'
)

cc = meson.get_compiler('cpp')

shared = shared_library('ytp', [ytp_srcs],
                      include_directories : [inc],
                      dependencies : [fmc_dep, static_libstdcxx_dep, thread_dep],
                      cpp_args : static_cpp_args,
                      link_args: static_link_args,
                      link_language: 'c',
                      install: true,
                      build_rpath: '$ORIGIN',
                      install_rpath: '$ORIGIN',
                      install_dir: 'lib')

shared_dep = declare_dependency(link_with: shared,
                                include_directories : [inc],
                                version : meson.project_version(),
                                dependencies : [fmc_dep])
