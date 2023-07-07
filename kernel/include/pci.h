#ifndef HELIUM_PCI_H
#define HELIUM_PCI_H

#include <attributes.h>
#include <stdint.h>

#define PCI_IO_CFG_ADR (0x0CF8)
#define PCI_IO_CFG_DATA (0x0CFC)

typedef struct PCI_ADR pciadr;
struct PCI_ADR {
  union {
    struct {
      uint64_t offset  : 8;
      uint64_t fn      : 3;
      uint64_t dev     : 5;
      uint64_t bus     : 8;
      uint64_t res0    : 7;
      uint64_t enabled : 1;
    } pack;
    uint32_t asint;
  } pack;
} pack;

typedef struct PCI_INF pci_inf;
struct PCI_INF {
  uint16_t vendorid;
  uint16_t devid;

  uint8_t class;
  uint8_t subclass;
  uint8_t progif;
  uint8_t revid;

  uint8_t header_type;
};

uint32_t    pci_read_reg(size_t bus, size_t dev, size_t fn, size_t reg);
uint16_t    pci_vendorid(size_t bus, size_t dev, size_t fn);

uint8_t     pci_classid(size_t bus, size_t dev, size_t fn);
uint8_t     pci_subclass(size_t bus, size_t dev, size_t fn);
uint8_t     pci_progif(size_t bus, size_t dev, size_t fn);
uint8_t     pci_revid(size_t bus, size_t dev, size_t fn);

pci_inf     pci_info(size_t bus, size_t dev, size_t fn);

void        pci_probe();

char const *pci_class(uint16_t class_id);

#define PCI_CLASS_MASS_STORAGE (0x01)
#define PCI_CLASS_NET_CTL (0x02)
#define PCI_CLASS_DISP_CTL (0x03)
#define PCI_CLASS_MMDEV (0x04)
#define PCI_CLASS_MEM_CTL (0x05)
#define PCI_CLASS_BRIDGE (0x06)
#define PCI_CLASS_SIMPL_COM_CTL (0x07)
#define PCI_CLASS_BSYS_PER (0x08)
#define PCI_CLASS_INDEV (0x09)
#define PCI_CLASS_DOCK_ST (0x0A)
#define PCI_CLASS__PROC (0x0B)
#define PCI_CLASS_SERIALB_CTL (0x0C)
#define PCI_CLASS_WL_CTL (0x0D)
#define PCI_CLASS_IIO_CTL (0x0E)
#define PCI_CLASS_SAT_COM_CTL (0x0F)
#define PCI_CLASS_ENC_CTL (0x10)
#define PCI_CLASS_DATACQ_SIGPROC_CTL (0x11)
#define PCI_CLASS_PROC_ACCEL (0x12)
#define PCI_CLASS_NON_ESSENTIAL (0x13)
#define PCI_CLASS_UNDEF (0xFF)

#endif