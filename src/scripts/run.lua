require("bit")
Beta = require("./src/scripts/beta")

--betalib.load("src/asm/graphics.bin")

function tick(pc, opcode, regc, rega, regb, lit)
	-- get rid of supervisor bit
	pc = bit.band(pc, 0x7FFFFFFF)

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
end
