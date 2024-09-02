        lw 0 1 6
        lw 0 2 7
        add 1 2 3
        sw 0 3 8
        beq 1 2 loop
        jalr 3 4
        halt
loop    add 3 2 1
        noop
num1    .fill 4
num2    .fill 3
