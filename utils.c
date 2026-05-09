#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

/* Removes leading and trailing whitespace from the given string*/
void trim_space(char *str) {
char *start = str;
char *end;

	/* Skip leading spaces*/
	while (isspace((unsigned char)*start)) start++;

	/* If the string is empty or all spaces*/
	if (*start == 0) {
        	str[0] = '\0';
        	return;
	}

	/* Move to the end of trimmed content*/
	end = start + strlen(start) - 1;

	while (end > start && isspace((unsigned char)*end)) end--;

	*(end + 1) = '\0';

/* Shift the trimmed content to beginning of the string*/
memmove(str, start, end - start + 2);
}

/*
 * Checks whether the given line is empty or a comment (starts with ';').
 * Returns 1 if true, 0 otherwise.
 */
int is_empty_or_comment(const char *line) {
	while (*line) {
		if (!isspace((unsigned char)*line) && *line != ';') return 0;
        	if (*line == ';') return 1;
        	line++;
	}
return 1;
}

/*
 * Determines the addressing mode of an operand string
 * Returns:
 *   0 - immediate addressing 
 *   1 - direct addressing 
 *   2 - matrix addressing
 *   3 - register addressing 
 *  -1 - invalid operand
 */
int get_address_mode(const char *operand) {
const char *bracket1, *bracket2, *end_bracket;
char label[32] = {0};

	if (!operand || !*operand)
		return -1;

	/* Skip leading spaces*/
	while (isspace((unsigned char)*operand)) operand++;

	/* Immediate value*/
	if (operand[0] == '#') {
        	return 0;
	}

	/* Register: r0 to r7*/
	if (operand[0] == 'r' && isdigit(operand[1]) && operand[2] == '\0') {
        	return 3;
	}

	/* Matrix*/
	bracket1 = strchr(operand, '[');
	bracket2 = bracket1 ? strchr(bracket1 + 1, '[') : NULL;
	end_bracket = bracket2 ? strchr(bracket2 + 1, ']') : NULL;

	if (bracket1 && bracket2 && end_bracket) {
		int label_len = bracket1 - operand;
        	if (label_len > 0 && label_len < 31) {
			strncpy(label, operand, label_len);
			label[label_len] = '\0';

			/* Reject if label itself looks like a register*/
			if (!(label[0] == 'r' && isdigit(label[1]) && label[2] == '\0')) {
				return 2;
			}
		}
	}

	if(bracket1 && !bracket2){
		return 2; 
	}

return 1;
}

/*
 * Splits a comma-separated operand string into up to 2 operands,
 * preserving matrix notation
 * Returns the number of operands parsed (1 or2).
 */
int split_operands(const char *input, char operands[2][31]) {
int count = 0;
int i = 0, bracket_depth = 0;
char temp[31];

	while (*input && count < 2) {
        	if (*input == '[') bracket_depth++;
        	if (*input == ']') bracket_depth--;

        	/* Split only at commas that are outside brackets*/
        	if (*input == ',' && bracket_depth == 0) {
			temp[i] = '\0';
			trim_space(temp);
			strncpy(operands[count], temp, 30);
			operands[count][30] = '\0';

			count++;
			i = 0;
			input++;
			continue;
		}

		if (i < 30)
			temp[i++] = *input;

		input++;
	}

	temp[i] = '\0';
	trim_space(temp);
	strncpy(operands[count], temp, 30);
	operands[count][30] = '\0';

return count + 1;
}






