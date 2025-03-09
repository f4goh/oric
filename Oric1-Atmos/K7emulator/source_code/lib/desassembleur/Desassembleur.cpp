#include "Desassembleur.h"

constexpr size_t Desassembleur::NUMBER_OPCODES;

opcode_t Desassembleur::g_opcode_table[NUMBER_OPCODES] = {
  {0x69, "ADC", IMMED, 2, 0}, /* ADC */
  {0x65, "ADC", ZEROP, 3, 0},
  {0x75, "ADC", ZEPIX, 4, 0},
  {0x6D, "ADC", ABSOL, 4, 0},
  {0x7D, "ADC", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x79, "ADC", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x61, "ADC", INDIN, 6, 0},
  {0x71, "ADC", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0x29, "AND", IMMED, 2, 0}, /* AND */
  {0x25, "AND", ZEROP, 3, 0},
  {0x35, "AND", ZEPIX, 4, 0},
  {0x2D, "AND", ABSOL, 4, 0},
  {0x3D, "AND", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x39, "AND", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x21, "AND", INDIN, 6, 0},
  {0x31, "AND", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0x0A, "ASL", ACCUM, 2, 0}, /* ASL */
  {0x06, "ASL", ZEROP, 5, 0},
  {0x16, "ASL", ZEPIX, 6, 0},
  {0x0E, "ASL", ABSOL, 6, 0},
  {0x1E, "ASL", ABSIX, 7, 0},

  {0x90, "BCC", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BCC */

  {0xB0, "BCS", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BCS */

  {0xF0, "BEQ", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BEQ */

  {0x24, "BIT", ZEROP, 3, 0}, /* BIT */
  {0x2C, "BIT", ABSOL, 4, 0},

  {0x30, "BMI", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BMI */

  {0xD0, "BNE", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BNE */

  {0x10, "BPL", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BPL */

  {0x00, "BRK", IMPLI, 7, 0}, /* BRK */

  {0x50, "BVC", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BVC */

  {0x70, "BVS", RELAT, 2, CYCLES_CROSS_PAGE_ADDS_ONE | CYCLES_BRANCH_TAKEN_ADDS_ONE}, /* BVS */

  {0x18, "CLC", IMPLI, 2, 0}, /* CLC */

  {0xD8, "CLD", IMPLI, 2, 0}, /* CLD */

  {0x58, "CLI", IMPLI, 2, 0}, /* CLI */

  {0xB8, "CLV", IMPLI, 2, 0}, /* CLV */

  {0xC9, "CMP", IMMED, 2, 0}, /* CMP */
  {0xC5, "CMP", ZEROP, 3, 0},
  {0xD5, "CMP", ZEPIX, 4, 0},
  {0xCD, "CMP", ABSOL, 4, 0},
  {0xDD, "CMP", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0xD9, "CMP", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0xC1, "CMP", INDIN, 6, 0},
  {0xD1, "CMP", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0xE0, "CPX", IMMED, 2, 0}, /* CPX */
  {0xE4, "CPX", ZEROP, 3, 0},
  {0xEC, "CPX", ABSOL, 4, 0},

  {0xC0, "CPY", IMMED, 2, 0}, /* CPY */
  {0xC4, "CPY", ZEROP, 3, 0},
  {0xCC, "CPY", ABSOL, 4, 0},

  {0xC6, "DEC", ZEROP, 5, 0}, /* DEC */
  {0xD6, "DEC", ZEPIX, 6, 0},
  {0xCE, "DEC", ABSOL, 6, 0},
  {0xDE, "DEC", ABSIX, 7, 0},

  {0xCA, "DEX", IMPLI, 2, 0}, /* DEX */

  {0x88, "DEY", IMPLI, 2, 0}, /* DEY */

  {0x49, "EOR", IMMED, 2, 0}, /* EOR */
  {0x45, "EOR", ZEROP, 3, 0},
  {0x55, "EOR", ZEPIX, 4, 0},
  {0x4D, "EOR", ABSOL, 4, 0},
  {0x5D, "EOR", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x59, "EOR", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x41, "EOR", INDIN, 6, 1},
  {0x51, "EOR", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0xE6, "INC", ZEROP, 5, 0}, /* INC */
  {0xF6, "INC", ZEPIX, 6, 0},
  {0xEE, "INC", ABSOL, 6, 0},
  {0xFE, "INC", ABSIX, 7, 0},

  {0xE8, "INX", IMPLI, 2, 0}, /* INX */

  {0xC8, "INY", IMPLI, 2, 0}, /* INY */

  {0x4C, "JMP", ABSOL, 3, 0}, /* JMP */
  {0x6C, "JMP", INDIA, 5, 0},

  {0x20, "JSR", ABSOL, 6, 0}, /* JSR */

  {0xA9, "LDA", IMMED, 2, 0}, /* LDA */
  {0xA5, "LDA", ZEROP, 3, 0},
  {0xB5, "LDA", ZEPIX, 4, 0},
  {0xAD, "LDA", ABSOL, 4, 0},
  {0xBD, "LDA", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0xB9, "LDA", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0xA1, "LDA", INDIN, 6, 0},
  {0xB1, "LDA", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0xA2, "LDX", IMMED, 2, 0}, /* LDX */
  {0xA6, "LDX", ZEROP, 3, 0},
  {0xB6, "LDX", ZEPIY, 4, 0},
  {0xAE, "LDX", ABSOL, 4, 0},
  {0xBE, "LDX", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0xA0, "LDY", IMMED, 2, 0}, /* LDY */
  {0xA4, "LDY", ZEROP, 3, 0},
  {0xB4, "LDY", ZEPIX, 4, 0},
  {0xAC, "LDY", ABSOL, 4, 0},
  {0xBC, "LDY", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0x4A, "LSR", ACCUM, 2, 0}, /* LSR */
  {0x46, "LSR", ZEROP, 5, 0},
  {0x56, "LSR", ZEPIX, 6, 0},
  {0x4E, "LSR", ABSOL, 6, 0},
  {0x5E, "LSR", ABSIX, 7, 0},

  {0xEA, "NOP", IMPLI, 2, 0}, /* NOP */

  {0x09, "ORA", IMMED, 2, 0}, /* ORA */
  {0x05, "ORA", ZEROP, 3, 0},
  {0x15, "ORA", ZEPIX, 4, 0},
  {0x0D, "ORA", ABSOL, 4, 0},
  {0x1D, "ORA", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x19, "ORA", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x01, "ORA", INDIN, 6, 0},
  {0x11, "ORA", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0x48, "PHA", IMPLI, 3, 0}, /* PHA */

  {0x08, "PHP", IMPLI, 3, 0}, /* PHP */

  {0x68, "PLA", IMPLI, 4, 0}, /* PLA */

  {0x28, "PLP", IMPLI, 4, 0}, /* PLP */

  {0x2A, "ROL", ACCUM, 2, 0}, /* ROL */
  {0x26, "ROL", ZEROP, 5, 0},
  {0x36, "ROL", ZEPIX, 6, 0},
  {0x2E, "ROL", ABSOL, 6, 0},
  {0x3E, "ROL", ABSIX, 7, 0},

  {0x6A, "ROR", ACCUM, 2, 0}, /* ROR */
  {0x66, "ROR", ZEROP, 5, 0},
  {0x76, "ROR", ZEPIX, 6, 0},
  {0x6E, "ROR", ABSOL, 6, 0},
  {0x7E, "ROR", ABSIX, 7, 0},

  {0x40, "RTI", IMPLI, 6, 0}, /* RTI */

  {0x60, "RTS", IMPLI, 6, 0}, /* RTS */

  {0xE9, "SBC", IMMED, 2, 0}, /* SBC */
  {0xE5, "SBC", ZEROP, 3, 0},
  {0xF5, "SBC", ZEPIX, 4, 0},
  {0xED, "SBC", ABSOL, 4, 0},
  {0xFD, "SBC", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0xF9, "SBC", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0xE1, "SBC", INDIN, 6, 0},
  {0xF1, "SBC", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0x38, "SEC", IMPLI, 2, 0}, /* SEC */

  {0xF8, "SED", IMPLI, 2, 0}, /* SED */

  {0x78, "SEI", IMPLI, 2, 0}, /* SEI */

  {0x85, "STA", ZEROP, 3, 0}, /* STA */
  {0x95, "STA", ZEPIX, 4, 0},
  {0x8D, "STA", ABSOL, 4, 0},
  {0x9D, "STA", ABSIX, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x99, "STA", ABSIY, 4, CYCLES_CROSS_PAGE_ADDS_ONE},
  {0x81, "STA", INDIN, 6, 0},
  {0x91, "STA", ININD, 5, CYCLES_CROSS_PAGE_ADDS_ONE},

  {0x86, "STX", ZEROP, 3, 0}, /* STX */
  {0x96, "STX", ZEPIY, 4, 0},
  {0x8E, "STX", ABSOL, 4, 0},

  {0x84, "STY", ZEROP, 3, 0}, /* STY */
  {0x94, "STY", ZEPIX, 4, 0},
  {0x8C, "STY", ABSOL, 4, 0},

  {0xAA, "TAX", IMPLI, 2, 0}, /* TAX */

  {0xA8, "TAY", IMPLI, 2, 0}, /* TAY */

  {0xBA, "TSX", IMPLI, 2, 0}, /* TSX */

  {0x8A, "TXA", IMPLI, 2, 0}, /* TXA */

  {0x9A, "TXS", IMPLI, 2, 0}, /* TXS */

  {0x98, "TYA", IMPLI, 2, 0} /* TYA */
};


Desassembleur::Desassembleur() {
  // Initialisation des membres de la structure options_t
  options = new options_t();
  options->cycle_counting = 0;
  options->hex_output = 1;
  options->oric_mode = 1;
  options->org = 0x600;
  options->max_num_bytes = 48U * 1024U;
  options->offset = 0U;
}

Desassembleur::~Desassembleur() {
  // Implémentation du destructeur
  // Libération des ressources si nécessaire
}
/* This function appends cycle counting to the comment block. See following
   for methods used:
   "Nick Bensema's Guide to Cycle Counting on the Atari 2600"
   http://www.alienbill.com/2600/cookbook/cycles/nickb.txt
*/
char* Desassembleur::append_cycle(char* input, uint8_t entry, uint16_t pc, uint16_t new_pc) {
  char tmpstr[256];
  int cycles = g_opcode_table[entry].cycles;
  int exceptions = g_opcode_table[entry].cycles_exceptions;
  int crosses_page = ((pc & 0xff00u) != (new_pc & 0xff00u)) ? 1 : 0;

  // On some exceptional conditions, instruction will take an extra cycle, or even two
  if (exceptions != 0) {
    if ((exceptions & CYCLES_BRANCH_TAKEN_ADDS_ONE) && (exceptions & CYCLES_CROSS_PAGE_ADDS_ONE)) {
      /* Branch case: check for page crossing, since it can be determined
         statically from the relative offset and current PC.
      */
      if (crosses_page) {
        /* Crosses page, always at least 1 extra cycle, two times */
        sprintf(tmpstr, " Cycles: %d/%d", cycles + 1, cycles + 2);
      } else {
        /* Does not cross page, maybe one extra cycle if branch taken */
        sprintf(tmpstr, " Cycles: %d/%d", cycles, cycles + 1);
      }
    } else {
      /* One exception: two times, can't tell in advance whether page crossing occurs */
      sprintf(tmpstr, " Cycles: %d/%d", cycles, cycles + 1);
    }
  } else {
    /* No exceptions, no extra time */
    sprintf(tmpstr, " Cycles: %d", cycles);
  }

  strcat(input, tmpstr);
  return (input + strlen(input));
}

void Desassembleur::add_oric_str(char *instr, const char *instr2) {
  //strcat(instr, " [ORIC] ");
  strcat(instr, instr2);
}

/* This function put ORIC-specific info in the comment block*/

void Desassembleur::append_oric(char* input, uint16_t arg) {
  switch (arg) {
    case 0x300: add_oric_str(input, " VIA ORB/IRB"); break;
    case 0x301: add_oric_str(input, " VIA ORA/IRA"); break;
    case 0x302: add_oric_str(input, " VIA DDRB"); break;
    case 0x303: add_oric_str(input, " VIA DDRA"); break;
    case 0x304: add_oric_str(input, " VIA T1C-L"); break;
    case 0x305: add_oric_str(input, " VIA T1C-H"); break;
    case 0x306: add_oric_str(input, " VIA T1L-L"); break;
    case 0x307: add_oric_str(input, " VIA T1L-H"); break;
    case 0x308: add_oric_str(input, " VIA T2C-L"); break;
    case 0x309: add_oric_str(input, " VIA T2C-H"); break;
    case 0x30A: add_oric_str(input, " VIA SR"); break;
    case 0x30B: add_oric_str(input, " VIA ACR"); break;
    case 0x30C: add_oric_str(input, " VIA PCR"); break;
    case 0x30D: add_oric_str(input, " VIA IFR"); break;
    case 0x30E: add_oric_str(input, " VIA IER"); break;
    case 0x30F: add_oric_str(input, " VIA ORA/IRA"); break;
    case 0x31C: add_oric_str(input, " ACIA TX/RX REG"); break;
    case 0x31D: add_oric_str(input, " ACIA STATUS REG"); break;
    case 0x31E: add_oric_str(input, " ACIA COMMAND REG"); break;
    case 0x31F: add_oric_str(input, " ACIA CTRL REG"); break;
  }
  // lores BB80 à BFDF  hires A000 à BF3F
  if (arg >= 0xA000 && arg <= 0xBF3F)  add_oric_str(input, " SCREEN");
  if ((arg & 0xC000) == 0xC000) add_oric_str(input, " ROM");
  if (arg==0x03F3) add_oric_str(input, " EREBUS"); 
}

/* This function disassembles the opcode at the PC and outputs it in *output */
void Desassembleur::disassemble(char* output, uint8_t* buffer, uint16_t* pc) {
  char opcode_repr[64], hex_dump[64];  //avant 256----------------------------------------------
  int opcode_idx;
  int len = 0;
  int entry = 0;
  int found = 0;
  uint8_t byte_operand;
  uint16_t word_operand = 0;
  uint16_t current_addr = *pc;
  uint8_t opcode = buffer[current_addr - options->org]; //decallage
  const char *mnemonic;

  opcode_repr[0] = '\0';
  hex_dump[0] = '\0';

  // Linear search for opcode
  for (opcode_idx = 0; opcode_idx < NUMBER_OPCODES; opcode_idx++) {
    if (opcode == g_opcode_table[opcode_idx].number) {
      /* Found the opcode, record its table index */
      found = 1;
      entry = opcode_idx;
    }
  }

  // For opcode not found, terminate early
  if (!found) {
    sprintf(opcode_repr, ".byte $%02X", opcode);
    if (options->hex_output) {
      sprintf(hex_dump, "$%04X> %02X:", current_addr, opcode);
      sprintf(output, "%-16s%-16s; INVALID OPCODE !!!\n", hex_dump, opcode_repr);
    } else {
      sprintf(hex_dump, "$%04X", current_addr);
      sprintf(output, "%-8s%-16s; INVALID OPCODE !!!\n", hex_dump, opcode_repr);
    }
    return;
  }

  // Opcode found in table: disassemble properly according to addressing mode
  mnemonic = g_opcode_table[entry].mnemonic;

  // Set hex dump to default single address format. Will be overwritten
  // by more complex output in case of hex dump mode enabled
  sprintf(hex_dump, "$%04X", current_addr);

  switch (g_opcode_table[entry].addressing) {
    case IMMED:
      /* Get immediate value operand */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      sprintf(opcode_repr, "%s #$%02X", mnemonic, byte_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case ABSOL:
      /* Get absolute address operand */
      word_operand = LOAD_WORD(buffer, *pc);
      *pc += 2;

      sprintf(opcode_repr, "%s $%02X%02X", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X%02X:", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
      }

      break;
    case ZEROP:
      /* Get zero page address */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      sprintf(opcode_repr, "%s $%02X", mnemonic, byte_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case IMPLI:
      sprintf(opcode_repr, "%s", mnemonic);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X:", current_addr, opcode);
      }

      break;
    case INDIA:
      /* Get indirection address */
      word_operand = LOAD_WORD(buffer, *pc);
      *pc += 2;

      sprintf(opcode_repr, "%s ($%02X%02X)", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X%02X:", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
      }

      break;
    case ABSIX:
      /* Get base address */
      word_operand = LOAD_WORD(buffer, *pc);
      *pc += 2;

      sprintf(opcode_repr, "%s $%02X%02X,X", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X%02X:", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
      }

      break;
    case ABSIY:
      /* Get baser address */
      word_operand = LOAD_WORD(buffer, *pc);
      *pc += 2;

      sprintf(opcode_repr, "%s $%02X%02X,Y", mnemonic, HIGH_PART(word_operand), LOW_PART(word_operand));
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X%02X:", current_addr, opcode, LOW_PART(word_operand), HIGH_PART(word_operand));
      }

      break;
    case ZEPIX:
      /* Get zero-page base address */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      sprintf(opcode_repr, "%s $%02X,X", mnemonic, byte_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case ZEPIY:
      /* Get zero-page base address */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      sprintf(opcode_repr, "%s $%02X,Y", mnemonic, byte_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case INDIN:
      /* Get zero-page base address */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      sprintf(opcode_repr, "%s ($%02X,X)", mnemonic, byte_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case ININD:
      /* Get zero-page base address */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      sprintf(opcode_repr, "%s ($%02X),Y", mnemonic, byte_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case RELAT:
      /* Get relative modifier */
      byte_operand = buffer[*pc + 1 - options->org];
      *pc += 1;

      // Compute displacement from first byte after full instruction.
      word_operand = current_addr + 2;
      if (byte_operand > 0x7Fu) {
        word_operand -= ((~byte_operand & 0x7Fu) + 1);
      } else {
        word_operand += byte_operand & 0x7Fu;
      }

      sprintf(opcode_repr, "%s $%04X", mnemonic, word_operand);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X %02X:", current_addr, opcode, byte_operand);
      }

      break;
    case ACCUM:
      sprintf(opcode_repr, "%s A", mnemonic);
      if (options->hex_output) {
        sprintf(hex_dump, "$%04X> %02X:", current_addr, opcode);
      }

      break;
    default:
      // Will not happen since each entry in opcode_table has address mode set
      break;
  }

  // Emit disassembly line content, prior to annotation comments
  len = sprintf(output, DUMP_FORMAT, hex_dump, opcode_repr);
  output += len;

  /* Add cycle count if necessary */
  if (options->cycle_counting) {
    output = append_cycle(output, entry, *pc + 1, word_operand);
  }

  /* Add NES port info if necessary */
  switch (g_opcode_table[entry].addressing) {
    case ABSOL:
    case ABSIX:
    case ABSIY:
      if (options->oric_mode) {
        append_oric(output, word_operand);
      }
      break;
    default:
      /* Other addressing modes: not enough info to add NES register annotation */
      break;
  }
}

void Desassembleur::setCycleCounting(int value) {
    options->cycle_counting = value;
}

void Desassembleur::setHexOutput(int value) {
    options->hex_output = value;
}

void Desassembleur::setOricMode(int value) {
    options->oric_mode = value;
}

void Desassembleur::setOrg(uint16_t value) {
    options->org = value;
}

void Desassembleur::setMaxNumBytes(uint32_t value) {
    options->max_num_bytes = value;
}

void Desassembleur::setOffset(uint32_t value) {
    options->offset = value;
}

uint16_t Desassembleur::getOrg() const {
    return options->org;
}
