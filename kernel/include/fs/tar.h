#ifndef HELIUM_FS_TAR_H
#define HELIUM_FS_TAR_H

#include <attributes.h>
#include <fs.h>

typedef struct TAR_HEADER tar_header;

#define TAR_ENTRY_TYPE_FILE ('0')
#define TAR_ENTRY_TYPE_HLINK ('1')
#define TAR_ENTRY_TYPE_SLINK ('2')
#define TAR_ENTRY_TYPE_DIR ('5')

struct TAR_HEADER {
  char name[100];
  char mode[8];
  char user[8];
  char group[8];
  char size[12];
  char lmod[12];
  char checksum[8];
  char type;
  char link_name[100];
  char ustar[6];
  char version[2];
  char user_name[32];
  char group_name[32];
  char dev_major[8];
  char dev_minor[8];
  char name_pref[155];
  char undef[12];
} pack;

void     tar_header_print(tar_header *header);

size_t   tar_file_size(tar_header *header);

// Make immutable memory filesystem
filesys *tar_mkimfs(char *fsname, void *membuf, size_t size);

// Get direct access to the memory region of a file
void    *tarfs_direct_access(fsnode *f);

#endif