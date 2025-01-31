#include "timer.h"
#include "alloc.h"
#include "uart.h"
#include "stdint.h"

void timeout_event_init() {
  timeout_queue_head = 0;
  timeout_queue_tail = 0;
}

void core_timer_enable() {
  asm volatile("mov x0, 1");
  asm volatile("msr cntp_ctl_el0, x0"); // enable
  asm volatile("mrs x0, cntfrq_el0");
  asm volatile("msr cntp_tval_el0, x0"); // set expired time
  asm volatile("mov x0, 2");
  asm volatile("ldr x1, =0x40000040");
  asm volatile("str w0, [x1]"); // unmask timer interrupt
}

// void core_timer_handler() {
//     uart_puts("===== timer handler =====\n");
//     uint64_t cntpct_el0, cntfrq_el0;
//     asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct_el0));
//     asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
//     asm volatile("mrs x0, cntfrq_el0");
//     asm volatile("mov x1, 2");
//     asm volatile("mul x0, x0, x1");
//     asm volatile("msr cntp_tval_el0, x0");
//     uart_puts("Time elapsed after booting: ");
//     // char c = cntpct_el0 / cntfrq_el0;
//     uart_int(cntpct_el0 / cntfrq_el0);
//     uart_puts("s\n");
// }

void core_timer_disable() {
  asm volatile("mov x0, 0");
  asm volatile("msr cntp_ctl_el0, x0");  // disable
  asm volatile("mov x0, 0");
  asm volatile("ldr x1, =0x40000040");
  asm volatile("str w0, [x1]");  // unmask timer interrupt
}

void core_timer_handler_lowerEL_64() {  // required 2
  set_expired_time(2);
  uart_puts("Time elapsed after booting: ");
  uart_int(get_current_time());
  uart_puts("s\n");
}

void core_timer_handler_currentEL_ELx() {  // elective 2
  uart_puts("Current time: ");
  uart_int(get_current_time());
  uart_puts("s, ");
  uart_puts("Command executed time: ");
  uart_int(timeout_queue_head->register_time);
  uart_puts("s, ");
  uart_puts("Duration: ");
  uart_int(timeout_queue_head->duration);
  uart_puts("s\n");
  timeout_queue_head->callback(timeout_queue_head->args);
  timeout_event *next = timeout_queue_head->next;
  // free(timeout_queue_head);
  if (next) {
    next->prev = 0;
    timeout_queue_head = next;
    uint64_t next_duration =
        next->register_time + next->duration - get_current_time();
    set_expired_time(next_duration);
  } else {
    timeout_queue_head = 0;
    timeout_queue_tail = 0;
    core_timer_disable();
  }
}

void add_timer(void (*callback)(char *), char *args, uint32_t duration) {
  timeout_event *new_timeout_event =
      (timeout_event *)malloc(sizeof(timeout_event));
  new_timeout_event->register_time = get_current_time();
  new_timeout_event->duration = duration;
  new_timeout_event->callback = callback;
  for (int i = 0; i < 20; i++) {
    new_timeout_event->args[i] = args[i];
    if (args[i] == '\0') break;
  }
  new_timeout_event->prev = 0;
  new_timeout_event->next = 0;

  if (timeout_queue_head == 0) {
    timeout_queue_head = new_timeout_event;
    timeout_queue_tail = new_timeout_event;
    core_timer_enable();
    set_expired_time(duration);
  } else {
    timeout_event *cur;
    uint64_t timeout =
        new_timeout_event->register_time + new_timeout_event->duration;
    for (cur = timeout_queue_head; cur; cur = cur->next) {
      if (cur->register_time + cur->duration > timeout) break;
    }

    if (cur == 0) {
      new_timeout_event->prev = timeout_queue_tail;
      timeout_queue_tail->next = new_timeout_event;
      timeout_queue_tail = new_timeout_event;
    } else if (cur->prev == 0) {
      new_timeout_event->next = cur;
      timeout_queue_head->prev = new_timeout_event;
      timeout_queue_head = new_timeout_event;
      set_expired_time(duration);
    } else {
      new_timeout_event->prev = cur->prev;
      new_timeout_event->next = cur;
      cur->prev->next = new_timeout_event;
      cur->prev = new_timeout_event;
    }
  }
}

uint64_t get_current_time() {
  uint64_t cntpct_el0, cntfrq_el0;
  asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct_el0));
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
  return cntpct_el0 / cntfrq_el0;
}


void set_expired_time(uint32_t duration) {
  uint64_t cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
  asm volatile("msr cntp_tval_el0, %0" : : "r"(cntfrq_el0 * duration));
}

void timer_callback(char *msg) {
  uart_puts("Message: ");
  uart_puts(msg);
  uart_puts("\n");
}