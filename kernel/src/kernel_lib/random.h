#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void random_init(uint64_t seed);
uint64_t random_u64(void);
uint32_t random_u32(void);
uint32_t random_range_u32(uint32_t min, uint32_t max);
uint64_t random_range_u64(uint64_t min, uint64_t max);
void random_bytes(void *buf, size_t len);
bool random_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif