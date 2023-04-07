#ifndef HELIUM_FS_H
#define HELIUM_FS_H

/* Helium filesystem definition, heavily ispired from Unix */

#include <stddef.h>
#include <stdint.h>

typedef enum FSNODE_TYPE fsnode_type;

typedef uint64_t dircap;
typedef uint64_t filecap;

#define FSCAP_USED      (1<<0)

#define FSCAP_FREAD     (1<<1)
#define FSCAP_FWRITE    (1<<2)
#define FSCAP_FAPPEND   (1<<3)
#define FSCAP_FPULL     (1<<4)
#define FSCAP_FTELLSIZE (1<<5)

#define FSCAP_DLIST     (1<<1)
#define FSCAP_DTELLSIZE (1<<2)
#define FSCAP_DCREAT    (1<<3)

typedef struct FS_IMPL fsimpl;

typedef struct FILESYS filesys;
typedef struct FSNODE fsnode;

#define FS_NAMELEN (16)
#define FSNODE_NAMELEN (512)

// Types of FS nodes
enum FSNODE_TYPE
{
  FSNODE_FILE,
  FSNODE_DIR,
  FSNODE_LINK
};

// If a file system cannot write/append to all files, and cannot mknode
// and rmnode from all dirs, then it is called immutable

// Structur containing function pointers to filesystem operations
struct FS_IMPL
{
  // Release any data in fs->ext
  void (*fs_release)(filesys *fs);

  // Read `size` bytes of data from file, returns number of bytes read
  size_t (*fs_file_read)(fsnode *file, size_t off, char *buf, size_t size);

  size_t (*fs_file_pull)(fsnode *file, char *buf, size_t size);

  size_t (*fs_file_skip)(fsnode *file, size_t size); // Default impl

  size_t (*fs_file_tellsize)(fsnode *file);
};

// Structure that defines a filesystem
struct FILESYS
{
  char name[FS_NAMELEN];

  // default capabilities
  dircap dir_cap;
  filecap file_cap;

  fsnode *root;

  fsimpl impl;

  // Filesystems can use this pointer as they please
  void *ext;
};

struct FSNODE
{
  char name[FSNODE_NAMELEN];

  filesys *fs;
  fsnode_type type;

  fsnode *parent; // can be NULL for a directory
  fsnode *nsib; // next sibling
  fsnode *psib; // prev sibling

  // How many times the file has been open
  // This is used for files in large filesystems that
  // can cache their nodes into disk or something
  // when refcount is not zero, the node should not be cached
  size_t refcount;

  // Each filesystem is free to use this pointer as they wish
  // I imagine it can store info to where the data can be read/written to
  // info about cache if there is any etc etc
  // Filesystems are required to release any memory allocated for this pointer
  // through fsnode_detach
  void *ext;

  // The first node that is a link to this node
  fsnode *flink;

  // Fields that only exist in file xor directories xor links
  union
  {
    struct
    {
      filecap cap;
      size_t size;
    } file;

    struct
    {
      dircap cap;
      fsnode *fchild;
    } dir;

    struct
    {
      // The node this node is a link to
      fsnode *target;

      // The next & prev node that is also a link to `target`
      fsnode *nlink;
      fsnode *plink;
    } link;
  };
};

filesys *fs_mount(char *name);
filesys *fs_from_name(char *name);
void fs_umount(filesys *fs);

// name functions
int fs_valid_sys_name(char *name);
int fs_valid_node_name(char *name);
int fs_valid_path(char *path);
void fs_makecanonical(char *path, char *opath);
void fs_pathtok(char *path, char *fsname, char **nodes, size_t *len);
void fs_basename(char *path, char *name);

// TODO: when processes are implemented, al fs functions should also
// get a pointer to the process making the request
fsnode *fs_open(char *fsname, char *names, size_t depth);
fsnode *fs_search(char *path);
fsnode *fs_dirof(char *path);

void fs_close(fsnode *node);

void fs_rm(fsnode *node);

fsnode *fs_mknode(fsnode *parent, char *name);
fsnode *fs_mkdir(fsnode *parent, char *name);
fsnode *fs_mkfile(fsnode *parent, char *name);
fsnode *fs_mklink(fsnode *parent, char *name, fsnode *target);

int fs_check_fcap(fsnode *node, int cap);
int fs_check_dcap(fsnode *dir, int cap);

// FSCAP_FREAD
size_t fs_read(fsnode *file, size_t off, char *buf, size_t size);

// FSCAP_FPULL
size_t fs_pull(fsnode *file, char *buf, size_t size);
size_t fs_skip(fsnode *file, size_t size);

// FSCAP_FTELLSIZE
size_t fs_tellsize(fsnode *file);

void fs_print(filesys *fs);

void fs_init();

#endif