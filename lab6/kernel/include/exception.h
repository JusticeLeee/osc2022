#pragma once
#include <stdint.h>
#include "gpio.h"
#include "utils.h"
#include "mmu.h"

#define CORE0_IRQ_SOURCE ((volatile unsigned int *)(KVA +0x40000060))

#define GPU_IRQ (1 << 8)
#define CNTPNS_IRQ (1 << 1)
void sync_handler_currentEL_ELx();
void sync_handler_lowerEL_64(uint64_t sp_addr);
void irq_handler_currentEL_ELx();
void irq_handler_lowerEL_64();
void default_handler();
void enable_interrupt();
void disable_interrupt();