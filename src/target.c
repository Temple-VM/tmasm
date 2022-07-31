#include "target.h"

const char *g_insts[] = {
	"move",

	"write64",
	"write32",
	"write16",
	"write8",

	"read64",
	"read32",
	"read16",
	"read8",

	"push64",
	"push32",
	"push16",
	"push8",
	"pusha",

	"pop64",
	"pop32",
	"pop16",
	"pop8",
	"popa",

	"eq",
	"neq",
	"gt",
	"ge",
	"lt",
	"le",

	"jump",
	"jumpt",
	"jumpf",

	"add",
	"inc",

	"sub",
	"dec",

	"mult",
	"div",
	"mod",

	"call",
	"callt",
	"callf",
	"ret",

	"writef",

	"memset",
	"memcopy",

	"debug",

	"halt"
};

const char *g_regs[] = {
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"R8",
	"R9",

	"AC",
	"IP",
	"SP",
	"SB",
	"CN",
	"EX"
};

void test(void) {
	assert(sizeof(g_insts) / sizeof(g_insts[0]) == IMPLEMENTED_INSTS);
	assert(sizeof(g_regs)  / sizeof(g_regs[0])  == IMPLEMENTED_REGS);
}
