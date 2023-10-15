#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <unistr.h>

typedef struct Squawk {
	FILE*    inputs[INPUT_MAX];
	FILE*	 output;
	size_t	 input_idx;
	uint8_t* record;
	size_t   record_len;
} squawk_t;

static squawk_t *sq_state;

#define INPUT_IDX 	sq_state->inputs[sq_state->input_idx]
#define OUTPUT		sq_state->output
#define RECORD		sq_state->record
#define STATE		sq_state

#define GET_FILE_SIZE(file, size)		\
	do {					\
		fseek(file, 1, SEEK_END);	\
		size = ftell(file);		\
		rewind(file);			\
	} while (0)

void do_on_exit(void) {
	free(INPUT_IDX);
	free(RECORD);
	free(STATE);
	fclose(OUTPUT);
}

void do_on_sigint(int signum) {
	if (signum == SIGINT)
		do_on_exit();
}

void initialize_squawk(int argc, uint8_t *argv[argc + 1]) {
	const struct sigaction sigint_action = {
		.sa_handler = do_on_sigint,
	};
	atexit(do_on_exit);
	if (sigaction(SIGINT, &sigint_action, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	sq_state = (squawk_t*)calloc(1, sizeof(squawk_t));
	OUTPUT = stdout;
}

int execute_and_rw(uint8_t const*const command, 
				const char *mode,
				uint8_t **result_ptr, 
				size_t *result_len,
				bool close_after) {
	fflush(stdin); fflush(stdout); fflush(stderr);
	FILE *pipe = popen(command, mode);
	fflush(stdin); fflush(stdout); fflush(stderr);
	int read_result = getline((char**)result_ptr, result_len, pipe);
	if (close_after) pclose(pipe);
	return read_result;
}

int main(int argc, uint8_t *argv[argc + 1]) {
	initialize_squawk(argc, argv);
	uint8_t *result_ptr;
	size_t result_len;
	execute_and_rw(argv[1], "r", &result_ptr, &result_len, true);
	fputs(result_ptr, OUTPUT);
	free(result_ptr);
}
