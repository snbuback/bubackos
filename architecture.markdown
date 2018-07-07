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
// boot module
void _boot(); // entry point
void native_boot(); // hardware startup. Call kernel_init(platform_t)
void logging_write(); // provides logging in the boot process

// pagging
native_page_table_t* native_pagetable_create()
void native_pagetable_set(native_page_table_t* hal_mmap, address_mapping_t)
void native_pagetable_switch(native_page_table_t* hal_mmap)

// task
native_task_t* hal_create_native_task()
void native_switch_task(native_task_t *task);
void native_sleep();

// timer
time_t get_current_time();  // time contains the number of tickets since the kernel starts

// locking
bool lock_init(lock_t* lock);
bool lock(lock_t* lock);
void unlock(lock_t* lock);
```

These functions are #define to the real function name, to make easy write kernel tests and mock them.

### Common events
* Missing basic driver functionalities like console, keyboard, block disk etc...

### Minimium hardware functionality
* Task switch
* Timer
* Memory Paging/Memory isolation
* locking

## Core

```c
void kernel_init(platform_t platform);

// 
task_t* get_next_task(): list of Task
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