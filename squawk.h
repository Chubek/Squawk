#ifndef SQUAWK_H_
#define SQUAWK_H_


#include <stdint.h>

typedef struct Symbol 		 symbol_t;
typedef struct Squawk		 squawk_t;
typedef struct Slhtbl		 slhtbl_t;
typedef enum   DefaultVar	 dflvar_t;
typedef enum   Symtype		 symtype_t;

int execute_and_rw(uint8_t* id_stream, uint8_t *id_var, 
				const uint8_t* command, 
				const char *mode,
				uint8_t **result_ptr, 
				size_t *result_len,
				bool close_after);

static inline void sym_init(void);

static void sym_put(uint8_t *id, 
			uintptr_t value, 
			symtype_t type);

static symtype_t sym_get(uint8_t* id, uintptr_t *value);

static void sym_remove(uint8_t* id);

static inline void sym_resize(void);

static inline uint8_t* gc_concat_str(uint8_t *string_1, 
					uint8_t *string_2,
					size_t len_string_1,
					size_t  len_string_2);


static void initialize_squawk(int argc, char **argv);

static void upsert_default_vars(void);

void do_on_sigint(int signum);

void do_on_exit(void);




#endif
