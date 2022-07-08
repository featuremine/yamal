tests = ['config', 'shared_map', 'threaded', 'variant_map']

foreach test : tests
    name = test
    main = name + '.cpp'
    exe = executable(
        'common_' + name, main,
        dependencies : [fmc_dep, gtest])
    test('common_' + name, exe, is_parallel: false)
endforeach