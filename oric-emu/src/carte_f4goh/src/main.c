/*
 ~/pico-sdk/tools/pioasm/build/pioasm -o c-sdk vga.pio vga.pio.h
pio project init --ide netbeans --board pico

 */
#include <Arduino.h>
#include "bus.h"
#include "pico/stdlib.h"
#include "sd_reader.h"


// simple delay boot helper
static void boot_blink()
{
    pinMode(25, OUTPUT);

    for (int i = 0; i < 3; i++) {
        digitalWrite(25, HIGH);
        delay(100);
        digitalWrite(25, LOW);
        delay(100);
    }
}

//uint8_t buffer[48*1024];

int main() {
    
    initVariant();
    //delay(10);

    // USB serial
    //Serial.begin(115200);
    
    // initialize stdio over usb serial
    //stdio_init_all();
 // boot indication
   

   // delay(50); // stabilisation bus SPI
    
    boot_blink();

    // init SD (LED controlled inside C++ module)
    /*
    if (sd_init() != 0) {
        while (1) {
            // fast blink handled in sd_init()
            delay(1000);
        }
    }
    */
     
    
    
    // read binary file (LED progress only)
    //sd_read_binary("test.bin");
    //sd_write_binary("totoi.txt");
    //sd_read_binary("totoi.txt");
    delay(50);
    // initialize the bus (all devices)
    busInit();

    // run  the PICO-56 boot menu
    //runBootMenu();

    // it's go time!
    busMainLoop();


    //while (1) { }

    return 0;
}

/*
#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/uart.h"
#include "hardware/gpio.h"

int _write(int fd, const char *buf, int len)
{
    uart_write_blocking(
        uart0,
        (const uint8_t *)buf,
        len
    );

    return len;
}


int main()
{
    uart_init(uart0, 115200);
    gpio_set_function(16, GPIO_FUNC_UART);

    while (1)
    {
        printf("%02x\n",128);
        //uart_puts(uart0, "HELLO\n");
        sleep_ms(500);
    }
}
 * */