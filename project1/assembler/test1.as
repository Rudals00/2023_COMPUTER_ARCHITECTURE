        lw 0 1 one
        lw 1 2 ten
start   add 3 1 3
        beq 3 2 done
        beq 0 0 start
done    noop
        halt
ten     .fill 10
one     .fill 1
