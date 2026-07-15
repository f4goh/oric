#include "6551.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"

#ifdef BACKEND_COM

static int com_baudrate = 115200;
static int com_bits = 8;
static char com_parity = 'N';
static int com_stopbits = 1;
static char com_name[ACIA_BACKEND_NAME_LEN];

/* =========================================================
   FIFO RX simulée
   ========================================================= */

#define COM_FIFO_SIZE 4096

static int com_fifo = 0;
static uint8_t com_fifo_buf[COM_FIFO_SIZE];

//static int com_fd = -1;
static int com_param = 0;
static bool uart_ready = false;



/* =========================================================
   Forward declarations
   ========================================================= */

static bool com_validate_param(void);
static bool com_open(void);
static bool com_close(void);
static bool com_write(uint8_t data);
static bool com_read(uint8_t* data);
static bool com_peek(uint8_t* data);
static bool com_set_config(uint32_t baud,int bits,char parity,int stopbits);
static void com_poll_rx(void);

/* =========================================================
   Backend callbacks
   ========================================================= */

static void com_done(struct acia* acia)
{
  //printf("[6551] backend shutdown\n");

  com_close();
  acia_init_none(acia);
}

static uint8_t com_stat(uint8_t stat)
{
  return (stat | (ASTF_CARRIER | ASTF_DSR));
}
/*
static bool com_has_byte(uint8_t* data)
{
    return fifo_peek(data);
}
*/


static bool com_has_byte(uint8_t* data)
{
    return com_peek(data);
}


static bool com_get_byte(uint8_t* data)
{
  return com_read(data);
}

static bool com_put_byte(uint8_t data)
{
  return com_write(data);
}

/* =========================================================
   Init backend
   ========================================================= */

bool acia_init_com(struct acia* acia)
{
  acia->done     = com_done;
  acia->stat     = com_stat;
  acia->has_byte = com_has_byte;
  acia->get_byte = com_get_byte;
  acia->put_byte = com_put_byte;
  acia->set_config = com_set_config;
  acia->poll_rx = com_poll_rx;

  if(com_validate_param())
  {
    if(com_open())
      uart_ready = true;  
      return true;
  }

  acia_init_none(acia);
  return false;
}

/* =========================================================
   Validation paramètres
   ========================================================= */

static bool com_validate_param(void)
{
  switch(com_baudrate)
  {
    case 50: case 75: case 110: case 134:
    case 150: case 300: case 600: case 1200:
    case 1800: case 2400: case 4800: case 9600:
    case 19200: case 38400: case 57600: case 115200:
      break;
    default:
      return false;
  }

  switch(com_bits)
  {
    case 5: case 6: case 7: case 8:
      break;
    default:
      return false;
  }

  switch(com_parity)
  {
    case 'N': case 'n':
    case 'O': case 'o':
    case 'E': case 'e':
      break;
    default:
      return false;
  }

  switch(com_stopbits)
  {
    case 1: case 2:
      break;
    default:
      return false;
  }

  com_param = 1;
  return true;
}

/* =========================================================
   Open
   ========================================================= */

static bool com_open(void)
{
  com_fifo = 0;
/*
  printf("[6551] COM backend open\n");
  printf("[6551] baud=%d bits=%d parity=%c stop=%d\n",
         com_baudrate,
         com_bits,
         com_parity,
         com_stopbits);
*/
  
 //=========================================================
 //PICO UART0 

  uart_init(uart0, com_baudrate);

  gpio_set_function(12, GPIO_FUNC_UART); // TX
  gpio_set_function(13, GPIO_FUNC_UART); // RX

  /*
  uart_set_format(uart0,
                  com_bits,
                  com_stopbits,
                  UART_PARITY_NONE);
  */
  com_set_config(
    com_baudrate,
    com_bits,
    com_parity,
    com_stopbits);


 

  return true;
}

/* =========================================================
   Close
   ========================================================= */

static bool com_close(void)
{
  //printf("[6551] COM backend closed\n");

  //com_fd = -1;

  
  uart_deinit(uart0); // PICO
  uart_ready = false;

  return true;
}

static void com_poll_rx(void) {
    if (uart_ready) {
        //gpio_put(25, 1);
        while (uart_is_readable(uart0)) {
            
            uint8_t c = uart_getc(uart0);
            uart_putc(uart0, c);
            if (com_fifo < COM_FIFO_SIZE)
                com_fifo_buf[com_fifo++] = c;
        }
        //gpio_put(25, 0);
    }
}

/* =========================================================
   TX
   ========================================================= */

static bool com_write(uint8_t data)
{
  
   // PICO UART TX

  uart_putc(uart0, data);
  
  return true;
}

//   RX

static bool com_read(uint8_t* data)
{
  if(com_fifo > 0)
  {
    *data = com_fifo_buf[0];

    memmove(com_fifo_buf,
            com_fifo_buf + 1,
            --com_fifo);
    
    return true;
  }

  return false;
}
/* =========================================================
   Peek RX
   ========================================================= */
static bool com_peek(uint8_t* data) {
    if (com_fifo > 0) {
        *data = com_fifo_buf[0];

        return true;
    }

    return false;
}


static bool com_set_config(uint32_t baud,int bits,char parity,int stopbits)
{
    uart_parity_t p = UART_PARITY_NONE;

    switch(parity)
    {
        case 'E':
        case 'e':
            p = UART_PARITY_EVEN;
            break;

        case 'O':
        case 'o':
            p = UART_PARITY_ODD;
            break;

        default:
            p = UART_PARITY_NONE;
            break;
    }

    uart_deinit(uart0);

    uart_init(uart0, baud);

    uart_set_format(
        uart0,
        bits,
        stopbits,
        p
    );

    return true;
}


#endif /* BACKEND_COM */
