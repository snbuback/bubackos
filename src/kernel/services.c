#include <kernel/services.h>

static SyscallInfo_t services_table[ MAX_SERVICES ];
static volatile servicehandler_t last_service_number;

static void no_function(void) {
  kprintf("No function\n");
}

static const SyscallInfo_t no_op = {
  .name = "no_op",
  .call = no_function
};

void services_initialize(void)
{
  for (servicehandler_t i = 0; i < MAX_SERVICES; i++) {
    services_table[i] = no_op;
  }
  last_service_number = 0;
  kprintf("Services initialized\n");
}

servicehandler_t services_register(SyscallInfo_t info)
{
  servicehandler_t service_number = ++last_service_number;
  services_table[service_number] = info;
  return service_number;
}

// TODO this function signature needs change: return and arguments should be more generic
void services_call(servicehandler_t service_handler, char* service_arg1) {
  SyscallInfo_t info = services_table[service_handler];
  kprintf("Calling %s at 0x%x", info.name, info.call);
  info.call(service_arg1);
  // no return so far
  return;
}
