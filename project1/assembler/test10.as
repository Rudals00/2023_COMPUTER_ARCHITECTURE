        lw 0 1 neg3
        lw 0 2 two
        add 1 2 3
        beq 3 4 loop
        jalr 1 2
        halt
loop    add 2 3 4
        add 4 5 6
        sw 1 2 3
ten     .fill 10
two     .fill 2
neg3    .fill -3
