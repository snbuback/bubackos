#include <system.h>
#include <kernel/js_engine.h>
#include <jerryscript/jerryscript.h>
#include <jerryscript/jerryscript-ext/handler.h>

/**
 * Return the platform_t struct from the jsobject or NULL if not found
 */
static platform_t* get_platform_from_jsobject(jerry_value_t platform_js_object) {
  platform_t *platform;
  const jerry_object_native_info_t *type_p;
  if (jerry_get_object_native_pointer (platform_js_object, &platform, &type_p)) {
    // TODO Pending check if is the corrent object as in the documentation of jerry_get_object_native_pointer
    return platform;
  }
  return NULL;
}

static jerry_value_t platform_get_total_memory(const jerry_value_t func_value, /**< function object */
                 const jerry_value_t this_value, /**< this arg */
                 const jerry_value_t *args_p, /**< function arguments */
                 const jerry_length_t args_cnt) /**< number of function arguments */
{
  platform_t *platform = get_platform_from_jsobject(this_value);
  return jerry_create_number(platform->total_memory);
}

static bool js_engine_inject_native_methods(jerry_value_t platform_js_object) {
    static const struct {
        const char *function_name;
        jerry_external_handler_t js_func_handler;
    } common_functions[] = {
        { "getTotalMemory", platform_get_total_memory },
        { NULL, NULL }
    };

    for (int i = 0; common_functions[i].function_name != NULL; i++) {
        jerry_value_t func_obj = jerry_create_external_function (common_functions[i].js_func_handler);
        if (jerry_value_has_error_flag(func_obj)) {
            jerry_release_value (func_obj);
            return false;
        }

        /* Set the native function as a property of the empty JS object */
        jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) common_functions[i].function_name);
        jerry_set_property (platform_js_object, prop_name, func_obj);
        jerry_release_value (prop_name);
        jerry_release_value (func_obj);
    }
    return true;
}

bool js_engine_initialize(platform_t *platform) {
    /* Initialize engine */
    jerry_init (JERRY_INIT_EMPTY);

    // create Platform object
    jerry_value_t platform_js_object = jerry_create_object ();
    if (jerry_value_has_error_flag(platform_js_object)) {
        return false;
    }

    // store platform_t reference in the object
    jerry_set_object_native_pointer (platform_js_object, platform, NULL);

    // inject native methods
    if (!js_engine_inject_native_methods(platform_js_object)) {
        return false;
    }

    /* Wrap the JS object (not empty anymore) into a jerry api value */
    jerry_value_t global_object = jerry_get_global_object ();

    /* Add the JS object to the global context */
    jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "platform");
    jerry_set_property (global_object, prop_name, platform_js_object);
    jerry_release_value (prop_name);
    jerry_release_value (platform_js_object);
    jerry_release_value (global_object);

    /* Register 'print' function from the extensions */
    jerryx_handler_register_global ((const jerry_char_t *) "print",
                                    jerryx_handler_print);

    /* Cleanup engine */
    //jerry_cleanup (); never happens
    return true;
}

bool js_engine_module_load(const char* module_name, const char* source_code, size_t size)
{
    bool loaded = false;
    /* Setup Global scope code */
    jerry_value_t parsed_code = jerry_parse_named_resource(module_name, strlen(module_name), source_code, size, true);

    if (!jerry_value_has_error_flag (parsed_code))
    {
        /* Execute the parsed source code in the Global scope */
        jerry_value_t ret_value = jerry_run (parsed_code);
        if (!jerry_value_has_error_flag (ret_value)) {
            loaded = true;
        }
        /* Returned value must be freed */
        jerry_release_value (ret_value);
    }

    /* Parsed source code must be freed */
    jerry_release_value (parsed_code);
    return loaded;
}

bool js_engine_eval(const char* str_to_eval)
{
    bool success = false;
    jerry_value_t ret_val = jerry_eval (str_to_eval, strlen(str_to_eval), true);
    if (!jerry_value_has_error_flag (ret_val)) {
        success = true;
    }
    jerry_release_value (ret_val);
    return success;
}
