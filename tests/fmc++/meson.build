config = executable('config', files('config.cpp'),
                         dependencies : [gtest, fmc_dep])
test('config', config, env : ['PYTHONPATH=package/'+meson.project_version()], timeout : 180)

variant_map = executable('variant_map', files('variant_map.cpp'),
                         dependencies : [gtest, fmc_dep])
test('variant_map', variant_map, env : ['PYTHONPATH=package/'+meson.project_version()], timeout : 180)
