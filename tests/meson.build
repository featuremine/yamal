src_dir = meson.current_source_dir()

ytp_tests = ['yamal', 'peer', 'channel', 'time', 'control', 'timeline', 'sequence', 'containers']

foreach test : ytp_tests
    name = test
    main = name + '.cpp'
    exe = executable(
        'ytp_' + name, main,
        dependencies : [shared_dep, gtest])
    test('ytp_' + name, exe, is_parallel: false)
endforeach

test('pyext' , python3,
    args : [src_dir + '/pyext.py'],
    env : ['PYTHONPATH='+ ytp_pymod_path],
    timeout: 180)

ytp_big_endian = executable(
    'ytp_big_endian', ['yamal.cpp'] + yamal_srcs,
    include_directories : [inc],
    cpp_args : '-DYTP_USE_BIG_ENDIAN',
    dependencies : [fmc_dep, static_libstdcxx_dep, thread_dep, gtest])
test('ytp_big_endian', ytp_big_endian, is_parallel: false)

compiles_c = executable(
    'ytp_compiles_c', 'compiles.c',
    dependencies : [shared_dep, py3_dep])
test('ytp_compiles_c', compiles_c, is_parallel: false)

ytp_common = executable(
    'ytp_common', 'common.cpp',
    dependencies : [shared_dep, gtest])
test('ytp_common', ytp_common, is_parallel: false)

if install_samples
    install_data('transport_writer_ids.base', install_dir : join_paths([get_option('prefix') + '/test/yamal']))
    install_data('transport_writer_transactions.base', install_dir : join_paths([get_option('prefix') + '/test/yamal']))
    install_data('transport_writer_test', install_dir: join_paths([get_option('prefix') + '/test/yamal']))
endif
