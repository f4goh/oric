#include "Desassembleur.h"
#include "DesassembleurWrapper.h"

static Desassembleur* g_desassembleur = nullptr;

extern "C" {

void desassembleur_init(void)
{
    if (!g_desassembleur)
        g_desassembleur = new Desassembleur();
}

void desassembleur_shutdown(void)
{
    delete g_desassembleur;
    g_desassembleur = nullptr;
}

void desassembleur_disassemble(
    char* output,
    uint8_t* buffer,
    uint16_t* pc)
{
    g_desassembleur->disassemble(output, buffer, pc);
}

void desassembleur_setCycleCounting(int value)
{
    g_desassembleur->setCycleCounting(value);
}

void desassembleur_setHexOutput(int value)
{
    g_desassembleur->setHexOutput(value);
}

void desassembleur_setOricMode(int value)
{
    g_desassembleur->setOricMode(value);
}

void desassembleur_setOrg(uint16_t value)
{
    g_desassembleur->setOrg(value);
}

void desassembleur_setMaxNumBytes(uint32_t value)
{
    g_desassembleur->setMaxNumBytes(value);
}

void desassembleur_setOffset(uint32_t value)
{
    g_desassembleur->setOffset(value);
}

uint16_t desassembleur_getOrg(void)
{
    return g_desassembleur->getOrg();
}

}
