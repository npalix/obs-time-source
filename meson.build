project('obs-time-source', 'c', version : '0.2',
	default_options : ['buildtype=release', 'warning_level=3'])

deps = [
	dependency('libobs'),
	dependency('pangocairo'),
]

shared_library('time-source', 'time-source.c', dependencies : deps,
	install : true, install_dir : get_option('libdir') / 'obs-plugins',
	name_prefix : '')

install_subdir('data/locale',
	install_dir : get_option('datadir') / 'obs/obs-plugins/time-source')
