#include <stdint.h>
#include <core/logging.h>

#define INTERACTIONS 50000000

void user_task1() {
	asm("xchg %bx, %bx");
	log_info("User: Hi chamado no modo usuario");

	unsigned cs, ds, ss;
	asm ("mov %%cs, %0; mov %%ds, %1; mov %%ss, %2"
		: "=r" (cs), "=r" (ds), "=r" (ss)
		);
	log_info("User: Segment registers: cs=0x%x ds=0x%x ss=0x%x", cs, ds, ss);

	for (register uint64_t i=0;; i++) {
		if (i%INTERACTIONS == 0) {
			log_debug("i=%d", i/INTERACTIONS);
		}
        double cos = 2;
        double sin = 3;
        double inp = 3;
        asm volatile ("fsincos" : "=t" (cos), "=u" (sin) : "0" (inp));
	}
}

void user_task2() {
	asm("xchg %bx, %bx");
	log_info("User: Hi chamado no modo usuario second task");

	unsigned cs, ds, ss;
	asm ("mov %%cs, %0; mov %%ds, %1; mov %%ss, %2"
		: "=r" (cs), "=r" (ds), "=r" (ss)
		);
	log_info("User: Segment registers second task: cs=0x%x ds=0x%x ss=0x%x", cs, ds, ss);

	for (register uint64_t i=0;; i++) {
		if (i%INTERACTIONS == 0) {
			log_debug("i second task=%ld", i/INTERACTIONS);
		}
        double cos = 2;
        double sin = 3;
        double inp = 3;
        asm volatile ("fsincos" : "=t" (cos), "=u" (sin) : "0" (inp));
	}
}


