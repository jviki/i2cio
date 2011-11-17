/**
 * I2C access library.
 * Jan Viktorin <xvikto03@stud.fit.vutbr.cz>
 */

#ifndef I2CIO_H
#define I2CIO_H

#include <stdint.h>

struct i2cio_dev {
	int fd;
	int bitmap[8];
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
 * Sets bit mappings.
 * Remaps bits of data that is being written or read.
 * Each bitmap[i] must be in range -1..7 where -1 means
 * ignore that bit. When a bitmap[i] and bitmap[j] (i != j)
 * remaps the bit to same position the result is undefined.
 */
int i2cio_setbitmap(struct i2cio_dev *dev, int bitmap[8]);

/**
 * Writes a byte to the I2C device.
 */
int i2cio_write8(struct i2cio_dev *dev, uint8_t addr, uint8_t b);

/**
 * Reads a byte.
 */
int i2cio_read8(struct i2cio_dev *dev, uint8_t addr, uint8_t *b);

#endif
