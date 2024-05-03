
function main()
	local t = newTest()
	dumpTest(t)
	test()
	print("test1 return", test1(123456, "afdsafasdfasfa"))

	local Test = require "Test"

	local t1 = Test()
	local ret = t1:set(888, "t1 lua set")
	assert(ret)
	dumpTest(t1:toludata()) -- !!! not dumpTest(t1)

	dumpTest(gt:toludata()) -- !!! not dumpTest(gt)
	gt.sss()            -- !!! not gt:sss()
end

local ok, msg = xpcall(main, function(msg)
	return string.format("%s: %s", msg, debug.traceback())
end)

print(msg or "ok")
