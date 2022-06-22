cc = meson.get_compiler('cpp')

ytp_incdir = include_directories('include')

yamal_lib_path = meson.current_source_dir() + '/lib'
is_arm = build_machine.cpu_family() == 'aarch64'

thread_dep = dependency('threads')
dw = cc.find_library('dw', required : true)
unwind = cc.find_library('unwind', required : true)
if is_arm
    unwind_lib = cc.find_library('unwind-aarch64', required : true)
else
    unwind_lib = cc.find_library('unwind-x86_64', required : true)
endif

m_dep = meson.get_compiler('c').find_library('m', required : false)
gtest = dependency('gtest', required : false)

common_incdir = include_directories('include')
common_dep = declare_dependency(include_directories : [common_incdir],
                                   dependencies : [cc.find_library('fmc', dirs : yamal_lib_path)],
                                   version : meson.project_version())

shared_ytp_dep = declare_dependency(include_directories : [ytp_incdir],
                                      dependencies: [common_dep, cc.find_library('ytp', dirs : yamal_lib_path)],
                                      version : meson.project_version())

if not meson.is_subproject()
    add_project_arguments('-std=c++17', language : 'cpp')
    if is_linux
        add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
    endif
    sequence_test = executable('sequence_test', 'samples/sequence_test.cpp',
                              dependencies : [shared_ytp_dep])
    yamal_simple = executable('yamal_simple', 'samples/yamal_simple.cpp',
                              dependencies : [shared_ytp_dep])
endif
