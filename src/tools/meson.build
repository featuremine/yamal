if is_windows
  link_args = []	
elif host_machine.system() in ['darwin']
  link_args = []
elif host_machine.system() == 'linux'
  link_args = []
endif

yamal_perf = executable('yamal-perf', 'yamal-perf.cpp',
						dependencies : [static_fmc_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

yamal_stats = executable('yamal-stats', 'yamal-stats.cpp',
						dependencies : [static_fmc_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

yamal_daemon = executable('yamal-daemon', 'yamal-daemon.cpp',
						dependencies : [static_fmc_dep, static_fmcpp_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

yamal_tail = executable('yamal-tail', 'yamal-tail.cpp',
						dependencies : [static_fmc_dep, static_fmcpp_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

# yamal_run = executable('yamal-run', 'yamal-run.cpp',
# 						dependencies : [static_fmc_dep, tclap_dep],
# 						install: true,
# 						install_dir: 'tools/fmc',
# 						build_rpath: '$ORIGIN:$ORIGIN/../lib',
# 						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
# 						link_args: link_args)

if install_samples
	install_data('yamal-perf.cpp', install_dir: 'tools/ytp')
endif
