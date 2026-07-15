#ifndef BUSI2C_H
#define BUSI2C_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Configuration I2C
#define SDA_PIN 20
#define SCL_PIN 21

// Bus utilisé
#define BUS_I2C i2c0



/*******************************************************************************
 * Initialisation
 ******************************************************************************/
void begin(void);

/*******************************************************************************
 * Accès registres I2C
 ******************************************************************************/
void reg_write(const uint addr,const uint8_t reg,uint8_t *buf,const uint8_t nbytes);

int reg_read(i2c_inst_t *i2c,const uint addr,const uint8_t reg,uint8_t *buf,const uint8_t nbytes);

#endif

/*
 bus i2c avec peek et poke
 
4 registres
addr0  READ   WRITE  BUFFER en auto incrémentation
addr1  démarrage du bus en passant le mode lecture ou écriture
addr2  SET ADDR
addr3  SET REG
 
 
 
 
 */