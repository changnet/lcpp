
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

	local t2 = Test()
	t2:set_i(922222)
	t2:set_s("222229")
	print("t2 value", t2:get_i(), t2:get_s())

	dumpTest(gt:toludata()) -- !!! not dumpTest(gt)
	gt.sss()            -- !!! not gt:sss()
	gt:test_param(true, 2222, 3.3333, 44.44444, "5555555", "6666666666666", gt:toludata());
	gt:test_param()

	local TestCtor = require "TestCtor"
	local tc = TestCtor(1111111, "tctctctctc")
	tc:dump()
end

local ok, msg = xpcall(main, function(msg)
	return string.format("%s: %s", msg, debug.traceback())
end)

print(msg or "ok")
