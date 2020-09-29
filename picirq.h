#ifndef XV6_PICIRQ_H_
#define XV6_PICIRQ_H_
#include "x86.h"

typedef void (*irq_handler_t)(struct trapframe *);
extern irq_handler_t irq_handler[];
void reg_irq_handler(int irq_num, irq_handler_t handler);
#define IRQ_MAX         256

#endif // XV6_PICIRQ_H_