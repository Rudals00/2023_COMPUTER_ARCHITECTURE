#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLINELENGTH 1000

typedef struct {
    int location;
    char label[MAXLINELENGTH];
} LabelData;

const char *rTypeOpcodes[] = {"add", "nor"};
const int rTypeOpcodesSize = sizeof(rTypeOpcodes) / sizeof(rTypeOpcodes[0]);

const char *iTypeOpcodes[] = {"lw", "sw", "beq"};
const int iTypeOpcodesSize = sizeof(iTypeOpcodes) / sizeof(iTypeOpcodes[0]);

const char *jTypeOpcodes[] = {"jalr"};
const int jTypeOpcodesSize = sizeof(jTypeOpcodes) / sizeof(jTypeOpcodes[0]);

const char *oTypeOpcodes[] = {"halt", "noop"};
const int oTypeOpcodesSize = sizeof(oTypeOpcodes) / sizeof(oTypeOpcodes[0]);

LabelData labels[MAXLINELENGTH];
int labelsCount = 0;

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isWhitespace(char);
int isInOpcodes(char *, const char **, int);
int isNumber(char *);
void buildLabels(FILE *);
void abortWithError(char *);
int formatRTypeInst(char *, char *, char *, char *);
int formatITypeInst(char *, char *, char *, char *, int);
int formatJTypeInst(char *, char *);
int formatOTypeInst(char *);
int getValueInLabels(char *);
int isValidRegister(char *, int);

int main(int argc, char *argv[]) {
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // Build Label Data for Assembler
    buildLabels(inFilePtr);

    int line = 0;

    // Assembler Main Logic
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        int mc = 0;

        if (isInOpcodes(opcode, rTypeOpcodes, rTypeOpcodesSize)) {
            mc = formatRTypeInst(opcode, arg0, arg1, arg2);
        }
        else if (isInOpcodes(opcode, iTypeOpcodes, iTypeOpcodesSize)) {
            mc = formatITypeInst(opcode, arg0, arg1, arg2, line);
        }
        else if (isInOpcodes(opcode, jTypeOpcodes, jTypeOpcodesSize)) {
            mc = formatJTypeInst(arg0, arg1);
        }
        else if (isInOpcodes(opcode, oTypeOpcodes, oTypeOpcodesSize)) {
            mc = formatOTypeInst(opcode);
        }
        else if (!strcmp(opcode, ".fill")) {
            if (isNumber(arg0)) {
                mc = atoi(arg0);
            }
            else {
                mc = getValueInLabels(arg0);
            }
        }
        else {
            abortWithError("Do not support its opcode.");
        }

        fprintf(outFilePtr, "%d\n", mc);
        printf("(Address %d): %d\n", line, mc);

        line++;
    }

    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(0);
}

int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
		char *arg1, char *arg2)
{
	char line[MAXLINELENGTH];
	char *ptr = line;

	/* delete prior values */
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

	/* read the line from the assembly-language file */
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		/* reached end of file */
		return(0);
	}

	/* check for line too long (by looking for a \n) */
	if (strchr(line, '\n') == NULL) {
		/* line too long */
		printf("error: line too long\n");
		exit(1);
	}

	/* is there a label? */
	ptr = line;
	if (sscanf(ptr, "%[^\t\n\r ]", label)) {
		/* successfully read label; advance pointer over the label */
		ptr += strlen(label);
	}

	/*
	 * Parse the rest of the line.  Would be nice to have real regular
	 * expressions, but scanf will suffice.
	 */
	sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%"
			"[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
	return(1);
}

int isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int isInOpcodes(char *opcode, const char **opcodes, int size) {
    for (int i = 0; i < size; i++) {
        if (!strcmp(opcode, opcodes[i])) {
            return 1;
        }
    }
    return 0;
}

int isNumber(char *string) {
    int i;
    return ((i = sscanf(string, " %d", &i)) == 1);
}

void buildLabels(FILE *inFilePtr) {
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int line = 0;

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        if (label[0] != '\0') {
            // Check for duplicate label
            for (int i = 0; i < labelsCount; i++) {
                if (strcmp(label, labels[i].label) == 0) {
                    abortWithError("Duplicate definition of labels.");
                }
            }
            strcpy(labels[labelsCount].label, label);
            labels[labelsCount].location = line;
            labelsCount++;
        }
        line++;
    }

    // Reset the file pointer to the beginning of the file
    rewind(inFilePtr);
}

void abortWithError(char *errorMessage) {
    printf("%s\n", errorMessage);
    exit(1);
}

int formatRTypeInst(char *opcode, char *arg0, char *arg1, char *arg2) {
    int op, regA, regB, destReg;

	if (!isValidRegister(arg0, 0) || !isValidRegister(arg1, 0) || !isValidRegister(arg2, 1))
    {
        abortWithError("Registers are not valid.");
    }
    op = (strcmp(opcode, "add") == 0) ? 0 : 1;
    regA = atoi(arg0);
    regB = atoi(arg1);
    destReg = atoi(arg2);
    return (op << 22) | (regA << 19) | (regB <<16 ) | destReg;
}

int formatITypeInst(char *opcode, char *arg0, char *arg1, char *arg2, int currentLine)
{
    int mc = 0, address = 0;

    // Valid Check All Reg
    if (!isValidRegister(arg0, 0) || !isValidRegister(arg1, 0))
    {
        abortWithError("Registers are not valid.");
    }

    // Opcode Formatting
    if (!strcmp(opcode, "lw"))
    {
        mc |= (2 << 22);
    }
    else if (!strcmp(opcode, "sw"))
    {
        mc |= (3 << 22);
    }
    else if (!strcmp(opcode, "beq"))
    {
        mc |= (4 << 22);
    }
    else
    {
        abortWithError("Wrong opcode for I Type Inst Formatting");
    }

    // Address Formatting
    mc |= (atoi(arg0) << 19);
    mc |= (atoi(arg1) << 16);

    if (isNumber(arg2))
    {
        address = atoi(arg2);
        if (address > 32767 || address < -32768)
        {
            abortWithError("Offset out of boundary");
        }
    }
    else
    {
        address = getValueInLabels(arg2);
    }

    if (!strcmp(opcode, "beq") && !isNumber(arg2))
    {
        int dest = address;
        address = dest - currentLine - 1;
        if (address > 32767 || address < -32768)
        {
            abortWithError("Offset out of boundary");
        }
    }

    address &= 0xFFFF;
    mc |= address;

    return mc;
}

int formatJTypeInst(char *arg0, char *arg1) {
	if (!isValidRegister(arg0, 0) || !isValidRegister(arg1, 0))
    {
        abortWithError("Registers are not valid.");
    }
	int op = 5; // "jalr"
	int regA = atoi(arg0);
	int regB = atoi(arg1);
	return (op << 22) | (regA << 19) | (regB << 16);
}

int formatOTypeInst(char *opcode) {
	int op = (strcmp(opcode, "halt") == 0) ? 6 : 7; // "halt" or "noop"
	return op << 22;
}

int getValueInLabels(char *label) {
	for (int i = 0; i < labelsCount; i++) {
		if (!strcmp(label, labels[i].label)) {
			return labels[i].location;
		}
	}
	abortWithError("Label not found.");
	return -1; // This line will never be executed
}

int isValidRegister(char *reg, int isDestReg)
{
    int regValue = 0;

    if (!isNumber(reg))
    {
        return 0;
    }

    regValue = atoi(reg);
    if (regValue < 0 || regValue >= 8)
    {
        return 0;
    }

    if (isDestReg && regValue == 0)
    {
        return 0;
    }

    return 1;
}