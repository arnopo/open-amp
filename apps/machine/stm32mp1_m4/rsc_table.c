/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#include <stdlib.h>
#include <string.h>
#include "openamp/open_amp.h"
#include "rsc_table.h"

/* Place resource table in special ELF section */
#define __section_t(S)          __attribute__((__section__(#S)))
#define __resource              __section_t(.resource_table)

#define RPMSG_IPU_C0_FEATURES        1

/* VirtIO rpmsg device id */
#define VIRTIO_ID_RPMSG_             7

#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000 // Warning = Linux required 0x1000
#define RING_TX                     0x30040000
#define RING_RX                     0x30042000
#define VRING_SIZE                  8

#define NUM_TABLE_ENTRIES           1

#define RPROC_TRACE_BUF

#ifdef RPROC_TRACE_BUF

#define RPROC_TRACE_BUF_SZ 2048
char rproc_log_buf[RPROC_TRACE_BUF_SZ];

void log_buff(int ch)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
 static int offset = 0;

	if (offset + 1 >= RPROC_TRACE_BUF_SZ)
		offset = 0;

	rproc_log_buf[offset] = ch;
	rproc_log_buf[offset++ + 1] = '\0';
}

/* Private functions ---------------------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small log_info (option LD Linker->Libraries->Small log_info
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __attribute__(( weak )) __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int __attribute__(( weak )) fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#ifndef __TERMINAL_IO__
PUTCHAR_PROTOTYPE
{
	log_buff(ch);
	return ch;
}
#endif

struct remote_resource_table __resource rproc_resource = {
	.version = 1, \
	.num = 2, \
	.reserved = {0, 0}, \
	.offset = { \
		offsetof(struct remote_resource_table, rpmsg_vdev),
		offsetof(struct remote_resource_table, cm_trace), \
	}, \

	/* Virtio device entry */
	.rpmsg_vdev= {
		RSC_VDEV, VIRTIO_ID_RPMSG_, 0, RPMSG_IPU_C0_FEATURES, 0, 0, 0,
		NUM_VRINGS, {0, 0},
	},

	/* Vring rsc entry - part of vdev rsc entry */
	.rpmsg_vring0 = {RING_TX, VRING_ALIGN, VRING_SIZE, 1, 0},
	.rpmsg_vring1 = {RING_RX, VRING_ALIGN, VRING_SIZE, 2, 0},

	.cm_trace = { 
		RSC_TRACE,
		(uint32_t)rproc_log_buf, RPROC_TRACE_BUF_SZ, 0, "cm4_log",
	},
	
};
void log_buff_init(void)
{
	printf("[DBG  ] log buff: addr:%p size:%#x\r\n",
		    rproc_log_buf, RPROC_TRACE_BUF_SZ);
}

#else
struct remote_resource_table __resource rproc_resource = {
	.version = 1,
	.num = 0,
	.reserved = {0, 0},
	.offset = {},
};
void log_buff_init(void) {}
#endif


// void *get_resource_table (int *len)
// {
// 	*len = sizeof(resources);
// 	return &resources;
// }

void *get_resource_table (int *len)
{
	/* Initialize RPROC Traces for debug prints */
	log_buff_init();

	*len = sizeof(rproc_resource);

	return &rproc_resource;
}
