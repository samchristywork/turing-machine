#include <cjson/cJSON.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"

int verbose = 0;

enum direction {
  NOP = 0,
  L = -1,
  R = 1
};

typedef struct state {
  char state[16];
  char tape_symbol;
  char write_symbol;
  int direction;
  char next_state[16];
} state;

typedef struct tape_t {
  char *data;
  size_t len;
} tape_t;

state *read_json(char *filename, int *len, int *max_iterations, int *start_offset, char *start_state, tape_t *tape) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);

  char *buffer = malloc(size + 1);
  if (!buffer) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  buffer[size] = 0;
  int ret = fread(buffer, 1, size, f);
  if (ret != size) {
    fprintf(stderr, "Could not read the expected number of bytes.\n");
    free(buffer);
    exit(EXIT_FAILURE);
  }

  fclose(f);

  cJSON *cjson = cJSON_Parse(buffer);
  free(buffer);
  if (!cjson) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    exit(EXIT_FAILURE);
  }

  const cJSON *initial_tape = cJSON_GetObjectItemCaseSensitive(cjson, "initial_tape");
  if (initial_tape && cJSON_IsString(initial_tape)) {
    size_t new_len = strlen(initial_tape->valuestring);
    if (new_len > tape->len) {
      tape->data = realloc(tape->data, new_len + 1);
      if (!tape->data) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
      tape->len = new_len;
    }
    memset(tape->data, ' ', tape->len);
    tape->data[tape->len] = '\0';
    for (int i = 0; i < new_len; i++) {
      tape->data[i] = initial_tape->valuestring[i];
    }
    if (verbose) {
      fprintf(stderr, "Loaded initial tape of length %zu\n", new_len);
    }
  }

  const cJSON *mi = cJSON_GetObjectItemCaseSensitive(cjson, "max_iterations");
  if (mi && cJSON_IsNumber(mi)) {
    (*max_iterations) = mi->valueint;
    if (verbose) {
      fprintf(stderr, "Set max iterations to %d\n", *max_iterations);
    }
  }

  const cJSON *so = cJSON_GetObjectItemCaseSensitive(cjson, "start_offset");
  if (so && cJSON_IsNumber(so)) {
    (*start_offset) = so->valueint;
    if (verbose) {
      fprintf(stderr, "Set start offset to %d\n", *start_offset);
    }
  }

  const cJSON *ss = cJSON_GetObjectItemCaseSensitive(cjson, "start_state");
  if (ss && cJSON_IsString(ss)) {
    if (strlen(ss->valuestring) < 16) {
      strcpy(start_state, ss->valuestring);
      if (verbose) {
        fprintf(stderr, "Set start state to \"%s\"\n", start_state);
      }
    } else {
      fprintf(stderr, "Start state name must have fewer than 16 characters.\n");
      exit(EXIT_FAILURE);
    }
  }

  state *s;
  const cJSON *states = cJSON_GetObjectItemCaseSensitive(cjson, "states");
  if (states && cJSON_IsArray(states)) {

    (*len) = cJSON_GetArraySize(states);
    s = malloc(cJSON_GetArraySize(states) * sizeof(state));
    for (int i = 0; i < cJSON_GetArraySize(states); i++) {

      cJSON *item = cJSON_GetArrayItem(states, i);
      if (item) {

        cJSON *state = cJSON_GetObjectItemCaseSensitive(item, "state");
        cJSON *tape_symbol = cJSON_GetObjectItemCaseSensitive(item, "tape_symbol");
        cJSON *write_symbol = cJSON_GetObjectItemCaseSensitive(item, "write_symbol");
        cJSON *direction = cJSON_GetObjectItemCaseSensitive(item, "direction");
        cJSON *next_state = cJSON_GetObjectItemCaseSensitive(item, "next_state");

        if (state && tape_symbol && write_symbol && direction && next_state) {
          if (strlen(state->valuestring) < 16 && strlen(next_state->valuestring) < 16) {

            strcpy(s[i].state, state->valuestring);
            s[i].tape_symbol = tape_symbol->valuestring[0];
            s[i].write_symbol = write_symbol->valuestring[0];

            switch (direction->valuestring[0]) {
            case 'R':
              s[i].direction = R;
              break;
            case 'L':
              s[i].direction = L;
              break;
            case 'N':
              s[i].direction = NOP;
              break;
            default:
              fprintf(stderr, "Invalid direction: \"%c\".\n", direction->valuestring[0]);
              exit(EXIT_FAILURE);
            }

            strcpy(s[i].next_state, next_state->valuestring);
          } else {
            fprintf(stderr, "State names must have fewer than 16 characters.\n");
            exit(EXIT_FAILURE);
          }
        } else {
          fprintf(stderr, "Not all required fields have been specified.\n");
          exit(EXIT_FAILURE);
        }
      } else {
        fprintf(stderr, "State could not be retrieved.\n");
        exit(EXIT_FAILURE);
      }
    }
  } else {
    printf("\"states\" could not be found.\n");
    exit(EXIT_FAILURE);
  }
  cJSON_Delete(cjson);
  return s;
}

void print_version() {
  printf("%s\n\n%s\n", VERSION_STRING, LICENSE_STRING);
}

void usage(char *argv[]) {
  fprintf(stderr,
          "Usage: %s [file...]\n"
          " -g,--graph     Create a graphviz diagram of the input state machine.\n"
          " -h,--help      Print this usage message.\n"
          " -v,--verbose   Display additional logging information.\n"
          " -V,--version   Display the software version and exit.\n"
          "",
          argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

  int graph = 0;

  int opt;
  int option_index = 0;
  char *optstring = "ghvV";
  static struct option long_options[] = {
      {"graph", no_argument, 0, 'g'},
      {"help", no_argument, 0, 'h'},
      {"verbose", no_argument, 0, 'v'},
      {"version", no_argument, 0, 'V'},
      {0, 0, 0, 0},
  };
  while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
    if (opt == 'g') {
      graph = 1;
    } else if (opt == 'h') {
      usage(argv);
    } else if (opt == 'v') {
      verbose = 1;
    } else if (opt == 'V') {
      print_version();
      exit(EXIT_SUCCESS);
    } else if (opt == '?') {
      usage(argv);
    } else {
      puts(optarg);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "No state machine selected.\n");
    usage(argv);
  }

  for (int arg_idx = optind; arg_idx < argc; arg_idx++) {
    tape_t tape;
    tape.len = 80;
    tape.data = malloc(tape.len + 1);
    if (!tape.data) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    memset(tape.data, ' ', tape.len);
    tape.data[tape.len] = '\0';

    state *state_machine = NULL;
    int state_machine_len = 0;
    int max_iterations = 0;
    int start_offset = 0;
    char start_state[16];
    strcpy(start_state, "A");

    state_machine = read_json(argv[arg_idx], &state_machine_len, &max_iterations, &start_offset, start_state, &tape);

    if (!state_machine) {
      fprintf(stderr, "Could not load state machine from %s.\n", argv[arg_idx]);
      free(tape.data);
      continue;
    }

    if (graph) {
      fprintf(stdout, "strict digraph {\n");
      fprintf(stdout, "  rankdir=LR;\n");
      fprintf(stdout, "  node [shape=record];\n");

      // Print nodes with dynamic record labels
      for (int i = 0; i < state_machine_len; i++) {
        // Only process each unique state once
        int already_processed = 0;
        for (int j = 0; j < i; j++) {
          if (strcmp(state_machine[i].state, state_machine[j].state) == 0) {
            already_processed = 1;
            break;
          }
        }
        if (already_processed) {
          continue;
        }

        fprintf(stdout, "  %s [label=\"%s|{", state_machine[i].state, state_machine[i].state);

        // Collect unique symbols for this state
        char symbols[256];
        int symbol_count = 0;
        for (int j = 0; j < state_machine_len; j++) {
          if (strcmp(state_machine[i].state, state_machine[j].state) == 0) {
            char s = state_machine[j].tape_symbol;
            int exists = 0;
            for (int k = 0; k < symbol_count; k++) {
              if (symbols[k] == s) {
                exists = 1;
                break;
              }
            }
            if (!exists) {
              symbols[symbol_count++] = s;
            }
          }
        }

        for (int j = 0; j < symbol_count; j++) {
          char s = symbols[j];
          if (j > 0) {
            fprintf(stdout, "|");
          }
          if (s == ' ') {
            fprintf(stdout, "<space>blank");
          } else {
            fprintf(stdout, "<%c>%c", s, s);
          }
        }
        fprintf(stdout, "}\"];\n");
      }

      // Print edges
      for (int i = 0; i < state_machine_len; i++) {
        char port[16];
        if (state_machine[i].tape_symbol == ' ') {
          strcpy(port, "space");
        } else {
          sprintf(port, "%c", state_machine[i].tape_symbol);
        }

        char dir_char = 'N';
        if (state_machine[i].direction == R) {
          dir_char = 'R';
        } else if (state_machine[i].direction == L) {
          dir_char = 'L';
        }

        fprintf(stdout, "  %s:%s -> %s [label=\"%c, %c\"];\n",
                state_machine[i].state,
                port,
                state_machine[i].next_state,
                dir_char,
                state_machine[i].write_symbol);
      }
      fprintf(stdout, "}\n");
      free(tape.data);
      free(state_machine);
      continue;
    }

    int head = start_offset;
    int sequence = 0;
    char instruction[16];
    strcpy(instruction, start_state);

    for (sequence = 0; strcmp(instruction, "HALT") != 0 && (sequence < max_iterations || max_iterations == 0); sequence++) {
      printf("|");
      for (int i = 0; i < tape.len; i++) {
        if (i == head) {
          printf("\x1b[7m%c\x1b[0m", tape.data[i]);
        } else {
          putchar(tape.data[i]);
        }
      }
      printf("| %s\n", instruction);

      int found_state = 0;
      for (int i = 0; i < state_machine_len; i++) {
        if (strcmp(instruction, state_machine[i].state) == 0) {
          if (tape.data[head] == state_machine[i].tape_symbol) {
            found_state = 1;
            tape.data[head] = state_machine[i].write_symbol;
            head += state_machine[i].direction;
            strcpy(instruction, state_machine[i].next_state);
            break;
          }
        }
      }
      if (!found_state) {
        fprintf(stderr, "Error: No transition found for state \"%s\" and symbol \"%c\".\n", instruction, tape.data[head]);
        break;
      }

      if (head < 0) {
        int increment = 80;
        if (verbose) {
          fprintf(stderr, "Expanding tape left by %d characters\n", increment);
        }
        tape.data = realloc(tape.data, tape.len + increment + 1);
        if (!tape.data) {
          perror("realloc");
          exit(EXIT_FAILURE);
        }
        memmove(tape.data + increment, tape.data, tape.len + 1);
        memset(tape.data, ' ', increment);
        head += increment;
        tape.len += increment;
      } else if (head >= tape.len) {
        int increment = 80;
        if (verbose) {
          fprintf(stderr, "Expanding tape right by %d characters\n", increment);
        }
        tape.data = realloc(tape.data, tape.len + increment + 1);
        if (!tape.data) {
          perror("realloc");
          exit(EXIT_FAILURE);
        }
        memset(tape.data + tape.len, ' ', increment);
        tape.len += increment;
        tape.data[tape.len] = '\0';
      }
    }

    free(tape.data);
    free(state_machine);
  }

  return EXIT_SUCCESS;
}
