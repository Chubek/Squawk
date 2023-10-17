#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sysexits.h>

#include <unistr.h>
#include <pcre2posix.h>
#include <gc.h>

#include "squawk.h"

#define INNER_HASH 		121
#define SYMTBL_LEN_STEP		64

typedef enum Symtype { 
		STRING, INT, FLOAT, FUNCTION, 
		NONE, IORES, PIPE_STREAM, IO_STREAM,
} symtype_t;

typedef enum BlockState {
	BEGIN_BLK, END_BLK, EXPR_BLK, PATT_BLK, NOEXPR_BLK,
} blstat_t;

typedef enum IoStat {
	READSREC = 1,
	GETSLINE = 2,
	REGEX	 = 4,
	EXPR	 = 8,
	PRINTS	 = 16,
	READS	 = 32,
	WRITES	 = 64,
	APPENDS  = 128,
	HASVAR	 = 256,
	PIPES	 = 512,
	CLOSES	 = 1024,
} iostat_t;

typedef struct Symbol {
	uint64_t 	key;
	uintptr_t	value;
	symtype_t	type;
	symbol_t*	next;
} symbol_t;

typedef struct Squawk {
	FILE**    	inputs;
	FILE*		input;
	FILE*	 	output;
	size_t	 	input_idx;
	uint8_t* 	record;
	size_t      	record_len;
	size_t		record_num;
	uint8_t**	fields;
	size_t		fields_num;
	regex_t		regex_cc;
	uint8_t*	pattern;
	uint8_t*	regex_in;
	size_t		nmatch;
	regmatch_t	pmatch[1];
	regoff_t	offset_init;
	regoff_t	offset_len;
	int		argc_int;
	uint8_t** 	argv;
	uint8_t*	argc;
	uint8_t*	convfmt;
	uint8_t**	environ;
	uint8_t*	filename;
	uint8_t*	fnr;
	uint8_t*	fs;
	uint8_t*	nf;
	uint8_t*	nr;
	uint8_t*	ofmt;
	uint8_t*	ofs;
	uint8_t*	ors;
	uint8_t*	rlength;
	uint8_t*	rs;
	uint8_t*	rstart;
	uint8_t*	subsep;
	symbol_t**	symtbl;
	size_t		symtbl_len;
	size_t		symtbl_cnt;
	blstat_t	block_stat;
	bool		block_range;
	bool 		action_getline;
	iostat_t	iostat;
	uint8_t*	iotext;
	uint8_t*	ioprompt;
	uint8_t**	ioresult;
	size_t		iotextlen;
	size_t		iopromptlen;
	size_t*		ioresultlen;
	uint8_t*	iofileid;
	uint8_t*	iovarid;
	FILE*		iostream;
	int 		ioint;
	bool		ioispipe;
} squawk_t;

typedef enum DefaultVar { 
		Argv, Argc, Convfmt, 
		Environ, Filename, Fnr, 
		Fs, Nf, Nr, 
		Ofmt, Ofs, Ors, 
		Rlength, Rs, Rstart, Subsep, 
} dflvar_t;

static squawk_t* 	sq_state;

#define STATE		sq_state
#define INPUTS	 	sq_state->inputs
#define INPUT		sq_state->input
#define INPUT_IDX	sq_state->input_idx
#define OUTPUT		sq_state->output
#define RECORD		sq_state->record
#define RECORD_NUM	sq_state->record_num
#define SYMTBL		sq_state->symtbl
#define SYMTBL_LEN	sq_state->symtbl_len
#define SYMTBL_CNT	sq_state->symtbl_cnt
#define FIELDS		sq_state->fields
#define FIELDS_NUM	sq_state->fields_num
#define PMATCH		sq_state->pmatch
#define NMATCH		sq_state->nmatch
#define REGEX_CC	sq_state->regex_cc
#define OFSINIT  	sq_state->offset_init
#define OFSLEN		sq_state->offset_len
#define PATTERN		sq_state->pattern
#define BLKSTAT		sq_state->block_stat
#define ACTGETLN	sq_state->action_getline
#define BLKRANGE	sq_state->block_range
#define REGEX_IN	sq_state->regex_in
#define ARGC_INT	sq_state->argc_int
#define ARGV		sq_state->argv
#define ARGC		sq_state->argc
#define CONVFMT		sq_state->convfmt
#define ENVIRON		sq_state->environ
#define FILENAME	sq_state->filename
#define FNR		sq_state->fnr
#define FS		sq_state->fs
#define NF		sq_state->nf
#define NR		sq_state->nr
#define OFMT		sq_state->ofmt
#define OFS		sq_state->ofs
#define ORS		sq_state->ors
#define RLENGTH		sq_state->rlength
#define RS		sq_state->rs
#define RSTART		sq_state->rstart
#define SUBSEP		sq_state->subsep
#define IOSTAT		sq_state->iostat
#define IOTEXT		sq_state->iotext
#define IORESULT	sq_state->ioresult
#define IORESLEN	sq_state->ioresultlen
#define IOTXTLEN	sq_state->iotextlen
#define IOPROMPT	sq_state->ioprompt
#define IOPROMPTLEN	sq_state->iopromptlen
#define IOFILEID	sq_state->iofileid
#define IOVARID		sq_state->iovarid
#define IOSTREAM	sq_state->iostream
#define IOINT		sq_state->ioint
#define IOISPIPE	sq_state->ioispipe

void do_on_exit(void) {
	fclose(OUTPUT);
}

void do_on_sigint(int signum) {
	if (signum == SIGINT)
		do_on_exit();
}

static void put_default_vars(void) {

	uint8_t* default_vars[] = {
		 ARGC, 	    "ARGC",
		 CONVFMT,   "CONVFMT",
		 FILENAME,  "FILENAME",
		 FNR, 	    "FNR",
		 FS, 	    "FS",
		 NF, 	    "NF",
		 NR, 	    "NR",
		 OFMT, 	    "OFMT",
		 OFS, 	    "OFS",
		 ORS, 	    "ORS",
		 RLENGTH,   "RLENGTH",
		 RS, 	    "RS",
		 RSTART,    "RSTART",
		 SUBSEP,    "SUBSEP",
	};
	
	uint8_t** default_array[2] = {
		ARGV, ENVIRON,
	};

	static uint8_t*  default_arraynames[2] = {
		"ARGV", "ENVIRON",
	};
	
	static int default_arraysize = 2;
	static int default_size      = 28;
	
	int       n = 0;
	uint8_t*  id;
	char*     alloc;
	uint8_t*  val;
	uint8_t** arr;
	
	for (int i = 0; i < default_arraysize; i++)
	{
		id  = default_arraynames[i];
		arr = &default_array[i][0];
		n   = 0;

		while ((val = *arr++)) {
			int len = asprintf(&alloc, "%d", n++);
			sym_array_put(id, alloc, (uintptr_t)val, STRING);
			free(alloc);
		}
	}
	
	for (int i = 0; i < default_size; i+=2) {
		id   = default_vars[i + 1];
		val  = default_vars[i];

		sym_put(id, (uintptr_t)val, STRING);
	}

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

	extern char **environ;

	STATE		= (squawk_t*)GC_MALLOC(sizeof(squawk_t));
	ARGV   		= (uint8_t**)argv;
	ENVIRON		= (uint8_t**)environ;
	ARGC_INT	= argc;
	NMATCH		= 1;
	OUTPUT 		= stdout;
	sym_init();
	put_default_vars();
}

static inline uint64_t hash64(uint8_t* s)
{
    uint64_t h = 0x100;
    uint8_t  c = 0;
    while ((c = *s++)) {
        h ^= c;
        h *= 1111111111111111111u;
    }
    return h ^ h >>32;
}

static inline uint16_t djb2_hash(uint8_t *id) {
	uint8_t  ch 	= 0;
	uint16_t hash 	= 0;
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

static inline void sym_init(void) {
	SYMTBL_LEN = SYMTBL_LEN_STEP;
	SYMTBL	   = (symbol_t**)GC_MALLOC(SYMTBL_LEN * sizeof(symbol_t*));
}

static void sym_put(uint8_t *id, uintptr_t value, symtype_t type)  {
	uint16_t   bucket = djb2_hash(id) % SYMTBL_LEN;
	symbol_t*  table  = SYMTBL[bucket];

	uint64_t key = hash64(id);
	if (table) {
		for (; table && table->key != key; table = table->next);
		if (table) {
			table->value = value;
			return;
		}
	}

	table 		= (symbol_t*)GC_MALLOC(sizeof(symbol_t));
	table->key 	= key;
	table->value	= value;
	table->type    	= type;
	table->next	= NULL;
	SYMTBL[bucket]	= table;
	SYMTBL_CNT++;

}

static symtype_t sym_get(uint8_t* id, uintptr_t* value) {
	uint16_t  bucket  = djb2_hash(id) % SYMTBL_LEN;
	symbol_t* table   = SYMTBL[bucket];

	uint64_t key = hash64(id);
	if (table) {
		do {
			if (table->key == key) {
				*value = table->value;
				return table->type;
			}
		} while ((table = table->next));
	}
	return NONE;
}

static void  sym_remove(uint8_t *id) {
	uint16_t    bucket = djb2_hash(id) % SYMTBL_LEN;
	
	if (!SYMTBL[bucket])
		return;

	uint64_t  key = hash64(id);
	if (SYMTBL[bucket]->key == key) {
		SYMTBL[bucket] = SYMTBL[bucket]->next;
		SYMTBL_CNT--;
		return;
	}

	symbol_t* prev = SYMTBL[bucket];
	symbol_t* curr = prev->next;

	while (curr != NULL && curr->key != key) {
		curr = curr->next;
		prev = curr;
	}

	if (curr != NULL) {
		symbol_t**  tblptr    = &prev->next;
		*tblptr    	      = curr->next;
		SYMTBL_CNT--;
	}

}

static inline void sym_array_put(uint8_t* id, 
				uint8_t*  index, 
				uintptr_t value,
				symtype_t type) {
	uint8_t* concat = gc_concat_str(id, index,
			u8_strlen(id), u8_strlen(index));
	sym_put(concat, value, type);
}

static inline symtype_t sym_array_get(uint8_t* id, 
		uint8_t*   index,
		uintptr_t* value) {
	uint8_t* concat = gc_concat_str(id, index, 
			u8_strlen(id), u8_strlen(index));
	return sym_get(concat, value);
}

static inline void sym_array_remove(uint8_t* id, uint8_t* index) {
	uint8_t* concat = gc_concat_str(id, index,
			u8_strlen(id), u8_strlen(index));
	sym_remove(concat);
}

static inline void sym_resize(void) {
	if (SYMTBL_CNT >= (SYMTBL_LEN * 0.75)) {
		SYMTBL_LEN += SYMTBL_LEN_STEP;
		SYMTBL      = (symbol_t**)GC_REALLOC(SYMTBL, SYMTBL_LEN * sizeof(symbol_t*));
	}
}


static void compile_re(void) {
	if (pcre2_regcomp(&REGEX_CC, PATTERN, 0) < 0) {
		perror("pcre2_regcomp");
		exit(EX_TEMPFAIL);
	}
}

static void free_re(void) {
	pcre2_regfree(&REGEX_CC);
}

static bool match_re(void) {
	if (!pcre2_regexec(&REGEX_CC, REGEX_IN, NMATCH, PMATCH, 0)) {
		OFSINIT = PMATCH[0].rm_so;
		OFSLEN  = PMATCH[0].rm_eo - OFSINIT;
		return true;
	}
	return false;
}

static void sprint_default_vars(dflvar_t var) {
	size_t val;
	switch (var) {
		case Argc:
			ARGC = NULL;
			ARGC = (uint8_t*)GC_MALLOC(32);
			sprintf(&ARGC[0], "%d", ARGC_INT);
			sym_put("ARGC", (uintptr_t)ARGC, STRING);
			break;
		case Fnr:
			val = (BLKSTAT == BEGIN_BLK) ? 0 : RECORD_NUM;
			FNR = NULL;
			FNR = (uint8_t*)GC_MALLOC(32);
			sprintf(&FNR[0], "%lu", val);
			sym_put("FNR", (uintptr_t)FNR, STRING);
			break;
		case Nf:
			val = (BLKSTAT != BEGIN_BLK && !ACTGETLN)
				? FIELDS_NUM
				: 0;
			NF  = NULL;
			NF  = (uint8_t*)GC_MALLOC(32);
			sprintf(&NF[0], "%lu", val);
			sym_put("NF", (uintptr_t)NF, STRING);
			break;
		case Rlength:
			RLENGTH = NULL;
			RLENGTH = (uint8_t*)GC_MALLOC(32);
			sprintf(&RLENGTH[0], "%u", OFSLEN);
			sym_put("RLENGTH", (uintptr_t)RLENGTH, STRING);
			break;
		case Rstart:
			RSTART = NULL;
			RSTART = (uint8_t*)GC_MALLOC(32);
			sprintf(&RLENGTH[0], "%u", OFSINIT);
			sym_put("RSTART", (uintptr_t)RSTART, STRING);
			break;
		default:
			break;


	}

}

static inline void read_record(void) {
	if (getdelim(&RECORD, &RECORD_LEN, RS, INPUT) < 0)
		wrap_input;
}

static inline void pipe_open(void) {
	if (!(IOSTREAM = popen(IOTEXT, IORWMODE))) {
		perror("popen");
		exit(EX_IOERR);
	}
}

static inline void file_open(void) {
	if (!(IOSTREAM = fopen(IOTEXT, IORWMODE))) {
		perror("fopen");
		exit(EX_IOERR);
	}
}

static inline void stream_read(void) {
	IOINT = getdelim(IORESULT, IORESLEN, RS, IOSTREAM);
	
}

static inline void stream_write(void) {
	IOINT = fputs(IOTEXT, IOSTREAM);
}

static inline void stream_close(void) {
	IOISPIPE ? pclose(IOSTREAM) : fclose(IOSTREAM);
}

static inline void stream_get(void) {
	uintptr_t value 	= 0;
	symtype_t symbol_type 	= sym_get(IOTEXT, &value);
	if (symbol_type != NONE) {
		if (symbol_type == PIPE_STREAM)
			IOSTAT 		|= PIPES;
		IOSTREAM = (FILE*)value;
	}
}

static inline void stream_put(void) {
	sym_put(IOTEXT, (uintptr_t)IOSTREAM, 
			IOISPIPE ? PIPE_STREAM : IO_STREAM);
}

static inline void ioint_get(void) {
	uintptr_t value		= 0;
	symtype_t symbol_type	= sym_get(IOFILEID, &value);
	if (symbol_type == INT)
		IOINT = (int)value;
}

static inline void ioint_put(void) {
	sym_put(IOFILEID, (uintptr_t)IOINT, INT);
}

static inline void iores_get(void) {
	uintptr_t value 	= 0;
	symtype_t symbol_type 	= sym_get(IOVARID, &value);
	if (symbol_type == STRING)
		*IORESULT = (uint8_t*)value;
}

static inline void iores_put(void) {
	sym_put(IOVARID, (uintptr_t)(*IORESULT), STRING);
}

int main(int argc, char **argv) {
	initialize_squawk(argc, argv);

	sprint_default_vars(Argc);
}
