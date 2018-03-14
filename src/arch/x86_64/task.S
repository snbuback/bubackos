
  .globl _platform_task_switch

  .section .text
_platform_task_switch:
  mov $0xbababa, %rdi
  hlt
