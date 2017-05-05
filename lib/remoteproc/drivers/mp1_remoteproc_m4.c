/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc.
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
 *       zynqmp_remoteproc_r5.c
 *
 * DESCRIPTION
 *
 *       This file is the Implementation of IPC hardware layer interface
 *       for Xilinx Zynq UltraScale+ MPSoC system.
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

/*--------------------------- Globals ---------------------------------- */
struct hil_platform_ops stm32mp1_proc_ops = {
	.enable_interrupt     = _enable_interrupt,
	.notify               = _notify,
	.boot_cpu             = _boot_cpu,
	.shutdown_cpu         = _shutdown_cpu,
	.poll                 = _poll,
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

