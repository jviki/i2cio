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

static
uint8_t remap_bits(int bitmap[8], uint8_t b)
{
	uint8_t r = 0;

	for(size_t i = 0; i < 8; ++i) {
		uint8_t bit = b & (0x01 << i);

		if(bit && bitmap[i] != -1) {
			r |= (0x01 << bitmap[i]);
		}
	}

	return r;
}

int i2cio_write8(struct i2cio_dev *dev, uint8_t addr, uint8_t b)
{
	assert(dev != NULL);
	assert(dev->fd >= 0);

	uint8_t d[2];
	d[0] = addr;
	d[1] = remap_bits(dev->bitmap, b);

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

	uint8_t result;
	ssize_t rlen = read(dev->fd, &result, 1);
	*b = remap_bits(dev->bitmap, result);

	return rlen != 1; // false is success
}

#ifdef REMAP_BIT_TEST
void remap_bits_test(uint8_t value, int bitmap[8], uint8_t expected)
{
	uint8_t result = remap_bits(bitmap, value);
	char *warn = result == expected? "" : "[!] ";
	printf("%s0x%02X -> 0x%02X (0x%02X)\n", warn, value, result, expected);
}

int main(void)
{
	int reverse[8] = {
		7, 6, 5, 4, 3, 2, 1, 0
	};
	int identity[8] = {
		0, 1, 2, 3, 4, 5, 6, 7
	};
	int zero[8] = {
		-1, -1, -1, -1, -1, -1, -1, -1
	};
	int swap_nibbles[8] = {
		4, 5, 6, 7, 0, 1, 2, 3
	};

	fprintf(stderr, "identity\n");
	remap_bits_test(0x0A, identity, 0x0A);
	remap_bits_test(0x1B, identity, 0x1B);
	remap_bits_test(0x2C, identity, 0x2C);
	remap_bits_test(0x3D, identity, 0x3D);
	remap_bits_test(0x4E, identity, 0x4E);
	remap_bits_test(0x59, identity, 0x59);
	remap_bits_test(0x68, identity, 0x68);
	remap_bits_test(0x77, identity, 0x77);
	remap_bits_test(0x86, identity, 0x86);
	remap_bits_test(0x95, identity, 0x95);
	remap_bits_test(0xFF, identity, 0xFF);

	fprintf(stderr, "reverse\n");
	remap_bits_test(0x0A, reverse, 0x50);
	remap_bits_test(0x1B, reverse, 0xD8);
	remap_bits_test(0x2C, reverse, 0x34);
	remap_bits_test(0x3D, reverse, 0xBC);
	remap_bits_test(0x4E, reverse, 0x72);
	remap_bits_test(0x59, reverse, 0x9A);
	remap_bits_test(0x68, reverse, 0x16);
	remap_bits_test(0x77, reverse, 0xEE);
	remap_bits_test(0x86, reverse, 0x61);
	remap_bits_test(0x95, reverse, 0xA9);
	remap_bits_test(0xFF, reverse, 0xFF);

	fprintf(stderr, "zero\n");
	remap_bits_test(0x0A, zero, 0x00);
	remap_bits_test(0x1B, zero, 0x00);
	remap_bits_test(0x2C, zero, 0x00);
	remap_bits_test(0x3D, zero, 0x00);
	remap_bits_test(0x4E, zero, 0x00);
	remap_bits_test(0x59, zero, 0x00);
	remap_bits_test(0x68, zero, 0x00);
	remap_bits_test(0x77, zero, 0x00);
	remap_bits_test(0x86, zero, 0x00);
	remap_bits_test(0x95, zero, 0x00);
	remap_bits_test(0xFF, zero, 0x00);

	fprintf(stderr, "swap_nibbles\n");
	remap_bits_test(0x0A, swap_nibbles, 0xA0);
	remap_bits_test(0x1B, swap_nibbles, 0xB1);
	remap_bits_test(0x2C, swap_nibbles, 0xC2);
	remap_bits_test(0x3D, swap_nibbles, 0xD3);
	remap_bits_test(0x4E, swap_nibbles, 0xE4);
	remap_bits_test(0x59, swap_nibbles, 0x95);
	remap_bits_test(0x68, swap_nibbles, 0x86);
	remap_bits_test(0x77, swap_nibbles, 0x77);
	remap_bits_test(0x86, swap_nibbles, 0x68);
	remap_bits_test(0x95, swap_nibbles, 0x59);
	remap_bits_test(0xFF, swap_nibbles, 0xFF);
}
#endif

