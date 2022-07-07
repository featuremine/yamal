fmc_root_incs = [
	'fmc/alignment.h',
	'fmc/debug.h',
	'fmc/endianness.h',
	'fmc/error.h',
	'fmc/extension.h',
	'fmc/files.h',
	'fmc/math.h',
	'fmc/murmur3.h',
	'fmc/platform.h',
	'fmc/process.h',
	'fmc/signals.h',
	'fmc/sockets.h',
	'fmc/test.h',
	'fmc/time.h',
]

cmp_incs = [
	'fmc/cmp/cmp.h',
	'fmc/cmp/serialization.hpp',
]

fmcpp_root_incs = [
	'fmc++/component.hpp',
	'fmc++/logger.hpp',
	'fmc++/metatable.hpp',
	'fmc++/ordered_map.hpp',
	'fmc++/threaded.hpp',
	'fmc++/strings.hpp',
	'fmc++/serialize.hpp',
	'fmc++/stackdump.hpp',
	'fmc++/memory.hpp',
	'fmc++/misc.hpp',
	'fmc++/mpl.hpp',
	'fmc++/fs.hpp',
	'fmc++/gtestwrap.hpp',
	'fmc++/time.hpp',
	'fmc++/shared_map.hpp',
	'fmc++/counters.hpp',
	'fmc++/convert.hpp',
	'fmc++/units.hpp',
]

fmcpp_cfg_incs = [
	'fmc++/config/config.hpp',
	'fmc++/config/serialize.hpp',
	'fmc++/config/variant_map.hpp'
]

fmcpp_python_incs = [
	'fmc++/python/component.hpp',
	'fmc++/python/config.hpp',
	'fmc++/python/wrapper.hpp'
]

if install_samples
	install_headers(fmc_root_incs, subdir : 'fmc')
	install_headers(cmp_incs, subdir : 'fmc/cmp')
	install_headers(fmcpp_root_incs, subdir : 'fmc++')
	install_headers(fmcpp_cfg_incs, subdir : 'fmc++/config')
	install_headers(fmcpp_python_incs, subdir : 'fmc++/python')
endif

inc = include_directories('.')
