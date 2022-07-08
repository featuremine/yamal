src_dir = meson.current_source_dir()

ytp_tests = ['yamal', 'peer', 'channel', 'time', 'control', 'timeline', 'sequence', 'containers']

foreach test : ytp_tests
    name = test
    main = name + '.cpp'
    exe = executable(
        'ytp_' + name, main,
        dependencies : [static_ytp_dep, gtest])
    test('ytp_' + name, exe, is_parallel: false)
endforeach


ytp_big_endian = executable(
    'ytp_big_endian', ['yamal.cpp'] + yamal_srcs,
    include_directories : [inc],
    cpp_args : '-DYTP_USE_BIG_ENDIAN',
    dependencies : [static_fmc_dep, static_libstdcxx_dep, thread_dep, gtest])
test('ytp_big_endian', ytp_big_endian, is_parallel: false)

compiles_c = executable(
    'ytp_compiles_c', 'compiles.c',
    dependencies : [static_ytp_dep])
test('ytp_compiles_c', compiles_c, is_parallel: false)
