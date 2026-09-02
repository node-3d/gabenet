{
	'variables': {
		'bin': '<!(node -e "import(\'@node-3d/addon-tools\').then((m) => m.printBin())")',
		'gns_include': '<!(node -p "require(\'@node-3d/deps-gns\').include")',
		'gns_bin': '<!(node -p "require(\'@node-3d/deps-gns\').bin")',
	},
	'targets': [{
		'target_name': 'gabenet', 'includes': ['common.gypi'], 'sources': [
			'cpp/bindings.cpp',
			'cpp/common.cpp',
			'cpp/events.cpp',
			'cpp/lifecycle.cpp',
			'cpp/message.cpp',
			'cpp/messages.cpp',
			'cpp/sockets.cpp',
			'cpp/utils.cpp',
		],
		'include_dirs': ['<(gns_include)', '<!@(node -e "import(\'@node-3d/addon-tools\').then((m) => m.printInclude())")'],
		'library_dirs': ['<(gns_bin)'],
		'conditions': [
			['OS=="linux"', { 'libraries': ["-Wl,-rpath,'$$ORIGIN'", "-Wl,-rpath,'$$ORIGIN/../node_modules/@node-3d/deps-gns/<(bin)'", "-Wl,-rpath,'$$ORIGIN/../../@node-3d/deps-gns/<(bin)'", "-Wl,-rpath,'$$ORIGIN/../../deps-gns/<(bin)'", '<(gns_bin)/libGameNetworkingSockets.so'] }],
			['OS=="mac"', { 'xcode_settings': { 'DYLIB_INSTALL_NAME_BASE': '@rpath' }, 'libraries': ['-Wl,-rpath,@loader_path', '-Wl,-rpath,@loader_path/../node_modules/@node-3d/deps-gns/<(bin)', '-Wl,-rpath,@loader_path/../../@node-3d/deps-gns/<(bin)', '-Wl,-rpath,@loader_path/../../deps-gns/<(bin)', '<(gns_bin)/libGameNetworkingSockets.dylib'] }],
			['OS=="win"', { 'libraries': ['GameNetworkingSockets.lib'] }],
		],
	}],
}
