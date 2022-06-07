/*
 * Copyright (c) 2023 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VIRTIO_DRV_H_
#define _VIRTIO_DRV_H_

#if defined(WITH_VIRTIO_MMIO_DRV)
#include <openamp/virtio_mmio.h>

#define virtio_drv_setup_virtqueue virtio_mmio_setup_virtqueue
#else
#error Only VIRTIO-MMIO transport layer supported
#endif /* defined(WITH_VIRTIO_MMIO_DRV) */

#endif /* _VIRTIO_DRV_H_ */
