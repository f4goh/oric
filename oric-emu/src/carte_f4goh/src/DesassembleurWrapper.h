#ifndef DESASSEMBLEUR_WRAPPER_H
#define DESASSEMBLEUR_WRAPPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void desassembleur_init(void);
void desassembleur_shutdown(void);

void desassembleur_disassemble(
    char* output,
    uint8_t* buffer,
    uint16_t* pc);

void desassembleur_setCycleCounting(int value);
void desassembleur_setHexOutput(int value);
void desassembleur_setOricMode(int value);
void desassembleur_setOrg(uint16_t value);
void desassembleur_setMaxNumBytes(uint32_t value);
void desassembleur_setOffset(uint32_t value);

uint16_t desassembleur_getOrg(void);

#ifdef __cplusplus
}
#endif

#endif
