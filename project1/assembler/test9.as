        lw 0 1 five
        lw 0 2 ten
        add 1 2 3
        beq 3 4 exit
        jalr 1 2
        halt
exit    noop
ten     .fill 10
five    .fill 5
