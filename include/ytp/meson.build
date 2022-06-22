version_conf_data = configuration_data()

ytp_version_array = meson.project_version().split('.')
version_conf_data.set_quoted('YTP_VERSION', meson.project_version())
version_conf_data.set('YTP_VERSION_MAJOR', ytp_version_array[0])
version_conf_data.set('YTP_VERSION_MINOR', ytp_version_array[1])
version_conf_data.set('YTP_VERSION_PATCH', ytp_version_array[2])

configure_file(
  input : 'version.h.in',
  output : 'version.h',
  configuration : version_conf_data,
  install_dir: 'include/ytp',
  install: true
)

ytp_incs = [
	'yamal.h',
	'peer.h',
	'channel.h',
	'time.h',
	'control.h',
	'sequence.h',
	'timeline.h',
]

install_headers(ytp_incs, subdir : 'ytp')
