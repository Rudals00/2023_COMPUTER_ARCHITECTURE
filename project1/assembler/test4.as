init    add 1 2 3
        lw 1 2 3
        sw 1 2 3
        beq 1 2 done
        jalr 1 2
        halt
done    noop
        .fill init
