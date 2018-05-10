#include <stdio.h>
#include <stdint.h>
#include <core/logging.h>
#include <hal/console.h>
#include <core/task_management.h>

#define INTERACTIONS 	3000000
#define BUFFER_SIZE		100

void syscall()
{
	asm volatile("xchg %bx, %bx; mov $1, %rdi; int $50");
}

void user_task1() {
	char buffer[BUFFER_SIZE];
	for (register int i=10*INTERACTIONS;; i++) {
		asm volatile ("movq $0x1111, %r11; movq $0x1115, %r15");
		if (i%INTERACTIONS == 0) {
			size_t sz = snprintf(buffer, BUFFER_SIZE, "task 1=%d", i/INTERACTIONS);
			console_raw_write(buffer, sz, 10, 24, 50);
		}
        double cos = 2;
        double sin = 3;
        double inp = 3;
        asm volatile ("fsincos" : "=t" (cos), "=u" (sin) : "0" (inp));
	}
}

void user_task2() {
	char buffer[BUFFER_SIZE];
	for (register int i=0;; i++) {
		asm volatile ("movq $0x2222, %r11; movq $0x2226, %r15");
		if (i%INTERACTIONS == 0) {
			size_t sz = snprintf(buffer, BUFFER_SIZE, "task 2=%d", i/INTERACTIONS);
			console_raw_write(buffer, sz, 11, 24, 65);
		}
        double cos = 2;
        double sin = 3;
        double inp = 3;
        asm volatile ("fsincos" : "=t" (cos), "=u" (sin) : "0" (inp));
	}
}

void user_task3() {
	bool destroyed = false;
	char buffer[BUFFER_SIZE];
	for (register int i=0;; i++) {
		asm volatile ("movq $0x3333, %r11; movq $0x3337, %r15");
		if (i%INTERACTIONS == 0) {
			size_t sz = snprintf(buffer, BUFFER_SIZE, "task 3=%d", i/INTERACTIONS);
			console_raw_write(buffer, sz, 11, 24, 35);
		}
		if (i/INTERACTIONS == 1 && !destroyed) {
			log_info("syscall");
			syscall();
			log_info("Return from syscall");
		}
        double cos = 2;
        double sin = 3;
        double inp = 3;
        asm volatile ("fsincos" : "=t" (cos), "=u" (sin) : "0" (inp));
	}
}
