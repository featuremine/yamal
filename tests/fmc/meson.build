tests = ['error', 'files', 'math', 'time', 'threaded', 'shared_map']

foreach test : tests
    name = test
    main = name + '.cpp'
    exe = executable(
        'common_' + name, main,
        dependencies : [fmc_dep, gtest])
    test('common_' + name, exe, is_parallel: false)
endforeach

compiles_c = executable(
    'common_compiles_c', 'compiles.c',
    dependencies : [fmc_dep])
test('common_compiles_c', compiles_c, is_parallel: false)

subdir('config')
