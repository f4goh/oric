#include "busi2c.h"


i2c_inst_t *i2c = BUS_I2C;


/*******************************************************************************
 * Initialisation du bus I2C
 ******************************************************************************/
void begin(void)
{
    // Initialisation I2C à 400 kHz
    i2c_init(i2c, 400 * 1000);

    // Configuration des GPIO
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    // Pull-up internes (optionnel mais recommandé)
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

/*******************************************************************************
 * Écriture registre(s)
 ******************************************************************************/
void reg_write(const uint addr,const uint8_t reg,uint8_t *buf,const uint8_t nbytes)
{
    if (nbytes < 1)
    {
        return;
    }

    uint8_t msg[nbytes + 1];

    // Adresse du registre
    msg[0] = reg;

    // Données
    for (uint8_t i = 0; i < nbytes; i++)
    {
        msg[i + 1] = buf[i];
    }

    i2c_write_blocking(i2c, addr, msg, nbytes + 1, false);
}

/*******************************************************************************
 * Lecture registre(s)
 ******************************************************************************/
int reg_read(const uint addr,const uint8_t reg,uint8_t *buf,const uint8_t nbytes)
{
    if (nbytes < 1)
    {
        return 0;
    }

    // Sélection du registre
    i2c_write_blocking(i2c, addr, &reg, 1, true);

    // Lecture des données
    return i2c_read_blocking(i2c, addr, buf, nbytes, false);
}
