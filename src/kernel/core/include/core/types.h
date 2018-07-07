#ifndef _CORE_TYPES_H
#define _CORE_TYPES_H
#include <stdlib.h>
#include <stdint.h>

// Set bit y (0-indexed) of x to '1' by generating a a mask with a '1' in the proper bit location and ORing x with the mask.
#define SET_BIT(x,y)        x |= (1 << y)

// Set bit y (0-indexed) of x to '0' by generating a mask with a '0' in the y position and 1's 
// elsewhere then ANDing the mask with x.
#define CLEAR_BIT(x,y)      x &= ~(1<< y)

// Return '1' if the bit value at position y within x is '1' and '0' if it's 0 by ANDing x 
// with a bit mask where the bit in y's position is '1' and '0' elsewhere and comparing it to all 0's.  Returns '1' in least significant bit position if the value of the bit is '1', '0' if it was '0'.
#define READ_BIT(x,y)       ((0u == (x & (1<<y)))?0u:1u)

// Toggle bit y (0-index) of x to the inverse: '0' becomes '1', '1' becomes '0' by XORing x 
// with a bitmask where the bit in position y is '1' and all others are '0'.
#define TOGGLE_BIT(x,y)     (x ^= (1<<y))

// Provides generic types

/**
 * Generic address structure
 */
typedef struct {
    uintptr_t addr_start;
    uintptr_t addr_end;
    size_t size;
} region_t;

// permission type
// map of bits
// 0 - read permission
// 1 - write permission
// 2 - execution permission
// 3 - user/kernel mode
typedef uint8_t permission_t;

#define PERM_IS_READ(p)					READ_BIT(p, 0)
#define PERM_SET_READ(p, v)				if(v) SET_BIT(p, 0); else CLEAR_BIT(p, 0);

#define PERM_IS_WRITE(p)				READ_BIT(p, 1)
#define PERM_SET_WRITE(p, v)			if(v) SET_BIT(p, 1); else CLEAR_BIT(p, 1);

#define PERM_IS_EXEC(p)					READ_BIT(p, 2)
#define PERM_SET_EXEC(p, v)				if(v) SET_BIT(p, 2); else CLEAR_BIT(p, 2);

#define PERM_IS_KERNEL_MODE(p)			READ_BIT(p, 3)
#define PERM_SET_KERNEL_MODE(p, v)		if(v) SET_BIT(p, 3); else CLEAR_BIT(p, 3);

#endif