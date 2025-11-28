#ifndef INFO_H
#define INFO_H

#include <stdint.h>

void info_get_cpu_vendor(char vendor[13]);

uint32_t info_cpu_features(void);
uint32_t info_cpu_features_ext(void);

int info_has_sse(void);
int info_has_sse2(void);
int info_has_avx(void);
int info_has_avx2(void);

void info_cmd_cpuinfo(void);
void info_cmd_meminfo(void);
void info_cmd_uptime(void);

#endif