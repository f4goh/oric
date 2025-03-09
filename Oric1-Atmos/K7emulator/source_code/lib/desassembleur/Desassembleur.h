#ifndef DESASSEMBLEUR_H
#define DESASSEMBLEUR_H

//#include <cstdint>
#include <Arduino.h>

#define VERSION_INFO "v2.1"
#define NB_OPCODES 151

/* Exceptions for cycle counting */
#define CYCLES_CROSS_PAGE_ADDS_ONE      (1 << 0)
#define CYCLES_BRANCH_TAKEN_ADDS_ONE    (1 << 1)

#define DUMP_FORMAT (options->hex_output ? "%-16s%-16s;" : "%-8s%-16s;")
#define HIGH_PART(val) (((val) >> 8) & 0xFFu)
#define LOW_PART(val) ((val) & 0xFFu)
#define LOAD_WORD(buffer, current_pc) ((uint16_t)buffer[(current_pc) + 1-options->org] | (((uint16_t)buffer[(current_pc) + 2-options->org]) << 8))


/* The 6502's 13 addressing modes */
typedef enum {
  IMMED = 0, /* Immediate */
  ABSOL, /* Absolute */
  ZEROP, /* Zero Page */
  IMPLI, /* Implied */
  INDIA, /* Indirect Absolute */
  ABSIX, /* Absolute indexed with X */
  ABSIY, /* Absolute indexed with Y */
  ZEPIX, /* Zero page indexed with X */
  ZEPIY, /* Zero page indexed with Y */
  INDIN, /* Indexed indirect (with X) */
  ININD, /* Indirect indexed (with Y) */
  RELAT, /* Relative */
  ACCUM /* Accumulator */
} addressing_mode_e;

/** Some compilers don't have EOK in errno.h */
#ifndef EOK
#define EOK 0
#endif

typedef struct opcode_s {
  uint8_t number; /* Number of the opcode */
  const char *mnemonic; /* Index in the name table */
  addressing_mode_e addressing; /* Addressing mode */
  unsigned int cycles; /* Number of cycles */
  unsigned int cycles_exceptions; /* Mask of cycle-counting exceptions */
} opcode_t;

typedef struct options_s {
  char *filename; /* Input filename */
  int oric_mode; /* 1 if NES commenting and warnings are enabled */
  int cycle_counting; /* 1 if we want cycle counting */
  int hex_output; /* 1 if hex dump output is desired at beginning of line */
  unsigned long max_num_bytes;
  uint16_t org; /* Origin of addresses */
  long offset; /* File offset to start disassembly from */
} options_t;


class Desassembleur {
public:
    Desassembleur(); // Constructeur
    ~Desassembleur(); // Destructeur
    void disassemble(char* output, uint8_t* buffer, uint16_t* pc);

    void setCycleCounting(int value);
    void setHexOutput(int value);
    void setOricMode(int value);
    void setOrg(uint16_t value);
    void setMaxNumBytes(uint32_t value);
    void setOffset(uint32_t value);
    uint16_t getOrg() const; // Méthode getter pour options.org
     
private:
    char* append_cycle(char* input, uint8_t entry, uint16_t pc, uint16_t new_pc);
    void add_oric_str(char *instr, const char *instr2);
    void append_oric(char* input, uint16_t arg);
    static constexpr size_t NUMBER_OPCODES = NB_OPCODES;
    static opcode_t g_opcode_table[NUMBER_OPCODES];
    options_t* options;
};

#endif // DESASSEMBLEUR_H
