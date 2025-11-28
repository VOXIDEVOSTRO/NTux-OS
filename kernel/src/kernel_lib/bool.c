#include <kernel_lib/bool.h>
#include <drivers/framebuffer/kprint.h>

const char* boolean_to_string(boolean b) {
    return b ? "true" : "false";
}

void print_bool(const char* name, boolean value) {
    kprint(name);
    kprint(" = ");
    kprint(boolean_to_string(value));
    kprint("\n");
}