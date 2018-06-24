#ifndef _ELF_H
#define _ELF_H
#include <stdint.h>
#include <stdlib.h>
#include <algorithms/linkedlist.h>

/*
 * Object file type
 */
enum {
        ELF_ET_NONE             = 0,            /* No file type */
        ELF_ET_REL              = 1,            /* Relocatable file */
        ELF_ET_EXEC             = 2,            /* Executable file */
        ELF_ET_DYN              = 3,            /* Shared object file */
        ELF_ET_CORE             = 4,            /* Core file */
        ELF_ET_LOOS             = 0xfe00,       /* Operating system-specific */
        ELF_ET_HIOS             = 0xfeff,       /* Operating system-specific */
        ELF_ET_LOPROC           = 0xff00,       /* Processor-specific */
        ELF_ET_HIPROC           = 0xffff,       /* Processor-specific */
};

/*
 * Architectures
 */
enum {
        ELF_EM_ANY          = 0,    /* No machine */
        ELF_EM_M32          = 1,    /* AT&T WE 32100 */
        ELF_EM_SPARC        = 2,    /* SPARC */
        ELF_EM_386          = 3,    /* Intel 80386 */
        ELF_EM_68K          = 4,    /* Motorola 68000 */
        ELF_EM_88K          = 5,    /* Motorola 88000 */
        ELF_EM_IAMCU        = 6,    /* Intel MCU */
        ELF_EM_860          = 7,    /* Intel 80860 */
        ELF_EM_MIPS         = 8,    /* MIPS I Architecture */
        ELF_EM_S370         = 9,    /* IBM System/370 Processor */
        ELF_EM_MIPS_RS3_LE  = 10,   /* MIPS RS3000 Little-endian */
        ELF_EM_PARISC       = 15,   /* Hewlett-Packard PA-RISC */
        ELF_EM_VPP500       = 17,   /* Fujitsu VPP500 */
        ELF_EM_SPARC32PLUS  = 18,   /* Enhanced instruction set SPARC */
        ELF_EM_960          = 19,   /* Intel 80960 */
        ELF_EM_PPC          = 20,   /* PowerPC */
        ELF_EM_PPC64        = 21,   /* 64-bit PowerPC */
        ELF_EM_S390         = 22,   /* IBM System/390 Processor */
        ELF_EM_SPU          = 23,   /* IBM SPU/SPC */
        ELF_EM_V800         = 36,   /* NEC V800 */
        ELF_EM_FR20         = 37,   /* Fujitsu FR20 */
        ELF_EM_RH32         = 38,   /* TRW RH-32 */
        ELF_EM_RCE          = 39,   /* Motorola RCE */
        ELF_EM_ARM          = 40,   /* ARM 32-bit architecture (AARCH32) */
        ELF_EM_ALPHA        = 41,   /* Digital Alpha */
        ELF_EM_SH           = 42,   /* Hitachi SH */
        ELF_EM_SPARCV9      = 43,   /* SPARC Version 9 */
        ELF_EM_TRICORE      = 44,   /* Siemens TriCore embedded processor */
        ELF_EM_ARC          = 45,   /* Argonaut RISC Core, Argonaut Technologies Inc. */
        ELF_EM_H8_300       = 46,   /* Hitachi H8/300 */
        ELF_EM_H8_300H      = 47,   /* Hitachi H8/300H */
        ELF_EM_H8S          = 48,   /* Hitachi H8S */
        ELF_EM_H8_500       = 49,   /* Hitachi H8/500 */
        ELF_EM_IA_64        = 50,   /* Intel IA-64 processor architecture */
        ELF_EM_MIPS_X       = 51,   /* Stanford MIPS-X */
        ELF_EM_COLDFIRE     = 52,   /* Motorola ColdFire */
        ELF_EM_68HC12       = 53,   /* Motorola M68HC12 */
        ELF_EM_MMA          = 54,   /* Fujitsu MMA Multimedia Accelerator */
        ELF_EM_PCP          = 55,   /* Siemens PCP */
        ELF_EM_NCPU         = 56,   /* Sony nCPU embedded RISC processor */
        ELF_EM_NDR1         = 57,   /* Denso NDR1 microprocessor */
        ELF_EM_STARCORE     = 58,   /* Motorola Star*Core processor */
        ELF_EM_ME16         = 59,   /* Toyota ME16 processor */
        ELF_EM_ST100        = 60,   /* STMicroelectronics ST100 processor */
        ELF_EM_TINYJ        = 61,   /* Advanced Logic Corp. TinyJ embedded processor family */
        ELF_EM_X86_64       = 62,   /* AMD x86-64 architecture */
        ELF_EM_PDSP         = 63,   /* Sony DSP Processor */
        ELF_EM_PDP10        = 64,   /* Digital Equipment Corp. PDP-10 */
        ELF_EM_PDP11        = 65,   /* Digital Equipment Corp. PDP-11 */
        ELF_EM_FX66         = 66,   /* Siemens FX66 microcontroller */
        ELF_EM_ST9PLUS      = 67,   /* STMicroelectronics ST9+ 8/16 bit microcontroller */
        ELF_EM_ST7          = 68,   /* STMicroelectronics ST7 8-bit microcontroller */
        ELF_EM_68HC16       = 69,   /* Motorola MC68HC16 Microcontroller */
        ELF_EM_68HC11       = 70,   /* Motorola MC68HC11 Microcontroller */
        ELF_EM_68HC08       = 71,   /* Motorola MC68HC08 Microcontroller */
        ELF_EM_68HC05       = 72,   /* Motorola MC68HC05 Microcontroller */
        ELF_EM_SVX          = 73,   /* Silicon Graphics SVx */
        ELF_EM_ST19         = 74,   /* STMicroelectronics ST19 8-bit microcontroller */
        ELF_EM_VAX          = 75,   /* Digital VAX */
        ELF_EM_CRIS         = 76,   /* Axis Communications 32-bit embedded processor */
        ELF_EM_JAVELIN      = 77,   /* Infineon Technologies 32-bit embedded processor */
        ELF_EM_FIREPATH     = 78,   /* Element 14 64-bit DSP Processor */
        ELF_EM_ZSP          = 79,   /* LSI Logic 16-bit DSP Processor */
        ELF_EM_MMIX         = 80,   /* Donald Knuth's educational 64-bit processor */
        ELF_EM_HUANY        = 81,   /* Harvard University machine-independent object files */
        ELF_EM_PRISM        = 82,   /* SiTera Prism */
        ELF_EM_AVR          = 83,   /* Atmel AVR 8-bit microcontroller */
        ELF_EM_FR30         = 84,   /* Fujitsu FR30 */
        ELF_EM_D10V         = 85,   /* Mitsubishi D10V */
        ELF_EM_D30V         = 86,   /* Mitsubishi D30V */
        ELF_EM_V850         = 87,   /* NEC v850 */
        ELF_EM_M32R         = 88,   /* Mitsubishi M32R */
        ELF_EM_MN10300      = 89,   /* Matsushita MN10300 */
        ELF_EM_MN10200      = 90,   /* Matsushita MN10200 */
        ELF_EM_PJ           = 91,   /* picoJava */
        ELF_EM_OPENRISC     = 92,   /* OpenRISC 32-bit embedded processor */
        ELF_EM_ARC_COMPACT  = 93,   /* ARC International ARCompact processor (old spelling/synonym: EM_ARC_A5) */
        ELF_EM_XTENSA       = 94,   /* Tensilica Xtensa Architecture */
        ELF_EM_VIDEOCORE    = 95,   /* Alphamosaic VideoCore processor */
        ELF_EM_TMM_GPP      = 96,   /* Thompson Multimedia General Purpose Processor */
        ELF_EM_NS32K        = 97,   /* National Semiconductor 32000 series */
        ELF_EM_TPC          = 98,   /* Tenor Network TPC processor */
        ELF_EM_SNP1K        = 99,   /* Trebia SNP 1000 processor */
        ELF_EM_ST200        = 100,  /* STMicroelectronics (www.st.com) ST200 microcontroller */
        ELF_EM_IP2K         = 101,  /* Ubicom IP2xxx microcontroller family */
        ELF_EM_MAX          = 102,  /* MAX Processor */
        ELF_EM_CR           = 103,  /* National Semiconductor CompactRISC microprocessor */
        ELF_EM_F2MC16       = 104,  /* Fujitsu F2MC16 */
        ELF_EM_MSP430       = 105,  /* Texas Instruments embedded microcontroller msp430 */
        ELF_EM_BLACKFIN     = 106,  /* Analog Devices Blackfin (DSP) processor */
        ELF_EM_SE_C33       = 107,  /* S1C33 Family of Seiko Epson processors */
        ELF_EM_SEP          = 108,  /* Sharp embedded microprocessor */
        ELF_EM_ARCA         = 109,  /* Arca RISC Microprocessor */
        ELF_EM_UNICORE      = 110,  /* Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University*/
        ELF_EM_EXCESS       = 111,  /* eXcess: 16/32/64-bit configurable embedded CPU */
        ELF_EM_DXP          = 112,  /* Icera Semiconductor Inc. Deep Execution Processor */
        ELF_EM_ALTERA_NIOS2 = 113,  /* Altera Nios II soft-core processor */
        ELF_EM_CRX          = 114,  /* National Semiconductor CompactRISC CRX microprocessor */
        ELF_EM_XGATE        = 115,  /* Motorola XGATE embedded processor */
        ELF_EM_C166         = 116,  /* Infineon C16x/XC16x processor */
        ELF_EM_M16C         = 117,  /* Renesas M16C series microprocessors */
        ELF_EM_DSPIC30F     = 118,  /* Microchip Technology dsPIC30F Digital Signal Controller */
        ELF_EM_CE           = 119,  /* Freescale Communication Engine RISC core */
        ELF_EM_M32C         = 120,  /* Renesas M32C series microprocessors */
        ELF_EM_TSK3000      = 131,  /* Altium TSK3000 core */
        ELF_EM_RS08         = 132,  /* Freescale RS08 embedded processor */
        ELF_EM_SHARC        = 133,  /* Analog Devices SHARC family of 32-bit DSP processors */
        ELF_EM_ECOG2        = 134,  /* Cyan Technology eCOG2 microprocessor */
        ELF_EM_SCORE7       = 135,  /* Sunplus S+core7 RISC processor */
        ELF_EM_DSP24        = 136,  /* New Japan Radio (NJR) 24-bit DSP Processor */
        ELF_EM_VIDEOCORE3   = 137,  /* Broadcom VideoCore III processor */
        ELF_EM_LATTICEMICO32        = 138,  /* RISC processor for Lattice FPGA architecture */
        ELF_EM_SE_C17       = 139,  /* Seiko Epson C17 family */
        ELF_EM_TI_C6000     = 140,  /* The Texas Instruments TMS320C6000 DSP family */
        ELF_EM_TI_C2000     = 141,  /* The Texas Instruments TMS320C2000 DSP family */
        ELF_EM_TI_C5500     = 142,  /* The Texas Instruments TMS320C55x DSP family */
        ELF_EM_TI_ARP32     = 143,  /* Texas Instruments Application Specific RISC Processor, 32bit fetch */
        ELF_EM_TI_PRU       = 144,  /* Texas Instruments Programmable Realtime Unit */
        ELF_EM_MMDSP_PLUS   = 160,  /* STMicroelectronics 64bit VLIW Data Signal Processor */
        ELF_EM_CYPRESS_M8C  = 161,  /* Cypress M8C microprocessor */
        ELF_EM_R32C         = 162,  /* Renesas R32C series microprocessors */
        ELF_EM_TRIMEDIA     = 163,  /* NXP Semiconductors TriMedia architecture family */
        ELF_EM_QDSP6        = 164,  /* QUALCOMM DSP6 Processor */
        ELF_EM_8051         = 165,  /* Intel 8051 and variants */
        ELF_EM_STXP7X       = 166,  /* STMicroelectronics STxP7x family of configurable and extensible RISC processors */
        ELF_EM_NDS32        = 167,  /* Andes Technology compact code size embedded RISC processor family */
        ELF_EM_ECOG1X       = 168,  /* Cyan Technology eCOG1X family */
        ELF_EM_MAXQ30       = 169,  /* Dallas Semiconductor MAXQ30 Core Micro-controllers */
        ELF_EM_XIMO16       = 170,  /* New Japan Radio (NJR) 16-bit DSP Processor */
        ELF_EM_MANIK        = 171,  /* M2000 Reconfigurable RISC Microprocessor */
        ELF_EM_CRAYNV2      = 172,  /* Cray Inc. NV2 vector architecture */
        ELF_EM_RX           = 173,  /* Renesas RX family */
        ELF_EM_METAG        = 174,  /* Imagination Technologies META processor architecture */
        ELF_EM_MCST_ELBRUS  = 175,  /* MCST Elbrus general purpose hardware architecture */
        ELF_EM_ECOG16       = 176,  /* Cyan Technology eCOG16 family */
        ELF_EM_CR16         = 177,  /* National Semiconductor CompactRISC CR16 16-bit microprocessor */
        ELF_EM_ETPU         = 178,  /* Freescale Extended Time Processing Unit */
        ELF_EM_SLE9X        = 179,  /* Infineon Technologies SLE9X core */
        ELF_EM_L10M         = 180,  /* Intel L10M */
        ELF_EM_K10M         = 181,  /* Intel K10M */
        ELF_EM_AARCH64      = 183,  /* ARM 64-bit architecture (AARCH64) */
        ELF_EM_AVR32        = 185,  /* Atmel Corporation 32-bit microprocessor family */
        ELF_EM_STM8         = 186,  /* STMicroeletronics STM8 8-bit microcontroller */
        ELF_EM_TILE64       = 187,  /* Tilera TILE64 multicore architecture family */
        ELF_EM_TILEPRO      = 188,  /* Tilera TILEPro multicore architecture family */
        ELF_EM_MICROBLAZE   = 189,  /* Xilinx MicroBlaze 32-bit RISC soft processor core */
        ELF_EM_CUDA         = 190,  /* NVIDIA CUDA architecture */
        ELF_EM_TILEGX       = 191,  /* Tilera TILE-Gx multicore architecture family */
        ELF_EM_CLOUDSHIELD  = 192,  /* CloudShield architecture family */
        ELF_EM_COREA_1ST    = 193,  /* KIPO-KAIST Core-A 1st generation processor family */
        ELF_EM_COREA_2ND    = 194,  /* KIPO-KAIST Core-A 2nd generation processor family */
        ELF_EM_ARC_COMPACT2 = 195,  /* Synopsys ARCompact V2 */
        ELF_EM_OPEN8        = 196,  /* Open8 8-bit RISC soft processor core */
        ELF_EM_RL78         = 197,  /* Renesas RL78 family */
        ELF_EM_VIDEOCORE5   = 198,  /* Broadcom VideoCore V processor */
        ELF_EM_78KOR        = 199,  /* Renesas 78KOR family */
        ELF_EM_56800EX      = 200,  /* Freescale 56800EX Digital Signal Controller (DSC) */
        ELF_EM_BA1          = 201,  /* Beyond BA1 CPU architecture */
        ELF_EM_BA2          = 202,  /* Beyond BA2 CPU architecture */
        ELF_EM_XCORE        = 203,  /* XMOS xCORE processor family */
        ELF_EM_MCHP_PIC     = 204,  /* Microchip 8-bit PIC(r) family */
        ELF_EM_INTEL205     = 205,  /* Reserved by Intel */
        ELF_EM_INTEL206     = 206,  /* Reserved by Intel */
        ELF_EM_INTEL207     = 207,  /* Reserved by Intel */
        ELF_EM_INTEL208     = 208,  /* Reserved by Intel */
        ELF_EM_INTEL209     = 209,  /* Reserved by Intel */
        ELF_EM_KM32         = 210,  /* KM211 KM32 32-bit processor */
        ELF_EM_KMX32        = 211,  /* KM211 KMX32 32-bit processor */
        ELF_EM_KMX16        = 212,  /* KM211 KMX16 16-bit processor */
        ELF_EM_KMX8         = 213,  /* KM211 KMX8 8-bit processor */
        ELF_EM_KVARC        = 214,  /* KM211 KVARC processor */
        ELF_EM_CDP          = 215,  /* Paneve CDP architecture family */
        ELF_EM_COGE         = 216,  /* Cognitive Smart Memory Processor */
        ELF_EM_COOL         = 217,  /* Bluechip Systems CoolEngine */
        ELF_EM_NORC         = 218,  /* Nanoradio Optimized RISC */
        ELF_EM_CSR_KALIMBA  = 219,  /* CSR Kalimba architecture family */
        ELF_EM_Z80          = 220,  /* Zilog Z80 */
        ELF_EM_VISIUM       = 221,  /* Controls and Data Services VISIUMcore processor */
        ELF_EM_FT32         = 222,  /* FTDI Chip FT32 high performance 32-bit RISC architecture */
        ELF_EM_MOXIE        = 223,  /* Moxie processor family */
        ELF_EM_AMDGPU       = 224,  /* AMD GPU architecture */
        ELF_EM_RISCV        = 243,  /* RISC-V */
};

/*
 * Operating system/ABI identification
 */
enum {
        ELF_OSABI_NONE           = 0,    /* No extensions or unspecified */
        ELF_OSABI_HPUX           = 1,    /* Hewlett-Packard HP-UX */
        ELF_OSABI_NETBSD         = 2,    /* NetBSD */
        ELF_OSABI_GNU            = 3,    /* GNU */
        ELF_OSABI_SOLARIS        = 6,    /* Sun Solaris */
        ELF_OSABI_AIX            = 7,    /* AIX */
        ELF_OSABI_IRIX           = 8,    /* IRIX */
        ELF_OSABI_FREEBSD        = 9,    /* FreeBSD */
        ELF_OSABI_TRU64          = 10,   /* Compaq TRU64 UNIX */
        ELF_OSABI_MODESTO        = 11,   /* Novell Modesto */
        ELF_OSABI_OPENBSD        = 12,   /* Open BSD */
        ELF_OSABI_OPENVMS        = 13,   /* Open VMS */
        ELF_OSABI_NSK            = 14,   /* Hewlett-Packard Non-Stop Kernel */
        ELF_OSABI_AROS           = 15,   /* Amiga Research OS */
        ELF_OSABI_FENIXOS        = 16,   /* The FenixOS highly scalable multi-core OS */
        ELF_OSABI_CLOUDABI       = 17,   /* Nuxi CloudABI */
        ELF_OSABI_OPENVOS        = 18,   /* Stratus Technologies OpenVOS */
};

/*
 * Program types
 */
enum {
        ELF_PT_NULL         = 0,
        ELF_PT_LOAD         = 1,
        ELF_PT_DYNAMIC      = 2,
        ELF_PT_INTERP       = 3,
        ELF_PT_NOTE         = 4,
        ELF_PT_SHLIB        = 5,
        ELF_PT_PHDR         = 6,
        ELF_PT_TLS          = 7,
        ELF_PT_NUM          = 8,
        ELF_PT_LOOS         = 0x60000000,
        ELF_PT_GNU_EH_FRAME = 0x6474e550,
        ELF_PT_GNU_STACK    = 0x6474e551,
        ELF_PT_GNU_RELRO    = 0x6474e552,
        ELF_PT_LOSUNW       = 0x6ffffffa,
        ELF_PT_SUNWBSS      = 0x6ffffffa,
        ELF_PT_SUNWSTACK    = 0x6ffffffb,
        ELF_PT_HISUNW       = 0x6fffffff,
        ELF_PT_HIOS         = 0x6fffffff,
        ELF_PT_LOPROC       = 0x70000000,
        ELF_PT_HIPROC       = 0x7fffffff,
};

/**
 * Program header flags
 */
enum {
    ELF_PF_FLAGS_X          = 0x1,
    ELF_PF_FLAGS_W          = 0x2,
    ELF_PF_FLAGS_R          = 0x4,
    ELF_PF_FLAGS_MASKOS     = 0x0ff00000,
    ELF_PF_FLAGS_MASKPROC   = 0xf0000000,
};

/*
 * Section Types
 */
enum {
        ELF_SHT_NULL                = 0,
        ELF_SHT_PROGBITS            = 1,
        ELF_SHT_SYMTAB              = 2,
        ELF_SHT_STRTAB              = 3,
        ELF_SHT_RELA                = 4,
        ELF_SHT_HASH                = 5,
        ELF_SHT_DYNAMIC             = 6,
        ELF_SHT_NOTE                = 7,
        ELF_SHT_NOBITS              = 8,
        ELF_SHT_REL                 = 9,
        ELF_SHT_SHLIB               = 10,
        ELF_SHT_DYNSYM              = 11,
        ELF_SHT_INIT_ARRAY          = 14,
        ELF_SHT_FINI_ARRAY          = 15,
        ELF_SHT_PREINIT_ARRAY       = 16,
        ELF_SHT_GROUP               = 17,
        ELF_SHT_SYMTAB_SHNDX        = 18,
        ELF_SHT_LOOS                = 0x60000000,
        ELF_SHT_HIOS                = 0x6fffffff,
        ELF_SHT_LOPROC              = 0x70000000,
        ELF_SHT_HIPROC              = 0x7fffffff,
        ELF_SHT_LOUSER              = 0x80000000,
        ELF_SHT_HIUSER              = 0xffffffff,
};

/*
 * Section Attribute Flags
 */
enum {
        ELF_SHF_WRITE               = 0x1,
        ELF_SHF_ALLOC               = 0x2,
        ELF_SHF_EXECINSTR           = 0x4,
        ELF_SHF_MERGE               = 0x10,
        ELF_SHF_STRINGS             = 0x20,
        ELF_SHF_INFO_LINK           = 0x40,
        ELF_SHF_LINK_ORDER          = 0x80,
        ELF_SHF_OS_NONCONFORMING    = 0x100,
        ELF_SHF_GROUP               = 0x200,
        ELF_SHF_TLS                 = 0x400,
        ELF_SHF_COMPRESSED          = 0x800,
        ELF_SHF_MASKOS              = 0x0ff00000,
        ELF_SHF_MASKPROC            = 0xf0000000,
};

typedef struct {
    uint8_t magic[4];
    uint8_t elfclass;
    uint8_t endian;
    uint8_t ident_version;
    uint8_t os_abi;
    uint8_t pad[8];
    uint16_t type;
    uint16_t machine;
    uint32_t elf_version;
} elf_ident_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uintptr_t vaddr;
    uintptr_t paddr;
    size_t file_size;
    size_t mem_size;
    uint64_t align;
} elf_program_header_t;

typedef struct {
    elf_ident_t* ident;
    uintptr_t entry_point;
    uintptr_t base;
    uint32_t flags;
    linkedlist_t* program_headers; // list of elf_program_header_t
    
} elf_t;

#define ELF_SUCCESS                         0
#define ELF_ERROR_NO_ELF                    1
#define ELF_ERROR_INVALID_ELF_VERSION       2
#define ELF_ERROR_INCOMPATIBLE              3

#define ELF_VALID_IDENT_VERSION             0x1
#define ELF_VALID_ELF_VERSION               0x1
#define ELF_VALID_CLASS                     0x2 // 64 bits
#define ELF_VALID_ENDIAN                    0x1 // little endian
#define ELF_VALID_OSABI                     ELF_OSABI_NONE
#define ELF_VALID_MACHINE                   ELF_EM_X86_64 // x86 64 bits


int elf_parser(void* raw_elf, size_t size, elf_t* elf);

#endif
