global as_sys_stop

as_sys_stop:
    cli
.loop:
    hlt
    jmp .loop
