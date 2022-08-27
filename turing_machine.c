#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum states {
  A,
  B,
  C,
  HALT
};

enum direction {
  NOP = 0,
  L = -1,
  R = 1
};

typedef struct state {
  int state;
  char tape_symbol;
  char write_symbol;
  int direction;
  int next_state;
} state;

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

  if (optind < argc) {
    int i = optind;
    while (i < argc) {
      fprintf(stdout, "Got additional argument: %s\n", argv[i]);
      i++;
    }
  }

  state state_machine[] = {
      {.state = A, .tape_symbol = ' ', .write_symbol = '1', .direction = R, .next_state = B},
      {.state = A, .tape_symbol = '1', .write_symbol = '1', .direction = L, .next_state = C},
      {.state = B, .tape_symbol = ' ', .write_symbol = '1', .direction = L, .next_state = A},
      {.state = B, .tape_symbol = '1', .write_symbol = '1', .direction = R, .next_state = B},
      {.state = C, .tape_symbol = ' ', .write_symbol = '1', .direction = L, .next_state = B},
      {.state = C, .tape_symbol = '1', .write_symbol = '1', .direction = R, .next_state = HALT}};


  int sequence = 0;
  char instruction = A;
  char tape[80];
  memset(tape, ' ', 80);
  tape[80] = 0;
  int head = 40;

  for (sequence = 0; instruction != HALT; sequence++) {
    char tape_string[79];
    memcpy(tape_string, tape, 79);
    tape_string[head] = 'h';
    printf("|%s|\n", tape_string);

    for (int i = 0; i < 6; i++) {
      if (instruction == state_machine[i].state) {
        if (tape[head] == state_machine[i].tape_symbol) {
          tape[head] = state_machine[i].write_symbol;
          head += state_machine[i].direction;
          instruction = state_machine[i].next_state;
          break;
        }
      }
    }
  }
}
