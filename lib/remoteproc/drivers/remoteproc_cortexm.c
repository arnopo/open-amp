/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
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

/**************************************************************************
 * FILE NAME
 *
 *       remoteproc_cortexm.c
 *
 * DESCRIPTION
 *
 *       This file is the Implementation of IPC hardware layer interface
 *       for a generic remote_proc Cortex-M system.
 *
 **************************************************************************/

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "metal/io.h"
#include "metal/device.h"
#include "metal/utilities.h"
#include "metal/atomic.h"
#include "metal/irq.h"
#include "metal/cpu.h"
#include "metal/alloc.h"
#include "openamp/hil.h"
#include "openamp/virtqueue.h"


#define _rproc_wait() metal_cpu_yield()

#define DEBUG 1

/* -- FIX ME: ipi info is to be defined -- */
struct ipi_info {
	const char *name;
	const char *bus_name;
	struct metal_device *dev;
	struct metal_io_region *io;
	metal_phys_addr_t paddr;
	uint32_t ipi_chn_mask;
	int registered;
	atomic_int sync;
};

/*--------------------------- Declare Functions ------------------------ */
static int _enable_interrupt(struct proc_intr *intr);
static int _boot_cpu(struct hil_proc *proc, unsigned int load_addr);
static void _shutdown_cpu(struct hil_proc *proc);
static int _initialize(struct hil_proc *proc);
static void _release(struct hil_proc *proc);
extern void env_register_isr(int vector, void *data,
		      void (*isr) (int vector, void *data));
extern int _poll(struct hil_proc *proc, int nonblock);
extern void _notify(struct hil_proc *proc, struct proc_intr *intr_info);
static struct metal_io_region* _alloc_shm(struct hil_proc *proc,
			metal_phys_addr_t pa,
			size_t size,
			struct metal_device **dev);
static void _release_shm(struct hil_proc *proc,
			struct metal_device *dev,
			struct metal_io_region *io);

/*--------------------------- Globals ---------------------------------- */
struct hil_platform_ops cortexm_proc_ops = {
	.enable_interrupt     = _enable_interrupt,
	.notify               = _notify,
	.boot_cpu             = _boot_cpu,
	.shutdown_cpu         = _shutdown_cpu,
	.poll                 = _poll,
	.alloc_shm 	          = _alloc_shm,
	.release_shm          = _release_shm,
	.initialize    = _initialize,
	.release    = _release,
};


static int _enable_interrupt(struct proc_intr *intr)
{
	printf("Vector Id: %d Registered \n\r", intr->vect_id );
	(void)intr;
	return 0;
}

static int _boot_cpu(struct hil_proc *proc, unsigned int load_addr)
{
	(void)proc;
	(void)load_addr;
	return -1;
}

static void _shutdown_cpu(struct hil_proc *proc)
{
	(void)proc;
	return;
}

static struct metal_io_region* _alloc_shm(struct hil_proc *proc,
			metal_phys_addr_t pa,
			size_t size,
			struct metal_device **dev)
{
	(void)proc;
	(void)pa;
	(void)size;

	*dev = hil_create_generic_mem_dev(pa, size, 0);
	if ((*dev))
			return &((*dev)->regions[0]);
	return NULL;
}

static void _release_shm(struct hil_proc *proc,
			struct metal_device *dev,
			struct metal_io_region *io)
{
	(void)proc;
	(void)io;
	(void)dev;
	hil_close_generic_mem_dev(dev);
}

static int _initialize(struct hil_proc *proc)
{
	int ret;
	struct proc_intr *intr_info;
	struct ipi_info *ipi;

	if (!proc)
		return -1;

	intr_info = &(proc->vdev.intr_info);
	ipi = intr_info->data;

	if (ipi && ipi->name && ipi->bus_name) {
		ret = metal_device_open(ipi->bus_name, ipi->name,
					     &ipi->dev);
		if (ret)
			return -ENODEV;
		ipi->io = metal_device_io_region(ipi->dev, 0);
	} else if (ipi->paddr) {
		ipi->io = metal_allocate_memory(
			sizeof(struct metal_io_region));
		if (!ipi->io)
			goto error;
		metal_io_init(ipi->io, (void *)ipi->paddr,
			&ipi->paddr, 0x1000,
			sizeof(metal_phys_addr_t) << 3,
			0,
			NULL);
	}

	ipi->registered = 0;
	return 0;

error:
	_release(proc);
	return -1;
}

static void _release(struct hil_proc *proc)
{
	struct proc_intr *intr_info;
	struct ipi_info *ipi;

	if (!proc)
		return;
	intr_info = &(proc->vdev.intr_info);
	ipi = (struct ipi_info *)(intr_info->data);
	if (ipi) {
		if (ipi->io) {
			if (ipi->dev) {
				metal_device_close(ipi->dev);
				ipi->dev = NULL;
			} else {
				metal_free_memory(ipi->io);
			}
			ipi->io = NULL;
		}

	}
}

