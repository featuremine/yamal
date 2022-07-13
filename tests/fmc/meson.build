tests = ['error', 'files', 'math', 'time']

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