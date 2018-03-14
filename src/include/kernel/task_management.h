#include <stdint.h>
#include <kernel/memory_allocator.h>

typedef uint64_t task_handler_t;

typedef struct {
  uint32_t task_handler;
  memory_handler_t memory_handler;
  bool ready;
  void* next_address;
} task_t;

void task_management_initialize(void);

uint32_t task_create(memory_handler_t memory_handler);

bool task_start(task_handler_t task_handler, void* start_code_address);

void task_switch(task_handler_t task_handler);

void task_destroy(task_handler_t task_handler);

void _platform_task_switch(void* next_address);
