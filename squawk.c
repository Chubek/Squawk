#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <unistr.h>
#include <pcre2posix.h>
#include <gc.h>

#define INNER_HASH 16

typedef struct Symbol {
	uintptr_t value;
	bool      defined;
	enum Symtype { 
		PIPE, STREAM, STRING, 
		INT, FLOAT, FUNCTION, 
		DEFAULT,
	}	type;
} symbol_t;

typedef struct Squawk {
	FILE**    	inputs;
	FILE*		input;
	FILE*	 	output;
	size_t	 	input_idx;
	uint8_t* 	record;
	size_t      	record_len;
	uint8_t** 	argv;
	int		argc;
	symbol_t*	symtbl;
	size_t		symtbl_len;
} squawk_t;

static squawk_t* 	sq_state;

#define INPUTS	 	sq_state->inputs
#define INPUT		sq_state->input
#define OUTPUT		sq_state->output
#define RECORD		sq_state->record
#define SYMTBL		sq_state->symtbl
#define SYMTBL_LEN	sq_state->symtbl_len
#define STATE		sq_state


void do_on_exit(void) {
	fclose(OUTPUT);
}

void do_on_sigint(int signum) {
	if (signum == SIGINT)
		do_on_exit();
}

static void initialize_squawk(int argc, char **argv) {
	GC_INIT();
	const struct sigaction sigint_action = {
		.sa_handler = do_on_sigint,
	};
	if (sigaction(SIGINT, &sigint_action, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	atexit(do_on_exit);

	STATE  = (squawk_t*)GC_MALLOC(sizeof(squawk_t));
	SYMTBL = 
	    (symbol_t*)GC_MALLOC(32 * sizeof(symbol_t));
	OUTPUT = stdout;
}


static inline uint16_t djb2_hash(uint8_t *id) {
	uint8_t ch;
	uint32_t hash = 0;
	while ((ch = *id++))
		hash = ((hash << 5) + hash) + ch;
	return hash;
}

static inline uint8_t* gc_concat_str(uint8_t *string_1, 
					uint8_t *string_2,
					size_t len_string_1,
					size_t  len_string_2) {
	static uint8_t null_term[2] = {0};
	uint8_t* concat_str = 
		 (uint8_t*)GC_MALLOC(len_string_1 + len_string_2 + 2);
	u8_strncat(concat_str, string_1, len_string_1);
	u8_strncat(concat_str, string_2, len_string_2);
	u8_strncat(concat_str, &null_term[0], 2);
	return concat_str;

}

static symbol_t* symtable_upsert(uint8_t *id, 
				uintptr_t value, 
				enum Symtype type) {
	uint16_t hash = djb2_hash(id);
	hash *= hash % INNER_HASH;
	if (hash > SYMTBL_LEN) {
		SYMTBL_LEN = hash;
		SYMTBL = 
                  (symbol_t*)GC_REALLOC(SYMTBL, SYMTBL_LEN * sizeof(symbol_t));
	}
	if (SYMTBL[hash - 1].defined == true)
		return &SYMTBL[hash - 1];
	SYMTBL[hash - 1].defined = true;
	SYMTBL[hash - 1].value   = value;
	SYMTBL[hash - 1].type    = type;
	return &SYMTBL[hash - 1];
}

int execute_and_rw(uint8_t* id_stream, uint8_t *id_var, 
				const uint8_t* command, 
				const char *mode,
				uint8_t **result_ptr, 
				size_t *result_len,
				bool close_after) {
	fflush(stdin); fflush(stdout); fflush(stderr);
 	FILE *pipe = popen((char*)command, mode);
	fflush(stdin); fflush(stdout); fflush(stderr);
	int read_result = getline((char**)result_ptr, result_len, pipe);
	if (id_stream)
		symtable_upsert(id_stream, (uintptr_t)pipe, PIPE);
	if (id_var)
		symtable_upsert(id_var, (uintptr_t)result_ptr, STRING);
	if (close_after) pclose(pipe);
	return read_result;
}


int main(int argc, char **argv) {
	initialize_squawk(argc, argv);
	printf("4444");
	uint8_t *result_ptr;
	size_t result_len;
	uint8_t *array_id = gc_concat_str((uint8_t*)"myarr", (uint8_t*)"2, 2, 2", 5, 7);
	symbol_t *sym = symtable_upsert(array_id, 222, INT);
	printf("%p", sym);
}
