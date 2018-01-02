/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE
#include <hardware/sterminal.h>
#include <hardware/hardware.h>

#include <cutils/log.h>

#include <malloc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include <poll.h>

//serial device terminal mapped to ttyS2 device port
#define STERMINAL_DEVICE	"/dev/ttyS2"
#define STERMINAL_DEVICES	1
#define SECTOMILLISEC(x)	(x * 1000)

typedef struct sterminal_notifier {
	int			revent_sts;
	st_data_callback_t	revent_hndlr;//upper layer data event notify handler
	pthread_t		rthread;     //pthread handler
	uint64_t		timeout_ts;  //Poll timeout in seconds
	struct pollfd		st_pollfd[STERMINAL_DEVICES];
} sterminal_notifier_t;

sterminal_notifier_t *poll_notifier;
sterminal_str_t     *st_io;

void *read_event_handler(void *arg)
{
	int rt;
	sterminal_device_t *hdev = (sterminal_device_t *)arg;

	//allocate memory for object
	st_io = calloc(1, sizeof(sterminal_str_t));

	//make an entry that we are in polling state
	poll_notifier->revent_sts = 1;

	//Add the file-descriptor to the polling list
	poll_notifier->st_pollfd[0].fd = hdev->st_fd;
	poll_notifier->st_pollfd[0].events = POLLIN;

	while(poll_notifier->revent_sts) {
		//polling on the device
		rt = poll(poll_notifier->st_pollfd, 1, poll_notifier->timeout_ts);
		if(rt < 0) {
			perror("poll failed!!!: ");
			break;
		}
		else if(rt == 0) {
			printf("poll timeout!!!\n");
			continue;
		}
		else if(rt > 0) {
			//An event occured on the file descriptor
			if(poll_notifier->st_pollfd[0].revents & POLLIN) {
				//data available to read from the device
				st_io->len = read(poll_notifier->st_pollfd[0].fd, st_io->data, ST_PAGE_SIZE);
				//share the received data to upper layer
				poll_notifier->revent_hndlr(st_io);
			}
		}
	}
	return NULL;
}

int st_init(sterminal_device_t *hdev, st_data_callback_t data_cback) {
	int rt;

	poll_notifier = calloc(1, sizeof(sterminal_notifier_t));
	if (poll_notifier == NULL)
		return -ENOMEM;

	if(data_cback != NULL)
	{
		poll_notifier->revent_hndlr     = data_cback;
		//create thread to read data from sterminal
		rt = pthread_create(&poll_notifier->rthread, NULL, read_event_handler, hdev);
	}
	else
		poll_notifier->revent_hndlr     = NULL;

	poll_notifier->timeout_ts	= SECTOMILLISEC(5); //timeout in seconds

	return 0;
}

int st_write(sterminal_device_t *hdev, sterminal_str_t *p_data) {
	int ret = 0;
	if(p_data != NULL)
	{
		ret = write(hdev->st_fd, (const void *)p_data->data, p_data->len);
		if(ret != p_data->len)
			printf("%s device write failed\n", STERMINAL_DEVICE);
		else
			printf("writeen %d Bytes on %s device\n", ret, STERMINAL_DEVICE);
	}
	else
	{
		printf("%s device write failed - Invalid Buffer Pointer\n", STERMINAL_DEVICE);
		return -ENOMEM;
	}
	return ret;
}

static int sterminal_close(hw_device_t *device) {
	sterminal_device_t *hdev = (sterminal_device_t *)device;

	//destroy the read thread
	poll_notifier->revent_sts = 0;

	//make sure the thread destroyed, the max time 5seconds
	//this can be achieved with pathread_conditional/pthread_mutex/semaphore
	sleep(poll_notifier->timeout_ts);

	close(hdev->st_fd);
	free(hdev->ops);
	free(hdev);
	free(st_io);

	return 0;
}

int sterminal_open(const struct hw_module_t* module, const char* id, struct hw_device_t** device) {

	printf("Device %s Open failed\n", STERMINAL_DEVICE);

	sterminal_device_t *dev = calloc(1, sizeof(sterminal_device_t));
	if (dev == NULL)
		return -ENOMEM;
	printf("Device %s Open failed\n", STERMINAL_DEVICE);

	sterminal_dev_ops *ops_dev = calloc(1, sizeof(sterminal_dev_ops));
	if (dev == NULL) {
		free(dev);
		return -ENOMEM;
	}
	printf("Device %s Open failed\n", STERMINAL_DEVICE);

	dev->common.tag		= HARDWARE_DEVICE_TAG;
	dev->common.version	= HARDWARE_DEVICE_API_VERSION(1,0);
	dev->common.module	= (hw_module_t*)module;
	dev->common.close	= sterminal_close;

	//attach the device read/write call-back handlers
	ops_dev->st_init		= st_init;
	ops_dev->st_write		= st_write;

	dev->ops = ops_dev;
	//open the device to perform read/write operations
	dev->st_fd = open(STERMINAL_DEVICE, O_RDWR);
	if(dev->st_fd < 0) {
		printf("Device %s Open failed\n", STERMINAL_DEVICE);
		perror("device open failed: ");
		free(ops_dev);
		free(dev);
		return -ENODEV;
	}
	printf("Device %s Open failed\n", STERMINAL_DEVICE);
	*device = (hw_device_t *) dev;

	return 0;
}

static struct hw_module_methods_t sterminal_module_methods = {
    .open = sterminal_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = STERMINAL_MODULE_API_VERSION_1_0,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = STERMINAL_HARDWARE_MODULE_ID,
    .name = "STerminal HAL",
    .author = "The Android Open Source Project",
    .methods = &sterminal_module_methods,
};

