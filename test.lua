package.cpath = package.cpath .. ";./?.dll;./?.dylib"

local hive = require "hive"

hive.start {
	thread = 4,
	main = "test.main",
}

