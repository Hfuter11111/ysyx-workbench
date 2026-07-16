/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

// Execution helpers: write-back, load, store, branch, jump and system.
#define EXEC_WRITE(result) \
  do { R(rd) = (result); } while (0)

#define EXEC_ALU(op, rhs) \
  EXEC_WRITE(src1 op (rhs))

#define EXEC_MUL(result_type, src1_type, src2_type, len) \
  do { \
    result_type result; \
    result = (result_type)(src1_type)src1 * (result_type)(src2_type)src2; \
    EXEC_WRITE((word_t)(result >> len)); \
  } while (0)

#define EXEX_DIV() \
  do { \
    }
    
#define EXEC_REM(is_signed) \
  do { \
    word_t result; \
    if (src2 == 0) { \
      result = src1; \
    } else if ((is_signed) && \
               src1 == 0x80000000u && \
               src2 == 0xffffffffu) { \
      result = 0; \
    } else if (is_signed) { \
      result = (word_t)((sword_t)src1 % (sword_t)src2); \
    } else { \
      result = src1 % src2; \
    } \
    EXEC_WRITE(result); \
  } while (0)
   
#define EXEC_DIV(is_signed) \
  do { \
    word_t result; \
    if (src2 == 0) { \
      result = 0xffffffffu; \
    } else if ((is_signed) && \
               src1 == 0x80000000u && \
               src2 == 0xffffffffu) { \
      result = 0x80000000u; \
    } else if (is_signed) { \
      result = (word_t)((sword_t)src1 / (sword_t)src2); \
    } else { \
      result = src1 / src2; \
    } \
    EXEC_WRITE(result); \
  } while (0)

#define EXEC_SHIFT(op, lhs, rhs) \
  EXEC_WRITE((lhs) op ((rhs) & 0x1f))

#define EXEC_LOAD(width, type) \
  EXEC_WRITE((word_t)(type)Mr(src1 + imm, (width)))

#define EXEC_STORE(width) \
  do { Mw(src1 + imm, (width), src2); } while (0)

#define EXEC_BRANCH(cond) \
  do { s->dnpc = (cond) ? s->pc + imm : s->snpc; } while (0)

#define EXEC_JUMP(target) \
  do { \
    R(rd) = s->snpc; \
    s->dnpc = (target); \
  } while (0)

enum {
  TYPE_R, TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, TYPE_J, TYPE_B// none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT((BITS(i, 31, 31) << 20) | \
                                (BITS(i, 19, 12) << 12) | \
                                (BITS(i, 20, 20) << 11) | \
                                (BITS(i, 30, 21) <<  1), 21); } while (0)
#define immB() do { *imm = SEXT((BITS(i, 31, 31) << 12) | \
                                (BITS(i,  7,  7) << 11) | \
                                (BITS(i, 30, 25) <<  5) | \
                                (BITS(i, 11,  8) <<  1), 13); } while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_N: break;
    default: panic("unsupported type = %d", type);
  }
}

static int decode_exec(Decode *s) {
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  int rd = 0; \
  word_t src1 = 0, src2 = 0, imm = 0; \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, EXEC_ALU(+, src2));
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, EXEC_ALU(-, src2));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, EXEC_SHIFT(>>, (sword_t)src1, src2));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, EXEC_SHIFT(>>, src1, src2));
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, EXEC_SHIFT(<<, src1, src2));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, EXEC_ALU(^, src2));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, EXEC_ALU(|, src2)); 
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, EXEC_ALU(&, src2)); 
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, EXEC_WRITE((sword_t)src1 < (sword_t)src2));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, EXEC_WRITE(src1 < src2));
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, EXEC_MUL(uint64_t, word_t, word_t, 0));
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, EXEC_MUL(int64_t, sword_t, sword_t, 32));
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, EXEC_MUL(int64_t, sword_t, word_t, 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, EXEC_MUL(uint64_t, word_t, word_t, 32));    
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, EXEC_DIV(true));  
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, EXEC_DIV(false));
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, EXEC_REM(true));
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, EXEC_REM(false));
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, EXEC_ALU(+, imm));
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, EXEC_SHIFT(<<, src1, imm));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, EXEC_SHIFT(>>, src1, imm));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, EXEC_SHIFT(>>, (sword_t)src1, imm));
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, EXEC_WRITE(src1 < imm));
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, EXEC_WRITE((sword_t)src1 < (sword_t)imm));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, EXEC_ALU(^, imm));
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, EXEC_ALU(|, imm)); 
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, EXEC_ALU(&, imm));   
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, EXEC_JUMP((src1 + imm) & ~1u));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, EXEC_LOAD(4, word_t));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, EXEC_LOAD(2, uint16_t));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, EXEC_LOAD(1, uint8_t));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, EXEC_LOAD(2, int16_t));
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, EXEC_LOAD(1, int8_t));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, EXEC_STORE(1));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, EXEC_STORE(2));  
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, EXEC_STORE(4));
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, EXEC_WRITE(s->pc + imm));
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, EXEC_WRITE(imm));
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, EXEC_JUMP(s->pc + imm));
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, EXEC_BRANCH(src1 == src2));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, EXEC_BRANCH(src1 != src2));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, EXEC_BRANCH((sword_t)src1 >= (sword_t)src2));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, EXEC_BRANCH(src1 >= src2));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, EXEC_BRANCH((sword_t)src1 < (sword_t)src2));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, EXEC_BRANCH(src1 < src2)); 
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
