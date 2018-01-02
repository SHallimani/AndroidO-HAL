#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/sterminal.h>

static const hw_module_t *hw_module;
static struct hw_device_t *device;
static sterminal_device_t *st_dev;
static sterminal_str_t *p_data;
static int busy_poll;

void sigint_handler(int sig) {

	printf("SigInt Occured!!!, trying to stop reading data from HAL\n");
	busy_poll = 1;
	sleep(2);
	st_dev->common.close(device);
	free(p_data);
	exit(0);
}

void st_data(sterminal_str_t* c_data)
{
	printf("Data Received from the STerminal!!! %s of %dBytes\n", c_data->data, c_data->len);
}

int main(int argc, char *argv[]) {
	int ret_code;

	// Open the vehicle module and just ask for the list of properties.
	ret_code = hw_get_module(STERMINAL_HARDWARE_MODULE_ID, &hw_module);
	if (ret_code != 0) {
		printf("Cannot open the hw module. Does the STerminal HAL exist? %d\n", ret_code);
		return -1;
	}
	printf("open the hw module. Exists the STerminal HAL exist? %d\n", ret_code);

	ret_code = hw_module->methods->open(hw_module, NULL, &device);
	if (!device) {
		printf("Cannot open the STerminal hw device: %d\n", ret_code);
		return -1;
	}

	st_dev = (sterminal_device_t *) (device);
	printf("STerminal HAL Loaded!\n");

	st_dev->ops->st_init(st_dev, st_data);        /** Returns no.of bytes writeen to the device */
	signal(SIGINT, sigint_handler);

	p_data = (sterminal_str_t *)malloc(sizeof(sterminal_str_t));
	p_data->len = strlen("Hello World");
	strcpy((char *)p_data->data, "Hello World");

	st_dev->ops->st_write(st_dev, p_data); /** returns the no.of bytes read from the device */

	do {
		sleep(1);
	} while(!busy_poll);

	return 0;
}