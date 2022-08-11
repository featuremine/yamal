tests = ['error', 'files', 'math', 'time', 'config', 'priority_queue']

foreach test : tests
    name = test
    main = name + '.cpp'
    exe = executable(
        'common_' + name, main,
        dependencies : [static_fmc_dep, gtest])
    test('common_' + name, exe, is_parallel: false)
endforeach

extension = executable(
    'common_extension', 'extension.cpp',
    dependencies : [static_fmc_dep, gtest])
test('common_extension', extension, args: [shared_fmc.full_path()], is_parallel: false)

compiles_c = executable(
    'common_compiles_c', 'compiles.c',
    dependencies : [static_fmc_dep])
test('common_compiles_c', compiles_c, is_parallel: false)

shared_test_component = shared_library('testcomponent', [files('testcomponent.c')],
                                        name_prefix : '',
                                        include_directories : [inc],
                                        dependencies : [shared_fmc_dep],
                                        link_language: 'c',
                                        install: false)

shared_io_component = shared_library('iocomponent', [files('iocomponent.c')],
                                     name_prefix : '',
                                     include_directories : [inc],
                                     dependencies : [shared_fmc_dep],
                                     link_language: 'c',
                                     install: false)

test_component = executable(
    'common_component', 'component.cpp',
    dependencies : [static_fmc_dep, gtest])
test('common_component', test_component, args: [shared_test_component.full_path()], is_parallel: false,
     dependencies: [shared_test_component, shared_io_component])

memory = executable(
    'common_memory', 'memory.cpp',
    dependencies : [static_fmc_dep, gtest])
test('common_memory', memory)
