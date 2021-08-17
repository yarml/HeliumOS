megs:            64
display_library: x
romimage:        file=BOCHS_ROM_IMAGE
vgaromimage:     file=BOCHS_VGA_ROM_IMAGE
ata0-master:     type=cdrom, path=ISO, status=inserted
boot:            cdrom
log:             bochslog.txt
clock:           sync=realtime, time0=local, rtc_sync=0
cpu:             count=1, ips=1000000
port_e9_hack:    enabled=1
magic_break:     enabled=1
com1:            enabled=1, mode=file, dev=com1.out
mouse:           enabled=1, type=ps2, toggle=ctrl+alt

