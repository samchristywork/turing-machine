#include <cjson/cJSON.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

state *read_json(char *filename, int *len) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);

  char buffer[size + 1];
  buffer[size] = 0;
  int ret = fread(buffer, 1, size, f);
  if (ret != size) {
    fprintf(stderr, "Could not read the expected number of bytes.\n");
    exit(EXIT_FAILURE);
  }

  fclose(f);

  cJSON *cjson = cJSON_Parse(buffer);
  if (!cjson) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    cJSON_Delete(cjson);
    exit(EXIT_FAILURE);
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
              fprintf(stderr, "Something went wrong.\n");
            }

            strcpy(s[i].next_state, next_state->valuestring);
          } else {
            fprintf(stderr, "Something went wrong.\n");
          }
        } else {
          fprintf(stderr, "Something went wrong.\n");
        }
      } else {
        fprintf(stderr, "Something went wrong.\n");
      }
    }
  } else {
    printf("\"states\" could not be found.\n");
  }
  cJSON_Delete(cjson);
  return s;
}

void usage(char *argv[]) {
  fprintf(stderr,
          "Usage: %s [file]\n"
          " -h,--help      Print this usage message.\n"
          " -v,--verbose   Display additional logging information.\n"
          "",
          argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

  int verbose = 0;

  int opt;
  int option_index = 0;
  char *optstring = "hv";
  static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"verbose", no_argument, 0, 'v'},
      {0, 0, 0, 0},
  };
  while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
    if (opt == 'h') {
      usage(argv);
    } else if (opt == 'v') {
      verbose = 1;
    } else if (opt == '?') {
      usage(argv);
    } else {
      puts(optarg);
    }
  }

  state *state_machine;
  int state_machine_len = 0;
  if (optind < argc) {
    int i = optind;
    while (i < argc) {
      fprintf(stdout, "Got additional argument: %s\n", argv[i]);
      state_machine = read_json(argv[i], &state_machine_len);
      i++;
    }
  }

  int sequence = 0;
  char instruction[16];
  strcpy(instruction, "A");
  char tape[80];
  memset(tape, ' ', 80);
  tape[79] = 0;
  int head = 40;
  int max_ops = 15;

  for (sequence = 0; strcmp(instruction, "HALT") != 0 && sequence < max_ops; sequence++) {
    char tape_string[79];
    memcpy(tape_string, tape, 79);
    tape_string[head] = 'h';
    printf("|%s|\n", tape_string);

    int found_state = 0;
    for (int i = 0; i < state_machine_len; i++) {
      if (strcmp(instruction, state_machine[i].state) == 0) {
        if (tape[head] == state_machine[i].tape_symbol) {
          found_state = 1;
          tape[head] = state_machine[i].write_symbol;
          head += state_machine[i].direction;
          strcpy(instruction, state_machine[i].next_state);
          break;
        }
      }
    }
    if (!found_state) {
      fprintf(stderr, "Error\n");
      break;
    }
    if (head > 78) {
      head = 0;
    }
    if (head < 0) {
      head = 78;
    }
  }
}
