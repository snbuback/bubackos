# Architecture

The kernel consist of three layers:
1. HAL (there is no relationship with Linux HAL): responsible to handle basic hardware functionality to the kernel,
like timers, context switch, hardware events, locks
2. Core: provides the basic kernel functionality: creation of tasks, handle the tasks queue/priority, memory management and IPC
3. Platform loaders (or just Loaders): provides platform specific funcionality (still pending of definition)

Core and HAL comunicates using regular function calls.
The communication between Loader and the Core (there is no direct access to HAL) is using the IPC.


## HAL

### Shared functions
```c
void boot_hal(); // hardware startup. Call bubackos_init()
time_t hal_current_time();
bool hal_lock(lock_t lock);
void hal_unlock(lock_t lock);
void hal_platform_events();
void hal_switch_task(native_task_t *task);
```

These functions are #define to the real function name, to make easy write kernel tests and mock them.

### Common events
* Missing basic driver functionalities like console, keyboard, block disk etc...

### Minimium hardware functionality
* Task switch
* Timer
* Memory Paging/Memory isolation

## Core

```c
void bubackos_init(platform_t platform);
task_t* get_next_task(): list of Task (first version always returning 1 task)
publish_hardware_event(...???)
```

# Project structure
```
├── bootloader (+builded kernel to make the iso image)
│   └── boot
│       └── grub
│           └── grub.cfg
├── kernel
│   └── src
│       └── kernel
│           ├── c
│           │   ├── algorithms
│           │   ├── core
│           │   └── hal
│           │       ├── (generic hal files)
│           │       └── x86_64
│           │           ├── (platform specific files)
│           └── headers
│               ├── algorithms
│               ├── core
│               └── hal
├── loaders
│   └── javascript (still pending code organisation)
└── tools
    ├── (build tools)
```