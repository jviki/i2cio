#include "i2cio.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <string.h>

#define I2C_SLAVE	0x0703	/* Change slave address			*/

int i2cio_init(struct i2cio_dev *dev, const char *path)
{
	dev->fd = open(path, O_RDWR);
	if(dev->fd == -1)
		return -1;

	for(size_t i = 0; i < 8; ++i)
		dev->bitmap[i] = i;

	return 0;
}

void i2cio_fini(struct i2cio_dev *dev)
{
	assert(dev->fd >= 0);
	close(dev->fd);
}

int i2cio_setaddr(struct i2cio_dev *dev, uint8_t addr)
{
	assert(dev != NULL);
	assert(dev->fd >= 0);

	if(ioctl(dev->fd, I2C_SLAVE, addr & 0x7F) < 0)
		return -1;

	return 0;
}

int i2cio_setbitmap(struct i2cio_dev *dev, int bitmap[8])
{
	for(size_t i = 0; i < 8; ++i) {
		if(bitmap[i] < -1 || bitmap[i] >= 8)
			return 1;
	}

	memcpy(dev->bitmap, bitmap, 8);
	return 0;
}

int i2cio_write8(struct i2cio_dev *dev, uint8_t addr, uint8_t b)
{
	assert(dev != NULL);
	assert(dev->fd >= 0);

	uint8_t d[2];
	d[0] = addr;
	d[1] = b;

	ssize_t wlen = write(dev->fd, d, 2);
	return wlen != 2; // false is success
}

int i2cio_read8(struct i2cio_dev *dev, uint8_t addr, uint8_t *b)
{
	assert(dev != NULL);
	assert(dev->fd >= 0);

	ssize_t wlen = write(dev->fd, &addr, 1);
	if(wlen != 1)
		return -1;

	ssize_t rlen = read(dev->fd, b, 1);
	return rlen != 1; // false is success
}



