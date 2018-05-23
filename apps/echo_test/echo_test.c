/* This is a sample demonstration application that showcases usage of rpmsg 
This application is meant to run on the remote CPU running baremetal code. 
This application echoes back data that was sent to it by the master core. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "rsc_table.h"
#include "platform_info.h"

#define SHUTDOWN_MSG	0xEF56A55A
#define APP_EPT_ADDR    0
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

struct _payload {
	unsigned long num;
	unsigned long size;
	unsigned char data[];
};

static int err_cnt;

#define PAYLOAD_MIN_SIZE	1

/* Globals */
static struct rpmsg_endpoint lept;
static struct _payload *i_payload;
static int rnum = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/* External functions */
extern int init_system();
extern void cleanup_system();

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static void rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			      uint32_t src, void *priv)
{
	int i;
	struct _payload *r_payload = (struct _payload *)data;

	(void)ept;
	(void)src;
	(void)priv;
	LPRINTF(" received payload number %lu of size %d \r\n",
		r_payload->num, len);

	if (r_payload->size == 0) {
		LPERROR(" Invalid size of package is received.\n");
		err_cnt++;
		return;
	}
	/* Validate data buffer integrity. */
	for (i = 0; i < (int)r_payload->size; i++) {
		if (r_payload->data[i] != 0xA5) {
			LPRINTF("Data corruption at index %d\n", i);
			err_cnt++;
			break;
		}
	}
	rnum = r_payload->num + 1;
}

static void rpmsg_endpoint_destroy(struct rpmsg_endpoint *ept)
{
	(void)ept;
	LPERROR("Endpoint is destroyed\n");
	ept_deleted = 1;
}

static void rpmsg_new_endpoint_cb(struct rpmsg_device *rdev, const char *name,
				  uint32_t src)
{
	if (strcmp(name, RPMSG_CHAN_NAME))
		LPERROR("Unexpected name service %s.\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_CHAN_NAME,
				       APP_EPT_ADDR, src,
				       rpmsg_endpoint_cb,
				       rpmsg_endpoint_destroy);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app (struct rpmsg_device *rdev, void *priv)
{
	int ret;
	int shutdown_msg = SHUTDOWN_MSG;
	int i;
	int size, max_size, num_payloads;
	int expect_rnum = 0;

	LPRINTF(" 1 - Send data to remote core, retrieve the echo");
	LPRINTF(" and validate its integrity ..\n");

	max_size = rpmsg_virtio_get_buffer_size(rdev);
	if (max_size < 0) {
		LPERROR("No avaiable buffer size.\n");
		return -1;
	}
	num_payloads = max_size - PAYLOAD_MIN_SIZE + 1;
	i_payload =
	    (struct _payload *)metal_allocate_memory(2 * sizeof(unsigned long) +
				      max_size);

	if (!i_payload) {
		LPERROR("memory allocation failed.\n");
		return -1;
	}

	/* Initialize RPMSG framework */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_CHAN_NAME, APP_EPT_ADDR,
			       RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_endpoint_destroy);

	if (ret) {
		LPERROR("Failed to initialize remoteproc resource.\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);

	LPRINTF("RPMSG endpoint is binded with remote.\n");
	for (i = 0, size = PAYLOAD_MIN_SIZE; i < num_payloads; i++, size++) {
		i_payload->num = i;
		i_payload->size = size;

		/* Mark the data buffer. */
		memset(&(i_payload->data[0]), 0xA5, size);

		LPRINTF("sending payload number %lu of size %d\n",
			i_payload->num, (2 * sizeof(unsigned long)) + size);

		ret = rpmsg_send(&lept, i_payload,
				 (2 * sizeof(unsigned long)) + size);

		if (ret < 0) {
			LPRINTF("Error sending data...\n");
			break;
		}
		LPRINTF("echo test: sent : %d\n",
		(2 * sizeof(unsigned long)) + size);

		expect_rnum++;
		do {
			platform_poll(priv);
		} while ((rnum < expect_rnum) && !err_cnt);

	}

	LPRINTF("**********************************\n");
	LPRINTF(" Test Results: Error count = %d \n", err_cnt);
	LPRINTF("**********************************\n");
	/* Send shutdown message to remote */
	rpmsg_send(&lept, &shutdown_msg, sizeof(int));
	while(!ept_deleted)
		platform_poll(priv);
	LPRINTF("Quitting application .. Echo test end\n");

	metal_free_memory(i_payload);
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned long proc_id = 0;
	unsigned long rsc_id = 0;
	struct remoteproc *rproc;
	struct rpmsg_device *rdev;
	int ret;

	/* Initialize HW system components */
	init_system();

	if (argc >= 2) {
		proc_id = strtoul(argv[1], NULL, 0);
	}

	if (argc >= 3) {
		rsc_id = strtoul(argv[2], NULL, 0);
	}

	rproc = platform_create_proc(proc_id, rsc_id);
	if (!rproc) {
		LPERROR("Failed to create remoteproc device.\n");
		ret = -1;
	} else {
		rdev = platform_create_rpmsg_vdev(rproc, 0,
						  VIRTIO_DEV_MASTER,
						  NULL,
						  rpmsg_new_endpoint_cb);
		if (!rdev) {
			LPERROR("Failed to create rpmsg virtio device.\n");
			ret = -1;
		} else {
			app(rdev, (void *)rproc);
			ret = 0;
		}
	}

	LPRINTF("Stopping application...\n");
	remoteproc_remove(rproc);
	cleanup_system();

	return ret;
}

