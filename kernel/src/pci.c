#include <stdio.h>
#include <pci.h>

#include <asm/io.h>

uint32_t pci_read_reg(size_t bus, size_t dev, size_t fn, size_t reg)
{
  pciadr adr;
  adr.offset = reg * 4;
  adr.fn = fn;
  adr.dev = dev;
  adr.bus = bus;
  adr.enabled = 1;

  as_outd(PCI_IO_CFG_ADR, adr.asint);
  return as_ind(PCI_IO_CFG_DATA);
}

uint16_t pci_vendorid(size_t bus, size_t dev, size_t fn)
{
  return pci_read_reg(bus, dev, fn, 0) & 0x0000FFFF;
}
uint16_t pci_devid(size_t bus, size_t dev, size_t fn)
{
  return (pci_read_reg(bus, dev, fn, 0) & 0xFFFF0000) >> 16;
}

uint8_t pci_classid(size_t bus, size_t dev, size_t fn)
{
  return (pci_read_reg(bus, dev, fn, 2) & 0xFF000000) >> 24;
}
uint8_t pci_subclass(size_t bus, size_t dev, size_t fn)
{
  return (pci_read_reg(dev, bus, fn, 2) & 0x00FF0000) >> 16;
}
uint8_t pci_progif(size_t bus, size_t dev, size_t fn)
{
  return (pci_read_reg(bus, dev, fn, 2) & 0x0000FF00) >> 8;
}
uint8_t pci_revid(size_t bus, size_t dev, size_t fn)
{
  return (pci_read_reg(bus, dev, fn, 2) & 0x000000FF);
}

pci_inf pci_info(size_t bus, size_t dev, size_t fn)
{
  pci_inf info;

  uint32_t r0 = pci_read_reg(bus, dev, fn, 0);
  uint32_t r2 = pci_read_reg(bus, dev, fn, 2);
  uint32_t r3 = pci_read_reg(bus, dev, fn, 3);

  info.vendorid = r0 & 0x0000FFFF;
  info.devid = (r0 & 0xFFFF0000) >> 16;

  info.class = (r2 & 0xFF000000) >> 24;
  info.subclass = (r2 & 0x00FF0000) >> 16;
  info.progif = (r2 & 0x0000FF00) >> 8;
  info.revid = (r2 & 0x000000FF);

  info.header_type = (r3 & 0x00FF0000) >> 16;

  return info;
}

void pci_probe()
{
  for(size_t bus = 0; bus < 256; ++bus)
    for(size_t dev = 0; dev < 32; ++dev)
      for(size_t fn = 0; fn < 8; ++fn)
      {
        if(pci_vendorid(bus, dev, fn) != 0xFFFF)
        {
          pci_inf info = pci_info(bus, dev, fn);
          printf(
            "%03d:%02d:%01d "
            "VendorId: %04x; DevId: %04x; "
            "Class: %02x; Subclass: %02x; ProgIf: %02x; Rev: %02x; "
            "HType: %02x "
            "%s\n",
            bus, dev, fn,
            info.vendorid, info.devid,
            info.class, info.subclass, info.progif, info.revid,
            info.header_type,
            pci_class(info.class)
          );
        }
      }
}

char const *pci_class(uint16_t class_id)
{
  switch (class_id)
  {
  case PCI_CLASS_MASS_STORAGE:
    return "MASS_STORAGE";
  case PCI_CLASS_NET_CTL:
    return "NET_CTL";
  case PCI_CLASS_DISP_CTL:
    return "DISP_CTL";
  case PCI_CLASS_MMDEV:
    return "MMDEV";
  case PCI_CLASS_MEM_CTL:
    return "MEM_CTL";
  case PCI_CLASS_BRIDGE:
    return "BRIDGE";
  case PCI_CLASS_SIMPL_COM_CTL:
    return "SIMPL_COM_CTL";
  case PCI_CLASS_BSYS_PER:
    return "BSYS_PER";
  case PCI_CLASS_INDEV:
    return "INDEV";
  case PCI_CLASS_DOCK_ST:
    return "DOCK_ST";
  case PCI_CLASS__PROC:
    return "_PROC";
  case PCI_CLASS_SERIALB_CTL:
    return "SERIALB_CTL";
  case PCI_CLASS_WL_CTL:
    return "WL_CTL";
  case PCI_CLASS_IIO_CTL:
    return "IIO_CTL";
  case PCI_CLASS_SAT_COM_CTL:
    return "SAT_COM_CTL";
  case PCI_CLASS_ENC_CTL:
    return "ENC_CTL";
  case PCI_CLASS_DATACQ_SIGPROC_CTL:
    return "DATACQ_SIGPROC_CTL";
  case PCI_CLASS_PROC_ACCEL:
    return "PROC_ACCEL";
  case PCI_CLASS_NON_ESSENTIAL:
    return "NON_ESSENTIAL";
  case PCI_CLASS_UNDEF:
    return "UNDEF";
  default:
    return "UNK";
  }
}
