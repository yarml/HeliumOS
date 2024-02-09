WIP
# Config Table
Config tables are a way to dynamically assign handlers for particular devices.
WHen parsing tables such as PCI or ACPI, multiple types of entries can be encoutred. It makes
sense for some of these to be directly handled by the core kernel, but others would probably
be better handled by a kernel module. Config tables are a way for the core kernel, as well as
kernel modules to register handlers for different types of entries in PCI, ACPI, and possibly
other tables in the future.