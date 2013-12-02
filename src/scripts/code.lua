require("bit")
Beta = require("./src/scripts/beta")

function tick(pc, opcode, regc, rega, regb, lit)
	-- get rid of supervisor bit
	pc = bit.band(pc, 0x7FFFFFFF)

	-- generate interrupt at location lab6 wants
	if pc == 1480 then
		betalib.interrupt(8)
	end

	-- pretty print executed instruction
	xp = betalib.read_reg(30)
	if Beta.opcodes[opcode] then
		name = Beta.opcodes[opcode][1]
		const = Beta.opcodes[opcode][2]

		if const then
			print(pc, name, "R"..rega, lit, "R"..regc)
		else
			print(pc, name, "R"..rega, "R"..regb, "R"..regc)
		end
	else
		print(pc, "???", "R"..rega, "R"..regb, "R"..regc)
	end

	-- if in error loop
	if pc == 16 then
		betalib.write_mem(0, 12) -- halt processor
		print("error code "..betalib.read_reg(0))
	end

	-- if in success loop
	if pc == 1520 then
		betalib.write_mem(0, 1520) -- halt processor
		print("TEST PASSED!")
	end
end
