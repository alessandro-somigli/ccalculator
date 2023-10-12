#include<stdlib.h>
#include <ctype.h>
#include<stdio.h>

// data structs
enum TokenType {
	INTEGER,
	FLOAT,
	ADD,
	SUB,
	MUL,
	DIV,
	LPAR,
	RPAR
};

union TokenValue {
	int int_v;
	double double_v;
	char char_v;
};

struct Token {
	enum TokenType type;
	union TokenValue value;
};

// globals
FILE *_file;

// functions
void throw_err(char *err);

void exec_instr(struct Token tokens[], int s_tokens);

double solve_expr(struct Token tokens[], int s_tokens);
double solve_term(struct Token tokens[], int s_tokens, int* i_ptr);
double solve_factor(struct Token tokens[], int s_tokens, int* i_ptr);

// impl
void throw_err(char *err) {
	printf("Error - %s", err);
	exit(1);
}

double solve_parenthesis(struct Token tokens[], int s_tokens, int* i_ptr) {
	struct Token sub_tokens[s_tokens-2]; // s_tokens-2 -> worst case but without parentheses
	int i_sub_tokens = 0;
	
	while (tokens[++(*i_ptr)].type != RPAR) {
		if (tokens[*i_ptr].type == LPAR) {
			struct Token* nest_tokens = &tokens[*i_ptr];
			int result = solve_parenthesis(nest_tokens, s_tokens-(*i_ptr), i_ptr);
		} else {
			sub_tokens[i_sub_tokens++] = tokens[*i_ptr];
		}
	}
	
	*i_ptr += i_sub_tokens+2;
	
	return solve_expr(sub_tokens, i_sub_tokens);
}

double solve_factor(struct Token tokens[], int s_tokens, int* i_ptr) {
	int i = *i_ptr;       
	             
	switch(tokens[i].type) {
		case INTEGER:
			return (double) tokens[(*i_ptr)++].value.int_v;
		case FLOAT:
			return tokens[(*i_ptr)++].value.double_v;
		case LPAR: {
			struct Token sub_tokens[s_tokens-2]; // s_tokens-2 -> worst case but without parentheses
			int i_sub_tokens = 0;
			
			int nested = 0;
			while (1) {
				i++;
				if (tokens[i].type == LPAR) nested++;
				else if (tokens[i].type == RPAR) {
                    if (nested > 0) nested--;
                    else break; // Found the closing parenthesis for the current expression
                }
			 	sub_tokens[i_sub_tokens++] = tokens[i];
			}
			
			// bug when nested brackets
			*i_ptr += i_sub_tokens+2;
			
			double val = solve_expr(sub_tokens, i_sub_tokens);
			return val;
		}
		default:
			throw_err("Invalid Expression: something went wrong while resolving an expression");
	}
}

double solve_term(struct Token tokens[], int s_tokens, int* i_ptr) {
	double result = solve_factor(tokens, s_tokens, i_ptr);
	
	int i = *i_ptr;
	
	while ((tokens[*i_ptr].type == MUL || tokens[*i_ptr].type == DIV) && *i_ptr < s_tokens) {
		switch(tokens[(*i_ptr)++].type) {
			case MUL:
				result *= solve_factor(tokens, s_tokens, i_ptr);
				break;
			case DIV:
				result /= solve_factor(tokens, s_tokens, i_ptr);
				break;
		}
	}
	
	return result;
}

double solve_expr(struct Token tokens[], int s_tokens) {
	int i = 0;
	
	double result = solve_term(tokens, s_tokens, &i);
	
	
	while ((tokens[i].type == ADD || tokens[i].type == SUB) && i < s_tokens) {
		switch(tokens[i++].type) {
			case ADD:
				result += solve_term(tokens, s_tokens, &i);
				break;
			case SUB:
				result -= solve_term(tokens, s_tokens, &i);
				break;
		}
	}
	
	return result;
}

void exec_instr(struct Token tokens[], int s_tokens) {}

struct Token get_num(char init_char) {
	struct Token token = {INTEGER, 0};
	
	char num_str[12] = {init_char};
	char *end_ptr;
	
	int i_num = 1;
	
	int ch = fgetc(_file);
	while ('0' <= ch && ch <= '9' || ch == '.') {
		if (ch == '.') token.type = FLOAT;
		
		num_str[i_num++] = ch;
		ch = fgetc(_file);
	}
	ungetc(ch, _file);
	
	if (token.type == INTEGER) {
		token.value.int_v = (int)strtod(num_str, &end_ptr);	
	} else {
		token.value.double_v = strtod(num_str, &end_ptr);
	}
	
	return token;
}

// main
int main(int argc, char *argv[]) {
	// handle args errors
	if (argc < 2) {
		throw_err("No Args: required file path.");
	} else if (argc > 2) {
		throw_err("Too Many Args: the only argument should be the file path.");
	}
	
	// open file and handle file not found
    _file = fopen(argv[1], "r");
    
    if (_file == NULL) {
        throw_err("File Not Found: the path does not lead to any file");
    }
    
    // loop through file
    struct Token tokens[256];
    int i_tokens = 0;
    
    int ch;
    
    while ((ch = fgetc(_file)) != EOF) {
    	struct Token token;
    	switch(ch) {
    		case ' ':
    		case '\t'...'\r': // horizontal tab (\t), newline (\n), vertical tab (\v), formfeed (\f), carriage line (\r)
    			break;
			case '0'...'9':
				tokens[i_tokens++] = get_num(ch);
				break;
			case '+':
				token.type = ADD;
				token.value.char_v = '+';
				tokens[i_tokens++] = token;
				break;
			case '-':
				token.type = SUB;
				token.value.char_v = '-';
				tokens[i_tokens++] = token;
				break;
			case '*':
				token.type = MUL;
				token.value.char_v = '*';
				tokens[i_tokens++] = token;
				break;
			case '/':
				token.type = DIV;
				token.value.char_v = '/';
				tokens[i_tokens++] = token;
				break;
			case '(':
				token.type = LPAR;
				token.value.char_v = '(';
				tokens[i_tokens++] = token;
				break;
			case ')':
				token.type = RPAR;
				token.value.char_v = ')';
				tokens[i_tokens++] = token;
				break;
			case ';':;
				double res = solve_expr(tokens, i_tokens);
				printf("res: %f\n", res);
				i_tokens = 0;
				break;
			default:
				throw_err("Token Not Recognized: token was not recognized");
		}	
	}
	
	return 0;
}
