#include <kernel_lib/info.h>
#include <interrupts/timer.h>
#include <drivers/framebuffer/kprint.h>
#include <mem/pmm.h>

// ---------------------------
// NTux-OS: Get CPU vendor string
// ---------------------------
void info_get_cpu_vendor(char vendor[13])
{
    uint32_t ebx, ecx, edx;
    asm volatile("cpuid"
                 : "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(0));
    ((uint32_t*)vendor)[0] = ebx;
    ((uint32_t*)vendor)[1] = edx;
    ((uint32_t*)vendor)[2] = ecx;
    vendor[12] = '\0';
}

// ---------------------------
// NTux-OS: Get CPU feature flags
// ---------------------------
uint32_t info_cpu_features(void)
{
    uint32_t edx;
    asm volatile("cpuid" : "=d"(edx) : "a"(1));
    return edx;
}

// ---------------------------
// NTux-OS: Get extended CPU feature flags
// ---------------------------
uint32_t info_cpu_features_ext(void)
{
    uint32_t ebx;
    asm volatile("cpuid" : "=b"(ebx) : "a"(7), "c"(0));
    return ebx;
}

// ---------------------------
// NTux-OS: Check for SSE support
// ---------------------------

int info_has_sse(void)   { return info_cpu_features() & (1 << 25); }
int info_has_sse2(void)  { return info_cpu_features() & (1 << 26); }
int info_has_avx(void)   { return info_cpu_features_ext() & (1 << 28); }
int info_has_avx2(void)   { return info_cpu_features_ext() & (1 << 5);  }

// ---------------------------
// NTux-OS: Shell command to display CPU info
// ---------------------------
void info_cmd_cpuinfo(void)
{
    char vendor[13];
    info_get_cpu_vendor(vendor);

    // --- Exakter CPU-Name (wie "Intel(R) Core(TM) i3-3110M CPU @ 2.40GHz") ---
    char brand[49] = {0};
    uint32_t* p = (uint32_t*)brand;

    // CPUID 0x80000002 - 0x80000004 = 48 Bytes Brand String
    for (int i = 0; i < 3; i++) {
        asm volatile("cpuid"
                     : "=a"(p[4*i + 0]), "=b"(p[4*i + 1]),
                       "=c"(p[4*i + 2]), "=d"(p[4*i + 3])
                     : "a"(0x80000002 + i));
    }

    // Leerzeichen am Anfang/Ende entfernen
    char* start = brand;
    while (*start == ' ') start++;
    char* end = start + strlen(start) - 1;
    while (end > start && *end == ' ') *end-- = '\0';

    kprint("CPU Model  : ");
    kprint(start);
    kprint("\n");

    kprint("Vendor     : ");
    kprint(vendor);
    kprint("\n");

    kprint("Features   :");
    if (info_has_sse())   kprint(" SSE");
    if (info_has_sse2())  kprint(" SSE2");
    if (info_has_avx())   kprint(" AVX");
    if (info_has_avx2())  kprint(" AVX2");
    kprint("\n");
}

// ---------------------------
// NTux-OS: Shell command to display memory info
// ---------------------------
void info_cmd_meminfo(void)
{
    //uint32_t free = pmm_get_free_memory_32() ;


    kprint("Free RAM   : ");
    //kprint_uint(free / 1024 / 1024);
    kprint(" MiB\n");
}


// ---------------------------
// NTux-OS: Shell command to display system uptime
// ---------------------------
void info_cmd_uptime(void)
{
    uint64_t ticks = get_tick_count();        
    uint64_t total_ms  = ticks;               
    uint64_t hours     = total_ms / 3600000;
    uint64_t minutes   = (total_ms % 3600000) / 60000;
    uint64_t seconds   = (total_ms % 60000) / 1000;
    uint64_t ms        = total_ms % 1000;

    kprint("Uptime     : ");
    if (hours)   kprint_uint(hours),   kprint("h ");
    if (hours || minutes) kprint_uint(minutes), kprint("m ");
    kprint_uint(seconds);
    kprint(".");
    if (ms < 100) kprint("0");
    if (ms < 10)  kprint("0");
    kprint_uint(ms);
    kprint("s\n");
}