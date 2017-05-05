#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include "openamp/hil.h"

/* Interrupt vectors */
#define IPI_IRQ_VECT_ID              65

#define RPMSG_CHAN_NAME              "rpmsg-openamp-demo-channel"

#define ISR_COUNT  2

struct isr_info {
    int vector;
    int priority;
    int type;
    void *data;
    void (*isr)(int vector, void *data);
};

struct hil_proc *platform_create_proc(int proc_index);
int ipcc_init(void);
#endif /* PLATFORM_INFO_H_ */
