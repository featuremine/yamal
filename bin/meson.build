if is_windows
  link_args = []	
elif host_machine.system() in ['darwin']
  link_args = []
elif host_machine.system() == 'linux'
  link_args = []
endif

yamal_producer = executable('yamal_producer', 'yamal_producer.cpp',
							  dependencies : [fmc_dep, shared_dep],
							  install: install_samples,
							  install_dir: 'tools/yamal',
                              build_rpath: '$ORIGIN:$ORIGIN/../lib',
                              install_rpath: '$ORIGIN:$ORIGIN/../../lib',
							  link_args: link_args)

yamal_consumer = executable('yamal_consumer', 'yamal_consumer.cpp',
							  dependencies : [fmc_dep, shared_dep],
							  install: install_samples,
							  install_dir: 'tools/yamal',
                              build_rpath: '$ORIGIN:$ORIGIN/../lib',
                              install_rpath: '$ORIGIN:$ORIGIN/../../lib',
							  link_args: link_args)

ytp_writer = executable('ytp_writer', 'ytp_writer.cpp',
							  dependencies : [fmc_dep, shared_dep],
							  install: false,
							  install_dir: 'tools/ytp',
                              build_rpath: '$ORIGIN:$ORIGIN/../lib',
                              install_rpath: '$ORIGIN:$ORIGIN/../../lib',
							  link_args: link_args)

ytp_reader = executable('ytp_reader', 'ytp_reader.cpp',
							  dependencies : [fmc_dep, shared_dep],
							  install: install_samples and not is_subproject,
							  install_dir: 'tools/ytp',
                              build_rpath: '$ORIGIN:$ORIGIN/../lib',
                              install_rpath: '$ORIGIN:$ORIGIN/../../lib',
							  link_args: link_args)

ytp_perf = executable('ytp_perf', 'ytp_perf.cpp',
							  dependencies : [fmc_dep, shared_dep, tclap_dep],
							  install: true,
							  install_dir: 'tools/ytp',
                              build_rpath: '$ORIGIN:$ORIGIN/../lib',
                              install_rpath: '$ORIGIN:$ORIGIN/../../lib',
							  link_args: link_args)

if install_samples
	install_data('ytp_tail', install_dir: 'tools/ytp')
	install_data('kafka_yamal.py', install_dir: 'tools/yamal')
	install_data('ytp_kafka.py', install_dir: 'tools/ytp')
	install_data('ytp_perf.cpp', install_dir: 'tools/ytp')
endif
