if is_windows
  link_args = []	
elif host_machine.system() in ['darwin']
  link_args = []
elif host_machine.system() == 'linux'
  link_args = []
endif

ytp_perf = executable('ytp-perf', 'ytp-perf.cpp',
						dependencies : [static_fmc_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

ytp_stats = executable('ytp-stats', 'ytp-stats.cpp',
						dependencies : [static_fmc_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

ytp_daemon = executable('ytp-daemon', 'ytp-daemon.cpp',
						dependencies : [static_fmc_dep, static_fmcpp_dep, static_ytp_dep, tclap_dep],
						install: true,
						install_dir: 'tools/ytp',
						build_rpath: '$ORIGIN:$ORIGIN/../lib',
						install_rpath: '$ORIGIN:$ORIGIN/../../lib',
						link_args: link_args)

if install_samples
	install_data('ytp-perf.cpp', install_dir: 'tools/ytp')
endif
