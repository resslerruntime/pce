/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     disasm.c                                                   *
 * Created:       2002-05-20 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2002-2003 by Hampa Hug <hampa@hampa.ch>                *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/

/* $Id: disasm.c,v 1.2 2003/04/16 02:26:17 hampa Exp $ */


#include <string.h>

#include "e8086.h"
#include "internal.h"


/**************************************************************************
 * Disassembler functions
 **************************************************************************/

static
char *d_tab_reg8[8] = {
  "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"
};

static
char *d_tab_reg16[8] = {
  "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
};

static
char *d_tab_sreg[4] = {
  "ES", "CS", "SS", "DS"
};

typedef struct {
  unsigned cnt;
  char     *arg;
} d_tab_ea_t;

static
d_tab_ea_t d_tab_ea8[32] = {
  { 1, "BYTE [BX + SI]" },
  { 1, "BYTE [BX + DI]" },
  { 1, "BYTE [BP + SI]" },
  { 1, "BYTE [BP + DI]" },
  { 1, "BYTE [SI]" },
  { 1, "BYTE [DI]" },
  { 3, "BYTE [%02X%02X]" },
  { 1, "BYTE [BX]" },
  { 2, "BYTE [BX + SI + %02X]" },
  { 2, "BYTE [BX + DI + %02X]" },
  { 2, "BYTE [BP + SI + %02X]" },
  { 2, "BYTE [BP + DI + %02X]" },
  { 2, "BYTE [SI + %02X]" },
  { 2, "BYTE [DI + %02X]" },
  { 2, "BYTE [BP + %02X]" },
  { 2, "BYTE [BX + %02X]" },
  { 3, "BYTE [BX + SI + %02X%02X]" },
  { 3, "BYTE [BX + DI + %02X%02X]" },
  { 3, "BYTE [BP + SI + %02X%02X]" },
  { 3, "BYTE [BP + DI + %02X%02X]" },
  { 3, "BYTE [SI + %02X%02X]" },
  { 3, "BYTE [DI + %02X%02X]" },
  { 3, "BYTE [BP + %02X%02X]" },
  { 3, "BYTE [BX + %02X%02X]" },
  { 1, "AL" },
  { 1, "CL" },
  { 1, "DL" },
  { 1, "BL" },
  { 1, "AH" },
  { 1, "CH" },
  { 1, "DH" },
  { 1, "BH" }
};

static
d_tab_ea_t d_tab_ea16[32] = {
  { 1, "WORD [BX + SI]" },
  { 1, "WORD [BX + DI]" },
  { 1, "WORD [BP + SI]" },
  { 1, "WORD [BP + DI]" },
  { 1, "WORD [SI]" },
  { 1, "WORD [DI]" },
  { 3, "WORD [%02X%02X]" },
  { 1, "WORD [BX]" },
  { 2, "WORD [BX + SI + %02X]" },
  { 2, "WORD [BX + DI + %02X]" },
  { 2, "WORD [BP + SI + %02X]" },
  { 2, "WORD [BP + DI + %02X]" },
  { 2, "WORD [SI + %02X]" },
  { 2, "WORD [DI + %02X]" },
  { 2, "WORD [BP + %02X]" },
  { 2, "WORD [BX + %02X]" },
  { 3, "WORD [BX + SI + %02X%02X]" },
  { 3, "WORD [BX + DI + %02X%02X]" },
  { 3, "WORD [BP + SI + %02X%02X]" },
  { 3, "WORD [BP + DI + %02X%02X]" },
  { 3, "WORD [SI + %02X%02X]" },
  { 3, "WORD [DI + %02X%02X]" },
  { 3, "WORD [BP + %02X%02X]" },
  { 3, "WORD [BX + %02X%02X]" },
  { 1, "AX" },
  { 1, "CX" },
  { 1, "DX" },
  { 1, "BX" },
  { 1, "SP" },
  { 1, "BP" },
  { 1, "SI" },
  { 1, "DI" }
};


static
unsigned disasm_reg8 (char *dst, unsigned r)
{
  strcpy (dst, d_tab_reg8[r & 7]);
  return (2);
}

static
unsigned disasm_reg16 (char *dst, unsigned r)
{
  strcpy (dst, d_tab_reg16[r & 7]);
  return (2);
}

static
unsigned disasm_sreg (char *dst, unsigned r)
{
  strcpy (dst, d_tab_sreg[r & 3]);
  return (2);
}

static
void disasm_uint16 (char *dst, unsigned short v)
{
  sprintf (dst, "%04X", v);
}

static
void disasm_imm8 (char *dst, unsigned char *src)
{
  sprintf (dst, "%02X", src[0]);
}

static
void disasm_simm8 (char *dst, unsigned char *src)
{
  if (src[0] < 0x80) {
    sprintf (dst, "+%02X", src[0]);
  }
  else {
    sprintf (dst, "-%02X", (~src[0] + 1) & 0xff);
  }
}

static
void disasm_imm16 (char *dst, unsigned char *src)
{
  sprintf (dst, "%04X", e86_mk_uint16 (src[0], src[1]));
}

static
void disasm_imm32 (char *dst, unsigned char *src)
{
  sprintf (dst, "%04X:%04X",
    e86_mk_uint16 (src[2], src[3]),
    e86_mk_uint16 (src[0], src[1])
  );
}

static
void disasm_addr16 (char *dst, unsigned char *src)
{
  sprintf (dst, "[%04X]", e86_mk_uint16 (src[0], src[1]));
}

static
unsigned disasm_ea8 (char *str, unsigned char *src)
{
  unsigned ea;

  ea = (src[0] & 7) | ((src[0] & 0xc0) >> 3);

  switch (d_tab_ea8[ea].cnt) {
    case 1:
      strcpy (str, d_tab_ea8[ea].arg);

    case 2:
      sprintf (str, d_tab_ea8[ea].arg, (unsigned) src[1]);
      break;

    case 3:
      sprintf (str, d_tab_ea8[ea].arg, (unsigned) src[2], (unsigned) src[1]);
      break;

    default:
      strcpy (str, "<bad>");
      break;
  }

  return (d_tab_ea8[ea].cnt);
}

static
unsigned disasm_ea16 (char *str, unsigned char *src)
{
  unsigned ea;

  ea = (src[0] & 7) | ((src[0] & 0xc0) >> 3);

  switch (d_tab_ea16[ea].cnt) {
    case 1:
      strcpy (str, d_tab_ea16[ea].arg);

    case 2:
      sprintf (str, d_tab_ea16[ea].arg, (unsigned) src[1]);
      break;

    case 3:
      sprintf (str, d_tab_ea16[ea].arg, (unsigned) src[2], (unsigned) src[1]);
      break;

    default:
      strcpy (str, "<bad>");
      break;
  }

  return (d_tab_ea16[ea].cnt);
}


static
void dop_ud (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "DB");
  disasm_imm8 (op->arg1, src);
}

static
void dop_ea8_reg8 (e86_disasm_t *op, unsigned char *src, char *sop)
{
  unsigned cnt;

  strcpy (op->op, sop);
  cnt = disasm_ea8 (op->arg1, src + 1);
  disasm_reg8 (op->arg2, (src[1] >> 3) & 7);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

static
void dop_ea16_reg16 (e86_disasm_t *op, unsigned char *src, char *sop)
{
  unsigned cnt;

  strcpy (op->op, sop);
  cnt = disasm_ea16 (op->arg1, src + 1);
  disasm_reg16 (op->arg2, (src[1] >> 3) & 7);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

static
void dop_reg8_ea8 (e86_disasm_t *op, unsigned char *src, char *sop)
{
  unsigned cnt;

  strcpy (op->op, sop);
  disasm_reg8 (op->arg1, (src[1] >> 3) & 7);
  cnt = disasm_ea8 (op->arg2, src + 1);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

static
void dop_reg16_ea16 (e86_disasm_t *op, unsigned char *src, char *sop)
{
  unsigned cnt;

  strcpy (op->op, sop);
  disasm_reg16 (op->arg1, (src[1] >> 3) & 7);
  cnt = disasm_ea16 (op->arg2, src + 1);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

static
void dop_ea8 (e86_disasm_t *op, unsigned char *src, char *sop)
{
  unsigned cnt;

  strcpy (op->op, sop);
  cnt = disasm_ea8 (op->arg1, src + 1);

  op->dat_n = 1 + cnt;
  op->arg_n = 1;
}

static
void dop_ea16 (e86_disasm_t *op, unsigned char *src, char *sop)
{
  unsigned cnt;

  strcpy (op->op, sop);
  cnt = disasm_ea16 (op->arg1, src + 1);

  op->dat_n = 1 + cnt;
  op->arg_n = 1;
}

/* DOP 00: ADD r/m8, reg8 */
static
void dop_00 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "ADD");
}

/* DOP 01: ADD r/m16, reg16 */
static
void dop_01 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "ADD");
}

/* DOP 02: ADD reg8, r/m8 */
static
void dop_02 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "ADD");
}

/* DOP 03: ADD reg16, r/m16 */
static void dop_03 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "ADD");
}

/* DOP 04: ADD AL, data8 */
static void dop_04 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "ADD");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 05: ADD AX, data16 */
static void dop_05 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "ADD");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 06: PUSH ES */
static void dop_06 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "PUSH");
  strcpy (op->arg1, "ES");
}

/* DOP 07: POP ES */
static void dop_07 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "POP");
  strcpy (op->arg1, "ES");
}

/* DOP 08: OR r/m8, reg8 */
static void dop_08 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "OR");
}

/* DOP 09: OR r/m16, reg16 */
static void dop_09 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "OR");
}

/* DOP 0A: OR reg8, r/m8 */
static void dop_0a (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "OR");
}

/* DOP 0B: OR reg16, r/m16 */
static void dop_0b (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "OR");
}

/* DOP 0C: OR AL, data8 */
static void dop_0c (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "OR");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 0D: OR AX, data16 */
static void dop_0d (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "OR");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 0E: PUSH CS */
static void dop_0e (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "PUSH");
  strcpy (op->arg1, "CS");
}

/* DOP 0F: POP ES */
static void dop_0f (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "POP");
  strcpy (op->arg1, "CS");
}

/* DOP 10: ADC r/m8, reg8 */
static void dop_10 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "ADC");
}

/* DOP 11: ADC r/m16, reg16 */
static void dop_11 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "ADC");
}

/* DOP 12: ADC reg8, r/m8 */
static void dop_12 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "ADC");
}

/* DOP 13: ADC reg16, r/m16 */
static void dop_13 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "ADC");
}

/* DOP 14: ADC AL, data8 */
static void dop_14 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "ADC");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 15: ADC AX, data16 */
static void dop_15 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "ADC");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 16: PUSH SS */
static void dop_16 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "PUSH");
  strcpy (op->arg1, "SS");
}

/* DOP 17: POP SS */
static void dop_17 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "POP");
  strcpy (op->arg1, "SS");
}

/* DOP 18: SBB r/m8, reg8 */
static
void dop_18 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "SBB");
}

/* DOP 19: SBB r/m16, reg16 */
static
void dop_19 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "SBB");
}

/* DOP 1A: SBB reg8, r/m8 */
static
void dop_1a (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "SBB");
}

/* DOP 1B: SBB reg16, r/m16 */
static void dop_1b (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "SBB");
}

/* DOP 1C: SBB AL, data8 */
static void dop_1c (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "SBB");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 1D: SBB AX, data16 */
static void dop_1d (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "SBB");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 1E: PUSH DS */
static void dop_1e (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "PUSH");
  strcpy (op->arg1, "DS");
}

/* DOP 1F: POP SS */
static void dop_1f (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "POP");
  strcpy (op->arg1, "DS");
}

/* DOP 20: AND r/m8, reg8 */
static void dop_20 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "AND");
}

/* DOP 21: AND r/m16, reg16 */
static void dop_21 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "AND");
}

/* DOP 22: AND reg8, r/m8 */
static void dop_22 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "AND");
}

/* DOP 23: AND reg16, r/m16 */
static void dop_23 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "AND");
}

/* DOP 24: AND AL, data8 */
static void dop_24 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "AND");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 25: AND AX, data16 */
static void dop_25 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "AND");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 26: ES: */
static
void dop_26 (e86_disasm_t *op, unsigned char *src)
{
  char tmp[256];

  e86_disasm (op, src + 1, op->ip);

  strcpy (tmp, op->op);
  strcpy (op->op, "ES: ");
  strcat (op->op, tmp);

  op->dat_n += 1;
}

/* DOP 27: DAA */
static
void dop_27 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;
  strcpy (op->op, "SUB");
}

/* DOP 28: SUB r/m8, reg8 */
static void dop_28 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "SUB");
}

/* DOP 29: SUB r/m16, reg16 */
static void dop_29 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "SUB");
}

/* DOP 2A: SUB reg8, r/m8 */
static void dop_2a (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "SUB");
}

/* DOP 2B: SUB reg16, r/m16 */
static void dop_2b (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "SUB");
}

/* DOP 2C: SUB AL, data8 */
static void dop_2c (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "SUB");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 2D: SUB AX, data16 */
static void dop_2d (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "SUB");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 2E: CS: */
static
void dop_2e (e86_disasm_t *op, unsigned char *src)
{
  char tmp[256];

  e86_disasm (op, src + 1, op->ip);

  strcpy (tmp, op->op);
  strcpy (op->op, "CS: ");
  strcat (op->op, tmp);

  op->dat_n += 1;
}

/* DOP 30: XOR r/m8, reg8 */
static void dop_30 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "XOR");
}

/* DOP 31: XOR r/m16, reg16 */
static void dop_31 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "XOR");
}

/* DOP 32: XOR reg8, r/m8 */
static void dop_32 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "XOR");
}

/* DOP 33: XOR reg16, r/m16 */
static void dop_33 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "XOR");
}

/* DOP 34: XOR AL, data8 */
static void dop_34 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "XOR");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 35: XOR AX, data16 */
static void dop_35 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "XOR");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 36: SS: */
static
void dop_36 (e86_disasm_t *op, unsigned char *src)
{
  char tmp[256];

  e86_disasm (op, src + 1, op->ip);

  strcpy (tmp, op->op);
  strcpy (op->op, "SS: ");
  strcat (op->op, tmp);

  op->dat_n += 1;
}

/* DOP 38: CMP r/m8, reg8 */
static void dop_38 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "CMP");
}

/* DOP 39: CMP r/m16, reg16 */
static void dop_39 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "CMP");
}

/* DOP 3A: CMP reg8, r/m8 */
static void dop_3a (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "CMP");
}

/* DOP 3B: CMP reg16, r/m16 */
static void dop_3b (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "CMP");
}

/* DOP 3C: CMP AL, data8 */
static void dop_3c (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "CMP");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP 3D: CMP AX, data16 */
static void dop_3d (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "CMP");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP 4x: INC reg16 */
static void dop_40 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "INC");
  disasm_reg16 (op->arg1, src[0] & 7);
}

/* DOP 4x: DEC reg16 */
static void dop_48 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "DEC");
  disasm_reg16 (op->arg1, src[0] & 7);
}

/* DOP 5x: PUSH reg16 */
static void dop_50 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "PUSH");
  disasm_reg16 (op->arg1, src[0] & 7);
}

/* DOP 5x: POP reg16 */
static void dop_58 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 1;

  strcpy (op->op, "POP");
  disasm_reg16 (op->arg1, src[0] & 7);
}

static char *d_tab_70[16] = {
"JO", "JNO", "JC",  "JNC", "JZ", "JNZ", "JBE", "JA",
"JS", "JNS", "JPE", "JPO", "JL", "JGE", "JLE", "JG"
};

/* DOP 7x: Jxx imm8 */
static void dop_70 (e86_disasm_t *op, unsigned char *src)
{
  unsigned short t;

  op->dat_n = 2;
  op->arg_n = 1;

  t = 2 + e86_add_sint8 (op->ip, src[1]);

  strcpy (op->op, d_tab_70[src[0] & 15]);
  disasm_uint16 (op->arg1, t);
}

static char *d_tab_80[16] = {
  "ADD", "OR",  "ADC", "SBB",
  "AND", "SUB", "XOR", "CMP"
};

/* DOP 80: xxx r/m8, imm8 */
static void dop_80 (e86_disasm_t *op, unsigned char *src)
{
  unsigned cnt;

  strcpy (op->op, d_tab_80[(src[1] >> 3) & 7]);
  cnt = disasm_ea8 (op->arg1, src + 1);
  disasm_imm8 (op->arg2, src + 1 + cnt);

  op->dat_n = 2 + cnt;
  op->arg_n = 2;
}

/* DOP 81: xxx r/m16, imm16 */
static void dop_81 (e86_disasm_t *op, unsigned char *src)
{
  unsigned cnt;

  strcpy (op->op, d_tab_80[(src[1] >> 3) & 7]);
  cnt = disasm_ea16 (op->arg1, src + 1);
  disasm_imm16 (op->arg2, src + 1 + cnt);

  op->dat_n = 3 + cnt;
  op->arg_n = 2;
}

/* DOP 83: xxx r/m16, imm8 */
static void dop_83 (e86_disasm_t *op, unsigned char *src)
{
  unsigned cnt;

  strcpy (op->op, d_tab_80[(src[1] >> 3) & 7]);
  cnt = disasm_ea16 (op->arg1, src + 1);
  disasm_simm8 (op->arg2, src + 1 + cnt);

  op->dat_n = 2 + cnt;
  op->arg_n = 2;
}

/* DOP 86: XCHG r/m8, reg8 */
static void dop_86 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "XCHG");
}

/* DOP 87: XCHG r/m16, reg16 */
static void dop_87 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "XCHG");
}

/* DOP 88: MOV r/m8, reg8 */
static void dop_88 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea8_reg8 (op, src, "MOV");
}

/* DOP 89: MOV r/m16, reg16 */
static void dop_89 (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16_reg16 (op, src, "MOV");
}

/* DOP 8A: MOV reg8, r/m8 */
static void dop_8a (e86_disasm_t *op, unsigned char *src)
{
  dop_reg8_ea8 (op, src, "MOV");
}

/* DOP 8B: MOV reg16, r/m16 */
static void dop_8b (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "MOV");
}

/* DOP 8C: MOV r/m16, sreg */
static void dop_8c (e86_disasm_t *op, unsigned char *src)
{
  unsigned cnt;

  strcpy (op->op, "MOV");
  cnt = disasm_ea16 (op->arg1, src + 1);
  disasm_sreg (op->arg2, (src[1] >> 3) & 3);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

/* DOP 8D: LEA reg16, r/m16 */
static void dop_8d (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "LEA");
}

/* DOP 8E: MOV sreg, r/m16 */
static void dop_8e (e86_disasm_t *op, unsigned char *src)
{
  unsigned cnt;

  strcpy (op->op, "MOV");
  disasm_sreg (op->arg1, (src[1] >> 3) & 3);
  cnt = disasm_ea16 (op->arg2, src + 1);

  op->dat_n = cnt + 1;
  op->arg_n = 2;
}

/* DOP 8F: POP r/m16 */
static void dop_8f (e86_disasm_t *op, unsigned char *src)
{
  dop_ea16 (op, src, "POP");
}

/* DOP 90: NOP */
static void dop_90 (e86_disasm_t *op, unsigned char *src)
{
  strcpy (op->op, "NOP");

  op->dat_n = 1;
  op->arg_n = 0;
}

/* DOP 9x: XCHG AX, reg16 */
static void dop_91 (e86_disasm_t *op, unsigned char *src)
{
  strcpy (op->op, "XCHG");
  strcpy (op->arg1, "AX");
  disasm_reg16 (op->arg2, src[0] & 7);

  op->dat_n = 1;
  op->arg_n = 2;
}

/* DOP 98: CBW */
static void dop_98 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "CBW");
}

/* DOP 99: CWD */
static void dop_99 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "CWD");
}

/* DOP 9A: CALL imm32 */
static
void dop_9a (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 5;
  op->arg_n = 1;

  strcpy (op->op, "CALL");
  disasm_imm32 (op->arg1, src + 1);
}

/* DOP 9C: PUSHF */
static void dop_9c (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "PUSHF");
}

/* DOP 9D: POPF */
static void dop_9d (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "POPF");
}

/* DOP 9E: SAHF */
static void dop_9e (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "SAHF");
}

/* DOP 9F: LAHF */
static void dop_9f (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "LAHF");
}

/* DOP A0: MOV AL, [data16] */
static void dop_a0 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "MOV");
  strcpy (op->arg1, "AL");
  disasm_addr16 (op->arg2, src + 1);
}

/* DOP A1: MOV AX, [data16] */
static void dop_a1 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "MOV");
  strcpy (op->arg1, "AX");
  disasm_addr16 (op->arg2, src + 1);
}

/* DOP A2: MOV [data16], AL */
static void dop_a2 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "MOV");
  disasm_addr16 (op->arg1, src + 1);
  strcpy (op->arg2, "AL");
}

/* DOP A3: MOV [data16], AX */
static void dop_a3 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "MOV");
  disasm_addr16 (op->arg1, src + 1);
  strcpy (op->arg2, "AX");
}

/* DOP A4: MOVSW */
static
void dop_a4 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "MOVSB");
}

/* DOP A5: MOVSW */
static
void dop_a5 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "MOVSW");
}

/* DOP A6: CMPSB */
static
void dop_a6 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "CMPSB");
}

/* DOP A7: CMPSW */
static
void dop_a7 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "CMPSW");
}

/* DOP A8: TEST AL, data8 */
static void dop_a8 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "TEST");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP A9: TEST AX, data16 */
static void dop_a9 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "TEST");
  strcpy (op->arg1, "AX");
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP AA: STOSB */
static
void dop_aa (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "STOSB");
}

/* DOP AB: STOSW */
static
void dop_ab (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "STOSW");
}

/* DOP AC: LODSB */
static
void dop_ac (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "LODSB");
}

/* DOP AD: LODSW */
static
void dop_ad (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "LODSW");
}

/* DOP Bx: MOV reg8, imm8 */
static void dop_b0 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "MOV");
  disasm_reg8 (op->arg1, src[0] & 7);
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP Bx: MOV reg16, imm16 */
static void dop_b8 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 2;

  strcpy (op->op, "MOV");
  disasm_reg16 (op->arg1, src[0] & 7);
  disasm_imm16 (op->arg2, src + 1);
}

/* DOP C2: RET imm16 */
static void dop_c2 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 1;

  strcpy (op->op, "RETN");
  disasm_imm16 (op->arg1, src + 1);
}

/* DOP C3: RET */
static void dop_c3 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "RETN");
}

/* DOP C4: LES reg16, r/m16 */
static void dop_c4 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "LES");
}

/* DOP C5: LDS reg16, r/m16 */
static void dop_c5 (e86_disasm_t *op, unsigned char *src)
{
  dop_reg16_ea16 (op, src, "LDS");
}

/* DOP C6: MOV r/m8, imm8 */
static void dop_c6 (e86_disasm_t *op, unsigned char *src)
{
  strcpy (op->op, "MOV");
  op->dat_n = disasm_ea8 (op->arg1, src + 1);
  disasm_imm8 (op->arg2, src + 1 + op->dat_n);

  op->dat_n += 2;
  op->arg_n = 2;
}

/* DOP C7: MOV r/m16, imm16 */
static void dop_c7 (e86_disasm_t *op, unsigned char *src)
{
  strcpy (op->op, "MOV");
  op->dat_n = disasm_ea16 (op->arg1, src + 1);
  disasm_imm16 (op->arg2, src + 1 + op->dat_n);

  op->dat_n += 3;
  op->arg_n = 2;
}

/* DOP CA: RETF imm16 */
static void dop_ca (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 3;
  op->arg_n = 1;

  strcpy (op->op, "RETF");
  disasm_imm16 (op->arg1, src + 1);
}

/* DOP CB: RETF */
static
void dop_cb (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "RETF");
}

/* DOP CD: INT imm8 */
static void dop_cd (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 1;

  strcpy (op->op, "INT");
  disasm_imm8 (op->arg1, src + 1);
}

/* DOP CF: IRET */
static
void dop_cf (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "IRET");
}

static char *d_tab_d0[8] = {
  "ROL", "ROR", "RCL", "RCR",
  "SHL", "SHR", "???",  "SAR"
};

/* DOP D0: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, 1 */
static void dop_d0 (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  strcpy (op->op, d_tab_d0[xop]);
  cnt = disasm_ea8 (op->arg1, src + 1);
  strcpy (op->arg2, "1");

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

/* DOP D1: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, 1 */
static void dop_d1 (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  strcpy (op->op, d_tab_d0[xop]);
  cnt = disasm_ea16 (op->arg1, src + 1);
  strcpy (op->arg2, "1");

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

static char *d_tab_d2[8] = {
  "ROL", "ROR", "RCL", "RCR",
  "SHL", "SHR", "???",  "SAR"
};

/* DOP D2: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m8, CL */
static void dop_d2 (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  strcpy (op->op, d_tab_d2[xop]);
  cnt = disasm_ea8 (op->arg1, src + 1);
  disasm_reg8 (op->arg2, E86_REG_CL);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

/* DOP D3: ROL, ROR, RCL, RCR, SHL, SHR, SAR r/m16, CL */
static void dop_d3 (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  strcpy (op->op, d_tab_d2[xop]);
  cnt = disasm_ea16 (op->arg1, src + 1);
  disasm_reg8 (op->arg2, E86_REG_CL);

  op->dat_n = 1 + cnt;
  op->arg_n = 2;
}

/* DOP D4: AAM imm8 */
static void dop_d4 (e86_disasm_t *op, unsigned char *src)
{
  strcpy (op->op, "AAM");
  disasm_imm8 (op->arg1, src + 1);

  op->dat_n = 2;
  op->arg_n = 1;
}

/* DOP D7: XLAT */
static
void dop_d7 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, "XLAT");
}

static char *d_tab_e0[4] = {
  "LOOPNZ", "LOOPZ", "LOOP", "JCXZ"
};

/* DOP E0: LOOPNZ imm8 */
static void dop_e0 (e86_disasm_t *op, unsigned char *src)
{
  unsigned short t;

  op->dat_n = 2;
  op->arg_n = 1;

  t = 2 + e86_add_sint8 (op->ip, src[1]);

  strcpy (op->op, d_tab_e0[src[0] - 0xe0]);
  disasm_uint16 (op->arg1, t);
}

/* DOP E4: IN AL, imm8 */
static void dop_e4 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "IN");
  strcpy (op->arg1, "AL");
  disasm_imm8 (op->arg2, src + 1);
}

/* DOP E6: OUT imm8, AL */
static void dop_e6 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 2;
  op->arg_n = 2;

  strcpy (op->op, "OUT");
  disasm_imm8 (op->arg1, src + 1);
  strcpy (op->arg2, "AL");
}

/* DOP E8: CALL imm16 */
static
void dop_e8 (e86_disasm_t *op, unsigned char *src)
{
  unsigned short t;

  op->dat_n = 3;
  op->arg_n = 1;

  t = (op->ip + e86_mk_uint16 (src[1], src[2]) + 3) & 0xffff;

  strcpy (op->op, "CALL");
  disasm_uint16 (op->arg1, t);
}

/* DOP E9: JMP imm16 */
static void dop_e9 (e86_disasm_t *op, unsigned char *src)
{
  unsigned short t;

  op->dat_n = 3;
  op->arg_n = 1;

  t = (op->ip + e86_mk_uint16 (src[1], src[2]) + 3) & 0xffff;

  strcpy (op->op, "JMPN");
  disasm_uint16 (op->arg1, t);
}

/* DOP EA: JMP imm32 */
static void dop_ea (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 5;
  op->arg_n = 1;

  strcpy (op->op, "JMPF");
  disasm_imm32 (op->arg1, src + 1);
}

/* DOP EB: JMP imm8 */
static void dop_eb (e86_disasm_t *op, unsigned char *src)
{
  unsigned short t;

  op->dat_n = 2;
  op->arg_n = 1;

  t = e86_add_sint8 (op->ip + 2, src[1]);

  strcpy (op->op, "JMPS");
  disasm_uint16 (op->arg1, t);
}

/* DOP EC: IN AL, DX */
static void dop_ec (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 2;

  strcpy (op->op, "IN");
  strcpy (op->arg1, "AL");
  strcpy (op->arg2, "DX");
}

/* DOP EE: OUT DX, AL */
static void dop_ee (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 2;

  strcpy (op->op, "OUT");
  strcpy (op->arg1, "DX");
  strcpy (op->arg2, "AL");
}

/* DOP F2: REPNE */
static
void dop_f2 (e86_disasm_t *op, unsigned char *src)
{
  char tmp[256];

  e86_disasm (op, src + 1, op->ip);

  strcpy (tmp, op->op);
  strcpy (op->op, "REPNE ");
  strcat (op->op, tmp);

  op->dat_n += 1;
}

/* DOP F3: REP */
static
void dop_f3 (e86_disasm_t *op, unsigned char *src)
{
  char tmp[256];

  e86_disasm (op, src + 1, op->ip);

  strcpy (tmp, op->op);
  strcpy (op->op, "REP ");
  strcat (op->op, tmp);

  op->dat_n += 1;
}

/* DOP F5: CMC */
static
void dop_f5 (e86_disasm_t *op, unsigned char *src)
{
  strcpy (op->op, "CMC");

  op->dat_n = 1;
  op->arg_n = 0;
}

/* DOP F6: xxx r/m8 */
static
void dop_f6 (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  switch (xop) {
    case 0:
      strcpy (op->op, "TEST");
      cnt = disasm_ea8 (op->arg1, src + 1);
      disasm_imm8 (op->arg2, src + 1 + cnt);
      op->dat_n = cnt + 2;
      op->arg_n = 2;
      return;

    case 1:
      dop_ud (op, src);
      return;

    case 2:
      dop_ea8 (op, src, "NOT");
      return;

    case 3:
      dop_ea8 (op, src, "NEG");
      return;

    case 4:
      dop_ea8 (op, src, "MUL");
      return;

    case 5:
      dop_ea8 (op, src, "IMUL");
      return;

    case 6:
      dop_ea8 (op, src, "DIV");
      return;

    case 7:
      dop_ea8 (op, src, "IDIV");
      return;

    default:
      dop_ud (op, src);
      return;
  }
}

/* DOP F7: xxx r/m16 */
static
void dop_f7 (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  switch (xop) {
    case 0:
      strcpy (op->op, "TEST");
      cnt = disasm_ea16 (op->arg1, src + 1);
      disasm_imm16 (op->arg2, src + 1 + cnt);
      op->dat_n = cnt + 3;
      op->arg_n = 2;
      return;

    case 1:
      dop_ud (op, src);
      return;

    case 2:
      dop_ea16 (op, src, "NOT");
      return;

    case 3:
      dop_ea16 (op, src, "NEG");
      return;

    case 4:
      dop_ea16 (op, src, "MUL");
      return;

    case 5:
      dop_ea16 (op, src, "IMUL");
      return;

    case 6:
      dop_ea16 (op, src, "DIV");
      return;

    case 7:
      dop_ea16 (op, src, "IDIV");
      return;

    default:
      dop_ud (op, src);
      return;
  }
}

static char *d_tab_f8[6] = {
  "CLC", "STC", "CLI", "STI", "CLD", "STD"
};

/* DOP F8-FD: CLC, STC, CLI, STI, CLD, STD */
static void dop_f8 (e86_disasm_t *op, unsigned char *src)
{
  op->dat_n = 1;
  op->arg_n = 0;

  strcpy (op->op, d_tab_f8[src[0] - 0xf8]);
}

/* DOP FE: INC, DEC r/m8 */
static void dop_fe (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop, cnt;

  xop = (src[1] >> 3) & 7;

  switch (xop) {
    case 0: /* INC r/m8 */
      strcpy (op->op, "INC");
      cnt = disasm_ea8 (op->arg1, src + 1);
      op->dat_n = cnt + 1;
      op->arg_n = 1;
      return;

    case 1: /* DEC r/m8 */
      strcpy (op->op, "DEC");
      cnt = disasm_ea8 (op->arg1, src + 1);
      op->dat_n = cnt + 1;
      op->arg_n = 1;
      return;

    default:
      dop_ud (op, src);
      return;
  }
}

/* DOP FF: xxx r/m16 */
static
void dop_ff (e86_disasm_t *op, unsigned char *src)
{
  unsigned xop;

  xop = (src[1] >> 3) & 7;

  switch (xop) {
    case 0:
      dop_ea16 (op, src, "INC");
      return;

    case 1:
      dop_ea16 (op, src, "DEC");
      return;

    case 2:
      dop_ea16 (op, src, "CALL");
      return;

    case 3:
      dop_ea16 (op, src, "CALLF");
      return;

    case 4:
      dop_ea16 (op, src, "JMP");
      return;

    case 5:
      dop_ea16 (op, src, "JMPF");
      return;

    case 6:
      dop_ea16 (op, src, "PUSH");
      return;

    case 7:
      dop_ud (op, src);
      return;

    default:
      dop_ud (op, src);
      return;
  }
}


typedef void (*e86_disasm_f) (e86_disasm_t *op, unsigned char *src);

static
e86_disasm_f dop_list[256] = {
  &dop_00, &dop_01, &dop_02, &dop_03, &dop_04, &dop_05, &dop_06, &dop_07,
  &dop_08, &dop_09, &dop_0a, &dop_0b, &dop_0c, &dop_0d, &dop_0e, &dop_0f,
  &dop_10, &dop_11, &dop_12, &dop_13, &dop_14, &dop_15, &dop_16, &dop_17,
  &dop_18, &dop_19, &dop_1a, &dop_1b, &dop_1c, &dop_1d, &dop_1e, &dop_1f,
  &dop_20, &dop_21, &dop_22, &dop_23, &dop_24, &dop_25, &dop_26, &dop_27, /* 20 */
  &dop_28, &dop_29, &dop_2a, &dop_2b, &dop_2c, &dop_2d, &dop_2e, &dop_ud,
  &dop_30, &dop_31, &dop_32, &dop_33, &dop_34, &dop_35, &dop_36, &dop_ud, /* 30 */
  &dop_38, &dop_39, &dop_3a, &dop_3b, &dop_3c, &dop_3d, &dop_ud, &dop_ud,
  &dop_40, &dop_40, &dop_40, &dop_40, &dop_40, &dop_40, &dop_40, &dop_40, /* 40 */
  &dop_48, &dop_48, &dop_48, &dop_48, &dop_48, &dop_48, &dop_48, &dop_48,
  &dop_50, &dop_50, &dop_50, &dop_50, &dop_50, &dop_50, &dop_50, &dop_50, /* 50 */
  &dop_58, &dop_58, &dop_58, &dop_58, &dop_58, &dop_58, &dop_58, &dop_58,
  &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, /* 60 */
  &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud,
  &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, /* 70 */
  &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, &dop_70, &dop_70,
  &dop_80, &dop_81, &dop_80, &dop_83, &dop_ud, &dop_ud, &dop_86, &dop_87, /* 80 */
  &dop_88, &dop_89, &dop_8a, &dop_8b, &dop_8c, &dop_8d, &dop_8e, &dop_8f,
  &dop_90, &dop_91, &dop_91, &dop_91, &dop_91, &dop_91, &dop_91, &dop_91, /* 90 */
  &dop_98, &dop_99, &dop_9a, &dop_ud, &dop_9c, &dop_9d, &dop_9e, &dop_9f,
  &dop_a0, &dop_a1, &dop_a2, &dop_a3, &dop_a4, &dop_a5, &dop_a6, &dop_a7, /* A0 */
  &dop_a8, &dop_a9, &dop_aa, &dop_ab, &dop_ac, &dop_ad, &dop_ud, &dop_ud,
  &dop_b0, &dop_b0, &dop_b0, &dop_b0, &dop_b0, &dop_b0, &dop_b0, &dop_b0, /* B0 */
  &dop_b8, &dop_b8, &dop_b8, &dop_b8, &dop_b8, &dop_b8, &dop_b8, &dop_b8,
  &dop_ud, &dop_ud, &dop_c2, &dop_c3, &dop_c4, &dop_c5, &dop_c6, &dop_c7, /* C0 */
  &dop_ud, &dop_ud, &dop_ca, &dop_cb, &dop_ud, &dop_cd, &dop_ud, &dop_cf,
  &dop_d0, &dop_d1, &dop_d2, &dop_d3, &dop_d4, &dop_ud, &dop_ud, &dop_d7, /* D0 */
  &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud, &dop_ud,
  &dop_e0, &dop_e0, &dop_e0, &dop_e0, &dop_e4, &dop_ud, &dop_e6, &dop_ud, /* E0 */
  &dop_e8, &dop_e9, &dop_ea, &dop_eb, &dop_ec, &dop_ud, &dop_ee, &dop_ud,
  &dop_ud, &dop_ud, &dop_f2, &dop_f3, &dop_ud, &dop_f5, &dop_f6, &dop_f7, /* F0 */
  &dop_f8, &dop_f8, &dop_f8, &dop_f8, &dop_f8, &dop_f8, &dop_fe, &dop_ff
};

void e86_disasm (e86_disasm_t *op, unsigned char *src, unsigned short ip)
{
  unsigned i;
  unsigned opc;

  op->ip = ip;

  opc = src[0];
  dop_list[opc] (op, src);

  for (i = 0; i < op->dat_n; i++) {
    op->dat[i] = src[i];
  }
}

void e86_disasm_mem (e8086_t *c, e86_disasm_t *op, unsigned short cs, unsigned short ip)
{
  unsigned i;
  unsigned char src[16];

  for (i = 0; i < 16; i++) {
    src[i] = e86_get_mem8 (c, cs, (ip + i) & 0xffff);
  }

  e86_disasm (op, src, ip);
}

void e86_disasm_cur (e8086_t *c, e86_disasm_t *op)
{
  e86_disasm_mem (c, op, c->sreg[E86_REG_CS], c->ip);
}
