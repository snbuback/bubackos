#include <kernel/services.h>
#include <kernel/logging.h>

static SyscallInfo_t services_table[ MAX_SERVICES ];
static volatile servicehandler_t last_service_number;

static void no_function(void) {
  LOG_DEBUG("No function");
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
  LOG_DEBUG("Services initialized");
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
  LOG_DEBUG("Calling %s at 0x%x", info.name, info.call);
  info.call(service_arg1);
  // no return so far
  return;
}
