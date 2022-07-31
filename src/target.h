#ifndef TARGET_H__HEADER_GUARD__
#define TARGET_H__HEADER_GUARD__

#include <assert.h> /* assert */
#include <stdint.h> /* uint64_t */

/* in bytes */
#define INST_SIZE 10

#define IMPLEMENTED_INSTS 44
#define IMPLEMENTED_REGS  15

typedef uint64_t word_t;

typedef enum {
/*x*/	OPCODE_NONE = 0,

/*X*/	OPCODE_MOVE,
/*X*/	OPCODE_MOVE_R,

/*X*/	OPCODE_WRITE_64,
/*X*/	OPCODE_WRITE_R_64,

/*X*/	OPCODE_WRITE_32,
/*X*/	OPCODE_WRITE_R_32,

/*X*/	OPCODE_WRITE_16,
/*X*/	OPCODE_WRITE_R_16,

/*X*/	OPCODE_WRITE_8,
/*X*/	OPCODE_WRITE_R_8,

/*X*/	OPCODE_READ_64,
/*X*/	OPCODE_READ_R_64,

/*X*/	OPCODE_READ_32,
/*X*/	OPCODE_READ_R_32,

/*X*/	OPCODE_READ_16,
/*X*/	OPCODE_READ_R_16,

/*X*/	OPCODE_READ_8,
/*X*/	OPCODE_READ_R_8,

/*x*/	OPCODE_PUSH_64,
/*x*/	OPCODE_PUSH_R_64,

/*x*/	OPCODE_PUSH_32,
/*x*/	OPCODE_PUSH_R_32,

/*x*/	OPCODE_PUSH_16,
/*x*/	OPCODE_PUSH_R_16,

/*x*/	OPCODE_PUSH_8,
/*x*/	OPCODE_PUSH_R_8,

/*x*/	OPCODE_PUSH_A,

/*x*/	OPCODE_POP_64,
/*x*/	OPCODE_POP_R_64,

/*x*/	OPCODE_POP_32,
/*x*/	OPCODE_POP_R_32,

/*x*/	OPCODE_POP_16,
/*x*/	OPCODE_POP_R_16,

/*x*/	OPCODE_POP_8,
/*x*/	OPCODE_POP_R_8,

/*x*/	OPCODE_POP_A,

/*x*/	OPCODE_EQ,
/*x*/	OPCODE_EQ_R,
/*x*/	OPCODE_NEQ,
/*x*/	OPCODE_NEQ_R,
/*x*/	OPCODE_GT,
/*x*/	OPCODE_GT_R,
/*x*/	OPCODE_GE,
/*x*/	OPCODE_GE_R,
/*x*/	OPCODE_LT,
/*x*/	OPCODE_LT_R,
/*x*/	OPCODE_LE,
/*x*/	OPCODE_LE_R,

/*x*/	OPCODE_JUMP,
/*x*/	OPCODE_JUMP_R,
/*x*/	OPCODE_JUMP_T,
/*x*/	OPCODE_JUMP_T_R,
/*x*/	OPCODE_JUMP_F,
/*x*/	OPCODE_JUMP_F_R,

/*x*/	OPCODE_ADD,
/*x*/	OPCODE_ADD_R,
	OPCODE_ADD_S,
	OPCODE_ADD_R_S,
/*x*/	OPCODE_INC,

/*x*/	OPCODE_SUB,
/*x*/	OPCODE_SUB_R,
	OPCODE_SUB_S,
	OPCODE_SUB_R_S,
/*x*/	OPCODE_DEC,

/*x*/	OPCODE_MULT,
/*x*/	OPCODE_MULT_R,
	OPCODE_MULT_S,
	OPCODE_MULT_R_S,

/*x*/	OPCODE_DIV,
/*x*/	OPCODE_DIV_R,
	OPCODE_DIV_S,
	OPCODE_DIV_R_S,

/*x*/	OPCODE_MOD,
/*x*/	OPCODE_MOD_R,
	OPCODE_MOD_S,
	OPCODE_MOD_R_S,

	OPCODE_RSHIFT,
	OPCODE_RSHIFT_R,
	OPCODE_LSHIFT,
	OPCODE_LSHIFT_R,

	OPCODE_AND,
	OPCODE_OR,
	OPCODE_NOT,

	OPCODE_BITAND,
	OPCODE_BITOR,

/*x*/	OPCODE_CALL,
/*x*/	OPCODE_CALL_R,
/*x*/	OPCODE_CALL_T,
/*x*/	OPCODE_CALL_T_R,
/*x*/	OPCODE_CALL_F,
/*x*/	OPCODE_CALL_F_R,
/*x*/	OPCODE_RET,

/*x*/	OPCODE_WRITEF,

/*x*/	OPCODE_MEMSET,
/*x*/	OPCODE_MEMCOPY,

/*x*/	OPCODE_DEBUG,

/*x*/	OPCODE_HALT = 0xFF
} opcode_t;

typedef enum {
	REG_1 = 0,
	REG_2,
	REG_3,
	REG_4,
	REG_5,
	REG_6,
	REG_7,
	REG_8,
	REG_9,
	REG_AC,
	REG_IP,
	REG_SP,
	REG_SB,
	REG_CN,
	REG_EX,
	REGS_COUNT
} reg_t;

typedef struct {
	opcode_t opcode: 8;
	reg_t    reg:    8;
	word_t   data;
} inst_t;

extern const char *g_insts[];
extern const char *g_regs[];

void test(void);

#endif
