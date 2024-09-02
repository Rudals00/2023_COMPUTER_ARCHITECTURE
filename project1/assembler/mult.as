mcand   .fill   32766
mplier  .fill   12328
maskb   .fill   -2
add1    .fill   1
sub1    .fill   -1
maxbit  .fill   15
        lw      0 2 mplier
        lw      0 3 mcand
        lw      0 4 maskb
        lw      0 5 maxbit
        noop
loop    nor     2 4 7
        beq     0 7 calc
        beq     0 0 proc
calc    add     1 3 1
proc    lw      0 7 sub1
        add     5 7 5
        beq     0 5 done
        add     3 3 3
        add     4 4 4
        lw      0 7 add1
        add     4 7 4
        beq     0 0 loop
done    noop
        halt
