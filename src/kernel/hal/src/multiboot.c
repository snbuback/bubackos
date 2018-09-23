#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <core/hal/platform.h>
#include <hal/boot.h>
#include <hal/multiboot2_spec.h>
#include <logging.h>
#include <core/memory.h>

/*  Check if MAGIC is valid and print the Multiboot information structure
  pointed by ADDR. */
bool multiboot_parser(uint64_t magic, uintptr_t addr, platform_t *platform)
{
    struct multiboot_tag* tag;

    log_trace("Multiboot arguments: magic=0x%x addr=0x%x", magic, addr);

    /*  Am I booted by a Multiboot-compliant boot loader? */
    if ((uint32_t)magic != (uint64_t)MULTIBOOT2_BOOTLOADER_MAGIC) {
        log_error("Invalid magic number. Expected 0x%x but 0x%x", (unsigned)magic,
            (unsigned)MULTIBOOT2_BOOTLOADER_MAGIC);
        return false;
    }

    if (((unsigned int)addr) & 7) {
        log_error("Unaligned mbi: %p", (void*) addr);
        return false;
    }

    log_trace("Announced mbi size 0x%x", *(unsigned*) addr);
    for (tag = (struct multiboot_tag*)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            log_trace("Command line = %s",
                ((struct multiboot_tag_string*)tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            log_trace("Boot loader name = %s",
                ((struct multiboot_tag_string*)tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_MODULE:
        {
            struct multiboot_tag_module* mb_module = ((struct multiboot_tag_module*)tag);
            log_trace("Module at 0x%x-0x%x. Command line %s",
                mb_module->mod_start,
                mb_module->mod_end,
                mb_module->cmdline);

            // new data structure
            info_module_t* module = NEW(info_module_t);
            size_t param_size = strlen(mb_module->cmdline);
            module->param = (char*) kalloc(param_size + 1);
            // copy data to kernel-data memory region
            strncpy(module->param, mb_module->cmdline, param_size);
            module->region.addr_start = mb_module->mod_start;
            module->region.addr_end = mb_module->mod_end;
            module->region.size = mb_module->mod_end - mb_module->mod_start;
            linkedlist_append(platform->modules, module);
            break;
        }
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
        {
            struct multiboot_tag_basic_meminfo* meminfo = (struct multiboot_tag_basic_meminfo*) tag;
            log_trace("mem_lower = %dKB, mem_upper = %dKB",
                meminfo->mem_lower,
                meminfo->mem_upper);
            platform->memory.total_memory = (meminfo->mem_upper + 1024 /*as total memory lower*/) * 1024;
            break;
        }
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
            log_trace("Boot device 0x%x,%u,%u",
                ((struct multiboot_tag_bootdev*)tag)->biosdev,
                ((struct multiboot_tag_bootdev*)tag)->slice,
                ((struct multiboot_tag_bootdev*)tag)->part);
            break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
            multiboot_memory_map_t* mmap;

            for (mmap = ((struct multiboot_tag_mmap*)tag)->entries;
                 (multiboot_uint8_t*)mmap < (multiboot_uint8_t*)tag + tag->size;
                 mmap = (multiboot_memory_map_t*)((unsigned long)mmap + ((struct multiboot_tag_mmap*)tag)->entry_size)) {
                if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) {
                    region_t* region = NEW(region_t);
                    region->addr_start = mmap->addr;
                    region->size = mmap->len;
                    region->addr_end = region->addr_start + region->size;
                    linkedlist_append(platform->memory.reserved_segments, region);

                    log_trace(
                        " reserved memory 0x%x%x,"
                        " length=0x%x%x (%d KB), 0x%x",
                        (unsigned)(mmap->addr >> 32), (unsigned)(mmap->addr & 0xffffffff),
                        (unsigned)(mmap->len >> 32), (unsigned)(mmap->len & 0xffffffff),
                        (unsigned)(mmap->len / 1024), (unsigned)mmap->type);
                }
            }
        } break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
            multiboot_uint32_t color;
            unsigned i;
            struct multiboot_tag_framebuffer* tagfb = (struct multiboot_tag_framebuffer*)tag;
            void* fb = (void*)(unsigned long)tagfb->common.framebuffer_addr;
            log_trace("framebuffer %d", (unsigned)tagfb->common.framebuffer_type);
            break;

            switch (tagfb->common.framebuffer_type) {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED: {
                unsigned best_distance, distance;
                struct multiboot_color* palette;

                palette = tagfb->framebuffer_palette;

                color = 0;
                best_distance = 4 * 256 * 256;

                for (i = 0; i < tagfb->framebuffer_palette_num_colors; i++) {
                    distance = (0xff - palette[i].blue) * (0xff - palette[i].blue) + palette[i].red * palette[i].red + palette[i].green * palette[i].green;
                    if (distance < best_distance) {
                        color = i;
                        best_distance = distance;
                    }
                }
            } break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                color = ((1 << tagfb->framebuffer_blue_mask_size) - 1)
                    << tagfb->framebuffer_blue_field_position;
                break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                color = '\\' | 0x0100;
                break;

            default:
                color = 0xffffffff;
                break;
            }

            for (i = 0; i < tagfb->common.framebuffer_width && i < tagfb->common.framebuffer_height;
                 i++) {
                switch (tagfb->common.framebuffer_bpp) {
                case 8: {
                    multiboot_uint8_t* pixel = fb + tagfb->common.framebuffer_pitch * i + i;
                    *pixel = color;
                } break;
                case 15:
                case 16: {
                    multiboot_uint16_t* pixel = fb + tagfb->common.framebuffer_pitch * i + 2 * i;
                    *pixel = color;
                } break;
                case 24: {
                    multiboot_uint32_t* pixel = fb + tagfb->common.framebuffer_pitch * i + 3 * i;
                    *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
                } break;

                case 32: {
                    multiboot_uint32_t* pixel = fb + tagfb->common.framebuffer_pitch * i + 4 * i;
                    *pixel = color;
                } break;
                }
            }
            break;
        }

        default:
            log_trace("Tag %d (0x%x), Size 0x%x unrecognized", tag->type, tag->type,
                tag->size);
            break;
        }
    }
    tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7));
    log_trace("Total mbi size 0x%x", (uintptr_t) tag - (uintptr_t) addr);
    return true;
}
