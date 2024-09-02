        lw 0 1 9
        lw 0 2 10
        add 1 2 3
        sw 0 3 11
        beq 1 2 exit
        jalr 1 2
        halt
exit    noop
start   .fill 5
        .fill 7
