/**
 * I2C access library.
 * Jan Viktorin <xvikto03@stud.fit.vutbr.cz>
 */

#ifndef I2CIO_H
#define I2CIO_H

#include <stdint.h>

struct i2cio_dev {
	int fd;
};

/**
 * Opens the device.
 */
int i2cio_init(struct i2cio_dev *dev, const char *path);

/**
 * Frees all resources held by dev.
 */
void i2cio_fini(struct i2cio_dev *dev);

/**
 * Sets the I2C device address.
 */
int i2cio_setaddr(struct i2cio_dev *dev, uint8_t addr);

/**
 * Writes a byte to the I2C device.
 */
int i2cio_write8(struct i2cio_dev *dev, uint8_t addr, uint8_t b);

/**
 * Reads a byte.
 */
int i2cio_read8(struct i2cio_dev *dev, uint8_t addr, uint8_t *b);

#endif
