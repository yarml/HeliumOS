#ifndef HELIUM_SMBIOS_H
#define HELIUM_SMBIOS_H

#include <stdint.h>
#include <pack.h>

typedef struct
{
    char     anchor[4]        ;
    int8_t   checksum         ;
    uint8_t  len              ;
    uint8_t  major            ;
    uint8_t  minor            ;
    uint16_t max_size         ;
    uint8_t  revision         ;
    uint8_t  formatted_area[5];
    char     anchor2[5]       ;
    int8_t   checksum2        ;
    uint16_t struct_len       ;
    uint32_t struct_adr       ;
    uint16_t struct_count     ;
    uint8_t  bcd_rev          ;
} pack smbios_entry_point;


#endif