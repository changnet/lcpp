
local function e_trace(msg)
	return string.format("%s: %s", msg, debug.traceback())
end

local function main()
	local t = newTest()
	dumpTest(t)
	test()
	print("test1 return", test1(123456, "afdsafasdfasfa"))

	local Test = require "Test"

	local t1 = Test()
	local ret = t1:set(888, "t1 lua set")
	assert(ret)
	dumpTest(t1:toludata()) -- !!! not dumpTest(t1)
	
	local vcfun = 0xFF77
	assert(vcfun == t1:static_not_cfunc(vcfun))
	assert(vcfun == Test:static_not_cfunc(vcfun))

	local t2 = Test()
	t2:set_i(922222)
	t2:set_s("222229")
	print("t2 value", t2:get_i(), t2:get_s())
	t2:test_char("a", "A")
	t2:test_char(97, 65)

	dumpTest(gt:toludata()) -- !!! not dumpTest(gt)
	gt.sss()            -- !!! not gt:sss()
	gt:test_param(true, 2222, 3.3333, 44.44444, "5555555", "6666666666666",
		gt:toludata(), "88888888", "999999999");

	-- if enable ARGS_CHECK, this should throw a error
	local ok, msg = xpcall(function()
		gt:test_param()
	end, e_trace)
	if not ok then
		print(msg)
	end

	local TestCtor = require "TestCtor"
	local tc = TestCtor(1111111, "tctctctctc")
	tc:dump()

	-- local TestCtor2 = require "TestCtor2"
	-- local gtc2 = TestCtor2() -- !!! no constructor
	gtc2:dump()

	local TestMore = require "TestMore"
	local tm = TestMore()
	tm:set_i(987563)
	print("TestMore get_i", tm:get_i())

	local v = 56312745
	assert(v == tm:lstyle(v))
end

local ok, msg = xpcall(main, e_trace)

print(msg or "ok")
