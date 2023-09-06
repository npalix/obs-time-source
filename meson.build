project('obs-time', 'c', version : '0.1',
	default_options : ['buildtype=release', 'warning_level=3'])

deps = [
	dependency('libobs'),
	dependency('pangocairo'),
]

shared_library('time-source', 'time-source.c', dependencies : deps,
	install : true, install_dir : get_option('libdir') / 'obs-plugins',
	name_prefix : '')
