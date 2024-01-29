#ifndef HELIUM_TAR_H
#define HELIUM_TAR_H

#include <attributes.h>

// These are the ones we support
#define TAR_ENTRY_TYPE_FILE ('0')
#define TAR_ENTRY_TYPE_DIR ('5')

typedef struct TAR_HEADER {
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
  char res0[12];
} pack tar_header;

#endif
