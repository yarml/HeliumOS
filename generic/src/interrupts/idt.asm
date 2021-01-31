global  load_idt

; void load_idt(idt);
load_idt:
    lidt    [esp + 4]
    sti
    ret