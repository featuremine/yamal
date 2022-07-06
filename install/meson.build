cc = meson.get_compiler('cpp')

ytp_incdir = include_directories('include')

yamal_lib_path = meson.current_source_dir() + '/lib'
is_arm = build_machine.cpu_family() == 'aarch64'

thread_dep = dependency('threads')

m_dep = meson.get_compiler('c').find_library('m', required : false)
gtest = dependency('gtest', required : false)

apr_incdir = include_directories('include') # TODO
apr_dep = declare_dependency(include_directories : [apr_incdir],
                             dependencies : [cc.find_library('apr-1', dirs : yamal_lib_path)],
                             version : meson.project_version())

shared_ytp_dep = declare_dependency(include_directories : [ytp_incdir],
                                      dependencies: [apr_dep, cc.find_library('ytp', dirs : yamal_lib_path)],
                                      version : meson.project_version())

if not meson.is_subproject()
    add_project_arguments('-std=c++17', language : 'cpp')
    if is_linux
        add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
    endif
    yamal_simple = executable('yamal_simple', 'samples/simple.cpp',
                              dependencies : [shared_ytp_dep])
endif
