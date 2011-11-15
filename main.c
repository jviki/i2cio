/**
 * Utility to read/write I2C.
 * Jan Viktorin <xvikto03@stud.fit.vutbr.cz>
 */

#include "i2cio.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//
// Application logic
//

uint8_t ashex(const char *s)
{
	if(s[0] == '0' && tolower(s[1]) == 'x')
		s += 2;
	long v = strtol(s, NULL, 16);
	return (uint8_t) v;
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

int process_action(const char **options)
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
	fprintf(stderr, "Usage: %s [ -V | -h | -r <base> | -w <base> ] [ -c <devfile> ] [ -a <addr> ] [ -d <data> ]\n", prog);
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

#define I2CIO_OPTS "hVr:w:a:d:c:"

int main(int argc, char **argv)
{
	int opt;
	opterr = 0;

	const char *read_addr  = NULL;
	const char *write_addr = NULL;
	const char *address = NULL;
	const char *data = NULL;
	const char *ctrldev = "/dev/i2c-0";

	while((opt = getopt(argc, argv, I2CIO_OPTS)) != -1) {
		switch(opt) {
		case 'h':
			return print_help(argv[0]);

		case 'V':
			return print_version(argv[0]);

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
	return process_action(options);
}
