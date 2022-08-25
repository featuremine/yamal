tests = ['config', 'shared_map', 'strings', 'threaded', 'variant_map']

foreach test : tests
    name = test
    main = name + '.cpp'
    exe = executable(
        'commonpp_' + name, main,
        dependencies : [static_fmc_dep, static_fmcpp_dep, gtest])
    test('commonpp_' + name, exe, is_parallel: false)
endforeach