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

  int sequence = 0;
  char instruction = A;
  char tape[80];
  memset(tape, ' ', 80);
  tape[80] = 0;
  int head = 40;

  for (sequence = 0; instruction != HALT; sequence++) {
    printf("|%s|\n", tape);

    switch (instruction) {
    case A:
      if (tape[head] == ' ') {
        tape[head] = '1';
        head++;
        instruction = B;
      } else {
        tape[head] = '1';
        head--;
        instruction = C;
      }
      break;
    case B:
      if (tape[head] == ' ') {
        tape[head] = '1';
        head--;
        instruction = A;
      } else {
        tape[head] = '1';
        head++;
        instruction = B;
      }
      break;
    case C:
      if (tape[head] == ' ') {
        tape[head] = '1';
        head--;
        instruction = B;
      } else {
        tape[head] = '1';
        head++;
        instruction = HALT;
      }
      break;
    case HALT:
      break;
    default:
      fprintf(stderr, "Something has gone wrong.\n");
    }
  }
}
