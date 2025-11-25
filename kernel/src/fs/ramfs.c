#include "ramfs.h"
#include <drivers/framebuffer/kprint.h>
#include <mem/kmalloc.h>
#include <mem/umalloc.h>
#include <kernel_lib/string.h>

static ramfs_node_t* ramfs_root = NULL;



static char* next_token(char** str) {
    if (!str || !*str || **str == '\0') return NULL;

    char* start = *str;

    
    while (*start == '/') start++;
    if (*start == '\0') {
        *str = start;
        return NULL;
    }

    char* end = start;
    while (*end && *end != '/') end++;

    if (*end == '/') {
        *end = '\0';           
        *str = end + 1;        
    } else {
        *str = end;            
    }

    return start;
}

static ramfs_node_t* ramfs_find_node(const char* path, int create_missing) {
    kprint("[DEBUG] ramfs_find_node: '"); kprint(path); kprint("'\n");

    if (!ramfs_root) {
        kprint("[DEBUG] ramfs_root is NULL!\n");
        return NULL;
    }
    kprint("[DEBUG] ramfs_root OK\n");

    if (!path || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        kprint("[DEBUG] returning root\n");
        return ramfs_root;
    }

    char temp[256];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    kprint("[DEBUG] path copy: '"); kprint(temp); kprint("'\n");

    ramfs_node_t* cur = ramfs_root;
    char* p = temp;
    char* token;

    while ((token = next_token(&p))) {
        if (token[0] == '\0') continue;

        kprint("[DEBUG] token: '"); kprint(token); kprint("'\n");

        int found = 0;
        for (size_t i = 0; i < cur->child_count; i++) {
            kprint("[DEBUG]   compare with child["); 
            char buf[16]; itoa(i, buf, 10); kprint(buf); kprint("]: '");
            kprint(cur->children[i]->name); kprint("'\n");

            if (strcmp(cur->children[i]->name, token) == 0) {
                cur = cur->children[i];
                found = 1;
                kprint("[DEBUG]   FOUND!\n");
                break;
            }
        }

        if (!found) {
            kprint("[DEBUG]   NOT FOUND\n");
            if (!create_missing) return NULL;
            ramfs_node_t* new_node = kmalloc(sizeof(ramfs_node_t));
            if (!new_node) {
                kprint("[DEBUG] kmalloc failed for new node!\n");
                return NULL;
            }
            memset(new_node, 0, sizeof(ramfs_node_t));
            strncpy(new_node->name, token, RAMFS_MAX_NAME - 1);
            new_node->name[RAMFS_MAX_NAME - 1] = '\0';
            new_node->type = RAMFS_DIR;
            new_node->parent = cur;
            new_node->child_count = 0;

            if (cur->child_count >= RAMFS_MAX_CHILDREN) {
                kprint("[DEBUG] DIR FULL!\n");
                return NULL;
            }

            cur->children[cur->child_count++] = new_node;
            cur = new_node;
            kprint("[DEBUG]   CREATED new dir: '"); kprint(token); kprint("'\n");
        }
    }

    kprint("[DEBUG] returning node\n");
    return cur;
}

void ramfs_init(void) {
    ramfs_root = kmalloc(sizeof(ramfs_node_t));
    if (!ramfs_root) {
        kprint("[RAMFS] ERROR: kmalloc returned NULL!\n");
        return;
    }
    memset(ramfs_root, 0, sizeof(ramfs_node_t));
    ramfs_root->name[0] = '\0';
    ramfs_root->type = RAMFS_DIR;
    ramfs_root->child_count = 0;
    kprint("[RAMFS] Initialized\n");
}

ramfs_node_t* ramfs_mkdir(const char* path) {
    if (!path || !ramfs_root) return NULL;

    ramfs_node_t* node = ramfs_find_node(path, 1);
    if (!node) {
        kprint("[RAMFS] Failed to mkdir: "); kprint(path); kprint("\n");
        return NULL;
    }

    if (node->type != RAMFS_DIR) {
        kprint("[RAMFS] Path exists but not dir: "); kprint(path); kprint("\n");
        return NULL;
    }

    kprint("[RAMFS] mkdir: "); kprint(path); kprint("\n");
    return node;
}

ramfs_node_t* ramfs_create_file(const char* path, const char* data) {
    if (!path || !data || !ramfs_root) return NULL;

    char path_copy[256];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char* last_slash = strrchr(path_copy, '/');
    if (!last_slash || last_slash == path_copy) return NULL;
    *last_slash = '\0';
    char* filename = last_slash + 1;
    if (filename[0] == '\0') return NULL;

    ramfs_node_t* dir = ramfs_find_node(path_copy, 1);
    if (!dir || dir->type != RAMFS_DIR) return NULL;

    for (size_t i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, filename) == 0) {
            kprint("[RAMFS] File exists: "); kprint(path); kprint("\n");
            return NULL;
        }
    }

    if (dir->child_count >= RAMFS_MAX_CHILDREN) {
        kprint("[RAMFS] ERR: Dir full\n");
        return NULL;
    }

    ramfs_node_t* file = kmalloc(sizeof(ramfs_node_t));
    memset(file, 0, sizeof(ramfs_node_t));
    strncpy(file->name, filename, RAMFS_MAX_NAME - 1);
    file->name[RAMFS_MAX_NAME - 1] = '\0';
    file->type = RAMFS_FILE;
    file->size = strlen(data);
    file->data = kmalloc(file->size + 1);
    strcpy(file->data, data);
    file->parent = dir;

    dir->children[dir->child_count++] = file;

    kprint("[RAMFS] Created file: "); kprint(path); kprint("\n");
    return file;
}

ramfs_node_t* ramfs_delete_file(const char* path) {
    if (!path || !ramfs_root) return NULL;

    ramfs_node_t* node = ramfs_find_node(path, 0);
    if (!node || node->type != RAMFS_FILE) {
        kprint("[RAMFS] Not found or not file: "); kprint(path); kprint("\n");
        return NULL;
    }

    ramfs_node_t* parent = node->parent;
    if (!parent) {
        kprint("[RAMFS] File has no parent: "); kprint(path); kprint("\n");
        return NULL;
    }

    int found_index = -1;
    for (size_t i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == node) {
            found_index = (int)i;
            break;
        }
    }

    if (found_index == -1) {
        kprint("[RAMFS] File not found in parent's children: "); kprint(path); kprint("\n");
        return NULL;
    }

    for (size_t i = found_index; i < parent->child_count - 1; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->child_count--;

    kfree(node->data);
    kfree(node);

    kprint("[RAMFS] Deleted file: "); kprint(path); kprint("\n");
    return parent;
}

ramfs_node_t* ramfs_read_file(const char* path) {
    ramfs_node_t* node = ramfs_find_node(path, 0);
    if (!node || node->type != RAMFS_FILE) {
        kprint("[RAMFS] Not found or not file: "); kprint(path); kprint("\n");
        return NULL;
    }

    kprint("[RAMFS] Read: "); kprint(node->name); kprint(" => "); kprint(node->data); kprint("\n");
    return node;
}


void ramfs_list_dir(const char* path) {
    const char* dirpath = path && path[0] ? path : "/";
    ramfs_node_t* dir = ramfs_find_node(dirpath, 0);
    if (!dir || dir->type != RAMFS_DIR) {
        kprint("[RAMFS] Dir not found: "); kprint(dirpath); kprint("\n");
        return;
    }

    kprint("[RAMFS] ls "); kprint(dirpath); kprint("\n");
    for (size_t i = 0; i < dir->child_count; i++) {
        kprint("  ");
        kprint(dir->children[i]->name);
        kprint(dir->children[i]->type == RAMFS_DIR ? "/" : "");
        kprint("\n");
    }
}

void* ramfs_read_file_to_memory(const char* path, size_t* size) {
    if (!path || !size) return NULL;

    ramfs_node_t* node = ramfs_find_node(path, 0);
    if (!node || node->type != RAMFS_FILE) {
        kprint("[RAMFS] File not found or not a file: "); kprint(path); kprint("\n");
        *size = 0;
        return NULL;
    }

    *size = node->size;
    void* buffer = kmalloc(*size);
    if (!buffer) {
        kprint("[RAMFS] Failed to allocate memory for file: "); kprint(path); kprint("\n");
        *size = 0;
        return NULL;
    }

    for (size_t i = 0; i < *size; i++) {
        ((uint8_t*)buffer)[i] = (uint8_t)node->data[i];
    }

    return buffer;
}
