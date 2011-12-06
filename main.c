/**
 * Utility to read/write I2C.
 * Jan Viktorin <xvikto03@stud.fit.vutbr.cz>
 */

#include "i2cio.h"
#include "util.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static int verbosity = 0;

//
// Application logic
//

void verbose_print(int level, const char *fmt, ...)
{
	if(level > verbosity)
		return;

	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr, "\n");
	fflush(stderr);
}

int print_error(const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr, "\n");
	fflush(stderr);
	return 1;
}

int process_read(uint8_t base, uint8_t address, struct i2cio_dev *d)
{
	uint8_t data;
	verbose_print(1, "Read from device at address 0x%02X [0x%02X]", base, address);

	if(i2cio_setaddr(d, base) != 0) {
		perror("i2cio_setaddr()");
		return 1;
	}

	if(i2cio_read8(d, address, &data) != 0) {
		perror("i2cio_read8()");
		return 1;
	}

	printf("0x%02X\n", data);
	return 0;
}

int process_write(uint8_t base, uint8_t address, uint8_t data, struct i2cio_dev *d)
{
	verbose_print(1, "Write to device at address 0x%02X [0x%02X] data 0x%02X", base, address, data);

	if(i2cio_setaddr(d, base) != 0) {
		perror("i2cio_setaddr()");
		return 1;
	}

	if(i2cio_write8(d, address, data) != 0) {
		perror("i2cio_write8()");
		return 1;
	}

	return 0;
}

int process_action(const char **options, int remap_bits[8])
{
	if(options[0] == NULL && options[1] == NULL)
		return print_error("No action specified (-r or -w)");	

	if(options[0] != NULL && options[1] != NULL)
		return print_error("Please, specify only one action (-r or -w)");

	if(options[2] == NULL)
		return print_error("No (register) address (-a) was specified");

	if(options[1] != NULL && options[3] == NULL)
		return print_error("No data specified (-d) to write to '%s'", options[1]);

	if(options[4] == NULL || strlen(options[4]) == 0)
		return print_error("No i2c device specifed (-c)");

	struct i2cio_dev dev;
	if(i2cio_init(&dev, options[4]) != 0) {
		perror("i2cio_init()");
		return 1;
	}

	if(i2cio_setbitmap(&dev, remap_bits) != 0)
		fprintf(stderr, "Can not set bit mapping\n");
	
	verbose_print(1, "Using file %s", options[4]);
	verbose_print(2, "Bit mapping: %d, %d, %d, %d, %d, %d, %d, %d", 
	                  remap_bits[0], remap_bits[1], remap_bits[2], remap_bits[3],
			  remap_bits[4], remap_bits[5], remap_bits[6], remap_bits[7]);

	int err = 0;
	if(options[0] != NULL)
		err = process_read(ashex(options[0]), ashex(options[2]), &dev);
	else if(options[1] != NULL)
		err = process_write(ashex(options[1]), ashex(options[2]), ashex(options[3]), &dev);
	else {
		fprintf(stderr, "(%s:%u) This should be never reached\n", __FILE__, __LINE__);
		abort();
	}

	i2cio_fini(&dev);
	return err;
}


//
// Main function
//

int print_help(const char *prog)
{
	fprintf(stderr, "Usage: %s [ -V | -h | -r <base> | -w <base> ] [ -c <devfile> ] [ -a <addr> ] [ -d <data> ] [ -v ] [ -L | -B ]\n", prog);
	fprintf(stderr, "  -r <base>     reads from device at <base> address\n");
	fprintf(stderr, "  -w <base>     writes to device at <base> address\n");
	fprintf(stderr, "  -c <devfile>  uses <devfile> instead of the default /dev/i2c-0\n");
	fprintf(stderr, "  -a <addr>     reads/writes data at address <addr> in the device\n");
	fprintf(stderr, "  -d <data>     writes <data> (use together with -w)\n");
	fprintf(stderr, "  -v            verbose\n");
	fprintf(stderr, "  -S            reverse bit endianness\n");
	fprintf(stderr, "  -V            prints version and exits\n");
	return 0;
}

int print_opterr(char opt)
{
	fprintf(stderr, "Invalid option '-%c' was given\n", opt);
	return 1;
}

int print_version(const char *prog)
{
	printf("%s (%s build on %s at %s)\n", prog, __FILE__, __DATE__, __TIME__);
	return 0;
}

#define I2CIO_OPTS "hVr:w:a:d:c:vS"

int main(int argc, char **argv)
{
	int opt;
	opterr = 0;

	static int identity[8] = {
		0, 1, 2, 3, 4, 5, 6, 7
	};
	static int reverse[8] = {
		7, 6, 5, 4, 3, 2, 1, 0
	};

	const char *read_addr  = NULL;
	const char *write_addr = NULL;
	const char *address = NULL;
	const char *data = NULL;
	const char *ctrldev = "/dev/i2c-0";
	int *bits_remap = identity;

	while((opt = getopt(argc, argv, I2CIO_OPTS)) != -1) {
		switch(opt) {
		case 'h':
			return print_help(argv[0]);

		case 'V':
			return print_version(argv[0]);

		case 'S':
			bits_remap = reverse;
			break;

		case 'r':
			read_addr = optarg;
			break;

		case 'w':
			write_addr = optarg;
			break;

		case 'd':
			data = optarg;
			break;

		case 'a':
			address = optarg;
			break;

		case 'c':
			ctrldev = optarg;
			break;

		case 'v':
			verbosity += 1;
			break;

		case '?':
		default:
			return print_opterr(optopt);
		}
	}

	const char *options[] = {
		read_addr,
		write_addr,
		address,
		data,
		ctrldev
	};
	return process_action(options, bits_remap);
}
