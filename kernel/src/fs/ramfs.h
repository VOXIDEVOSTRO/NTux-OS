#ifndef RAMFS_H
#define RAMFS_H

#include <stddef.h>
#include <stdint.h>

#define RAMFS_MAX_NAME      32
#define RAMFS_MAX_CHILDREN  16

typedef enum {
    RAMFS_FILE,
    RAMFS_DIR
} ramfs_node_type_t;

typedef struct ramfs_node {
    char name[RAMFS_MAX_NAME];
    ramfs_node_type_t type;
    struct ramfs_node* parent;
    struct ramfs_node* children[RAMFS_MAX_CHILDREN];  
    size_t child_count;
    char* data;       
    size_t size;      
} ramfs_node_t;


void ramfs_init(void);
ramfs_node_t* ramfs_mkdir(const char* path);
ramfs_node_t* ramfs_create_file(const char* path, const char* data);
ramfs_node_t* ramfs_read_file(const char* path);
void ramfs_list_dir(const char* path);
void* ramfs_read_file_to_memory(const char* path, size_t* size);
ramfs_node_t* ramfs_delete_file(const char* path);

#endif