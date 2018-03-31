#include <system.h>
#include <string.h>
#include <kernel/logging.h>
#include <kernel/page_allocator.h>
#include <jerryscript/jerryscript.h>
#include <jerryscript/jerryscript-ext/handler.h>

void jerryx_port_handler_print_char (char c) {
	terminal__write(&c, 1);
}

int js_engine (void)
{
const jerry_char_t script[] = "var x=5; print('Hello, World from js! x=' + x + ';');";
  size_t script_size = strlen ((const char *) script);

  /* Initialize engine */
  jerry_init (JERRY_INIT_EMPTY);

  /* Register 'print' function from the extensions */
  jerryx_handler_register_global ((const jerry_char_t *) "print",
                                  jerryx_handler_print);

  /* Setup Global scope code */
  jerry_value_t parsed_code = jerry_parse (script, script_size, true);

  if (!jerry_value_has_error_flag (parsed_code))
  {
    /* Execute the parsed source code in the Global scope */
    jerry_value_t ret_value = jerry_run (parsed_code);

    /* Returned value must be freed */
    jerry_release_value (ret_value);
  }

  /* Parsed source code must be freed */
  jerry_release_value (parsed_code);

  /* Cleanup engine */
  jerry_cleanup ();

}

void kernel_main(uint64_t magic, uint64_t *addr)
{

	console__initialize();

	console__clear();

	/* Initialize terminal interface */
	terminal_initialize();

	LOG_INFO("Booting at 0x%x", &kernel_main);

	LOG_INFO("js_engine=%d", js_engine());

	page_allocator_initialize(128*1024*1024);

	mem_alloc_initialize();

	services_initialize();

	// SyscallInfo_t logging;
	// logging.name = "logging";
	// logging.call = (Syscall) log_info;
	//
	// servicehandler_t syscall_logging = services_register(logging);
	// services_call(syscall_logging, "\n**** testing first syscall ****\n");

	if (multiboot_parser(magic, addr) == -1) {
		LOG_ERROR("Try to use a bootloader with multiboot2 support");
		return;
	}

	LOG_DEBUG("linha 1 0x%x", malloc(5));
	LOG_DEBUG("linha 2 0x%x", sbrk(0));
	LOG_DEBUG("Goodbye");
}
