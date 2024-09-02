/* LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000 

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    int count =0;
    stateType state;
    FILE *filePtr;
    int inst;
    int opcode;
    int reg1, reg2, dest, offset;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    
    for(int i = 0; i <NUMREGS; i++)
        state.reg[i] = 0;

    while(1) {
        
        printState(&state);
        
        int inst = state.mem[state.pc];

        state.pc++;
        count++;

        if(count > 200) {
            printf("error: infinite loop\n");
            exit(1);
        }

        opcode = (inst >> 22) & 7;
        reg1 = (inst >> 19) & 7;
        reg2 = (inst >> 16) & 7;
        dest = (inst >> 0) & 7;
        


        switch (opcode) {
            case 0: // [R-TYPE] add
                state.reg[dest] = state.reg[reg1] + state.reg[reg2];
                break;
            case 1: // [R-TYPE] nor
                state.reg[dest] = ~(state.reg[reg1] | state.reg[reg2]);
                break;
            case 2: // [I-TYPE] lw
                offset = inst & 0xffff;
                if(offset > 0x7fff)
                offset = offset | (0xffff << 16);
                state.reg[reg2] = state.mem[state.reg[reg1] + offset];
                break;
            case 3: // [I-TYPE] sw
                offset = inst & 0xffff;
                if(offset > 0x7fff)
                offset = offset | (0xffff << 16);
                state.mem[state.reg[reg1] + offset] = state.reg[reg2];
                break;
            case 4: // [I-TYPE] beq
                offset = inst & 0xffff;
                if(offset > 0x7fff)
                offset = offset | (0xffff << 16);
                if (state.reg[reg1] == state.reg[reg2]) {
                    state.pc += offset;
                    if (state.pc < 0) {
                        printf("error: branching to negative address\n");
                        exit(1);
                    }
                }
                break;
            case 5: // [J-TYPE] jalr
                state.reg[dest] = state.pc;
                state.pc = state.reg[reg1];

                if(state.pc < 0) {
                printf("error branching to negative address\n");
                exit(1);
            }
                break;
            case 6: // [O-TYPE] halt
                printf("machine halted\n");
                printf("total of %d instructions executed\n", count);
                printf("final state of machine:\n");
                printState(&state);
                exit(0);
                break;
            case 7: // [O-TYPE] noop
                break;
            default:
                printf("error: unrecognized opcode %d\n", opcode);
                exit(1);
                break;
        }
    }

    return 0;
}



void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}