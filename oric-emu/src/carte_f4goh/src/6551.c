/*
 6551
 */

#include "6551.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


// NOTE: work with integers 109.92 and 134.58 are approximated
static uint32_t boudrates[] = { 0, 50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 19200 };

static void acia_update_params(struct acia *acia)
{
  switch(8 - ((acia->regs[ACIA_CONTROL]&ACONF_WLEN)>>5))
  {
    case 5:
      acia->bitmask = 0x1f;
      break;
    case 6:
      acia->bitmask = 0x3f;
      break;
    case 7:
      acia->bitmask = 0x7f;
      break;
    case 8:
    default:
      acia->bitmask = 0xff;
      break;
  }

  acia->rx = (acia->regs[ACIA_COMMAND] & ACOMF_DTR)? true : false;
  acia->irqrx = (acia->rx && 0 == (acia->regs[ACIA_COMMAND] & ACOMF_IRQDIS))? true : false;

  switch((acia->regs[ACIA_COMMAND] & ACOMF_TXCON) >> 2)
  {
    case 0:
      acia->tx = false;
      acia->irqtx = false;
      break;
    case 1:
      acia->tx = true;
      acia->irqtx = true;
      break;
    case 2:
      acia->tx = true;
      acia->irqtx = false;
      break;
    case 3:
      // FIXME: ACIA transmit BRK...
      acia->tx = false;
      acia->irqtx = false;
      break;
  }

  acia->echo = (acia->regs[ACIA_COMMAND]&ACOMF_ECHO)? true : false;

  // FIXME: calculate number of frame bits include 1.5 stop bits
  acia->framebits =
    (1) +                                               // start bit
    (8 - ((acia->regs[ACIA_CONTROL]&ACONF_WLEN)>>5)) +  // data bits
    ((acia->regs[ACIA_COMMAND]>>5)&1) +                 // parity bit_nums
    ((acia->regs[ACIA_CONTROL]&ACONF_STOPB) ? 2 : 1);   // stop bit(s)

  acia->boudrate = boudrates[acia->regs[ACIA_CONTROL]&ACONF_BAUD];

  // 1.8432 MHz / 16 (internal divider) = 115,2 kHz
  // 115,2 kHz => ~ 8.68 usec
  if(0 < acia->boudrate)
    acia->framecycle = (115200 * 8680) / (acia->boudrate * 1000);
  else
    acia->framecycle = 0;
}

//void acia_init(struct acia *acia, struct machine *oric)
void acia_init(struct acia *acia)
{
  if(acia->done)
    acia->done(acia);

  //acia->oric = oric;
  acia->regs[ACIA_RXDATA]  = 0;
  acia->regs[ACIA_TXDATA]  = 0;
  acia->regs[ACIA_STATUS]  = ASTF_TXEMPTY;
  acia->regs[ACIA_COMMAND] = ACOMF_IRQDIS;
  acia->regs[ACIA_CONTROL] = 0;

  acia->cycles = 0;
  acia_update_params(acia);

  acia_init_com(acia);

}
/*
void acia_clock(struct acia *acia, unsigned int cycles)
{
  // EXT clock used - not implemented
  if(0 == (acia->regs[ACIA_CONTROL] & ACONF_SRC))
    return;

  // INT clock
  if(0 == acia->framecycle)
    return;

  acia->cycles += cycles;
  while(acia->framecycle <= acia->cycles)
  {
    acia->cycles -= acia->framecycle;

    // TX enabled
    if(acia->tx && 0 == (acia->regs[ACIA_STATUS] & ASTF_TXEMPTY))
    {
      if(!acia->put_byte(acia->regs[ACIA_TXDATA]))
        acia->regs[ACIA_STATUS] |= ASTF_OVRUNERR;
      else
      {
        acia->regs[ACIA_STATUS] |= ASTF_TXEMPTY;
        if(acia->irqtx)
        {
          acia->regs[ACIA_STATUS] |= ASTF_IRQ;
          //acia->oric->cpu.irq |= IRQF_ACIA;  //----------------------------------------------irq
        }
        if(acia->echo)
          acia->put_byte(acia->regs[ACIA_TXDATA]);
      }
    }

    // RX enabled
    if(acia-> rx)
    {
      uint8_t data = 0;
      if(acia->has_byte(&data))
      {
        acia->regs[ACIA_STATUS] |= ASTF_RXDATA;
        if(acia->irqrx)
        {
          acia->regs[ACIA_STATUS] |= ASTF_IRQ;
          //acia->oric->cpu.irq |= IRQF_ACIA; //------------------------------------------------irq
        }
      }
    }
  }

  // update modem lines status
  if(acia->stat)
    acia->regs[ACIA_STATUS] = acia->stat(acia->regs[ACIA_STATUS]);
  else
    acia->regs[ACIA_STATUS] |= (ASTF_CARRIER|ASTF_DSR);
}
*/
void acia_clock(struct acia *acia, unsigned int cycles)
{
  /* EXT clock used - not implemented */
  if(0 == (acia->regs[ACIA_CONTROL] & ACONF_SRC))
    return;

  /* INT clock */
  if(0 == acia->framecycle)
    return;

  acia->cycles += cycles;

  

acia->poll_rx();   // 1 fois max par tick ACIA
 
  while(acia->framecycle <= acia->cycles)
  {
    
    acia->cycles -= acia->framecycle;

    /* =====================================
       TX
       ===================================== */

    if(acia->tx &&
       acia->put_byte &&
       0 == (acia->regs[ACIA_STATUS] & ASTF_TXEMPTY))
    {
      if(!acia->put_byte(acia->regs[ACIA_TXDATA]))
      {
        acia->regs[ACIA_STATUS] |= ASTF_OVRUNERR;
      }
      else
      {
        acia->regs[ACIA_STATUS] |= ASTF_TXEMPTY;

        if(acia->irqtx)
        {
          acia->regs[ACIA_STATUS] |= ASTF_IRQ;
        }

        if(acia->echo && acia->put_byte)
        {
          acia->put_byte(acia->regs[ACIA_TXDATA]);
        }
      }
    }

    /* =====================================
       RX
       ===================================== */

    if(acia->rx && acia->has_byte)
    {
      uint8_t data = 0;

      if(acia->has_byte(&data))
      {
        /* IMPORTANT :
           stocker le byte reçu dans RXDATA
        */
        acia->regs[ACIA_RXDATA] = data;

        acia->regs[ACIA_STATUS] |= ASTF_RXDATA;

        if(acia->irqrx)
        {
          acia->regs[ACIA_STATUS] |= ASTF_IRQ;
        }
      }
    }
  }

  /* =====================================
     modem status
     ===================================== */

  if(acia->stat)
  {
    acia->regs[ACIA_STATUS] =
      acia->stat(acia->regs[ACIA_STATUS]);
  }
  else
  {
    acia->regs[ACIA_STATUS] |=
      (ASTF_CARRIER | ASTF_DSR);
  }
}


void acia_write(struct acia *acia, uint16_t addr, uint8_t data)
{
#if DEBUG_ACIA
  //dbg_printf("ACIA write: (%04X) %02X to %s", acia->oric->cpu.pc-1, data, writereg_names[addr&3]);
#endif
  switch(addr & 3)
  {
    case ACIA_RXDATA:      // Data for TX
      acia->regs[ACIA_TXDATA] = data & acia->bitmask;
      acia->regs[ACIA_STATUS] &= ~(ASTF_TXEMPTY);
      break;

    case ACIA_STATUS:      // Reset
      acia->regs[ACIA_COMMAND] &= ACOMF_PARITY;
      acia->regs[ACIA_COMMAND] |= ACOMF_IRQDIS;
      acia->regs[ACIA_STATUS]  &= ~(ASTF_OVRUNERR);
      acia_update_params(acia);
      break;

    case ACIA_COMMAND:
      acia->regs[ACIA_COMMAND] = data;
      acia_update_params(acia);
      acia_apply_config(acia);
  

/*
#if DEBUG_ACIA
      dbg_printf("  Parity  = %s", parity_modes[data>>5]);
      dbg_printf("  RX Mode = %s", (data&ACOMF_ECHO) ? "ECHO" : "NORMAL");
      dbg_printf("  TXCon   = %s", txcon_modes[(data&ACOMF_TXCON)>>2]);
      dbg_printf("  IRQ     = %s", (data&ACOMF_IRQDIS) ? "OFF" : "ON");
      dbg_printf("  DTR     = %s", (data&ACOMF_DTR) ? "ON" : "OFF");
#endif
*/
      break;

    case ACIA_CONTROL:
      acia->regs[ACIA_CONTROL] = data;
      acia_update_params(acia);
      acia_apply_config(acia);

/*
#if DEBUG_ACIA
      dbg_printf("  Baud     = %s", baud_rates[data&ACONF_BAUD]);
      dbg_printf("  RXClock  = %s", (data&ACONF_SRC) ? "INT" : "EXT");
      dbg_printf("  Bits     = %s", bit_nums[(data&ACONF_WLEN)>>5]);
      dbg_printf("  Stopbits = %d", (data&ACONF_STOPB) ? 2 : 1);
#endif
*/
      break;
  }
}

uint8_t acia_read(struct acia *acia, uint16_t addr)
{
  uint8_t data = acia->regs[addr & 3];

  switch(addr & 3)
  {
    case ACIA_RXDATA:
        acia->get_byte(&data);
        data&=acia->bitmask;
        
      
      acia->regs[ACIA_RXDATA]=data;

      acia->regs[ACIA_STATUS] &=
        ~(ASTF_PARITYERR |
          ASTF_FRAMEERR  |
          ASTF_OVRUNERR  |
          ASTF_RXDATA);

      break;

    case ACIA_STATUS:

      /* clear IRQ */
      acia->regs[ACIA_STATUS] &= ~(ASTF_IRQ);

      break;

    default:
      break;
  }

  return data;
}

void acia_apply_config(struct acia *acia)
{
    if (!acia->set_config)
        return;

    int bits =
        8 - ((acia->regs[ACIA_CONTROL] & ACONF_WLEN) >> 5);

    int stopbits =
        (acia->regs[ACIA_CONTROL] & ACONF_STOPB) ? 2 : 1;

    char parity = 'N';

    if (acia->regs[ACIA_COMMAND] & ACOMF_PARITY)
    {
        parity =
            (acia->regs[ACIA_COMMAND] & ACOMF_PARITY)
            ? 'O'
            : 'E';
    }

    acia->set_config(
        acia->boudrate,
        bits,
        parity,
        stopbits
    );
}

// none - fake back-end, no api function will be called
bool acia_init_none(struct acia *acia)
{
  acia->done = NULL;
  acia->stat = NULL;
  acia->has_byte = NULL;
  acia->get_byte = NULL;
  acia->put_byte = NULL;
  acia->set_config = NULL;
  acia->poll_rx = NULL;
  //acia->oric->aciabackend = ACIA_TYPE_NONE;
  return true;
}
