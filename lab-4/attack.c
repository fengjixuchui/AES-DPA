/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "attack.h"

#define BUFFER_SIZE (80)

pid_t pid = 0;           // process ID (of either parent or child) from fork

int target_raw[2];       // unbuffered communication: attacker -> attack target
int attack_raw[2];       // unbuffered communication: attack target -> attacker

FILE* target_out = NULL; // buffered attack target input  stream
FILE* target_in  = NULL; // buffered attack target output stream

void interact(int* t, int* r, const char* G ) {
  // Send G to attack target.
  fprintf(target_in, "%s\n", G); fflush(target_in);

  // Receive (t, r) from attack target.
  if (1 != fscanf(target_out, "%d", t)) {
    abort();
  }
  if (1 != fscanf(target_out, "%d", r)) {
    abort();
  }
}

void attack() {
  // Select a hard-coded guess ...
  char* G = "guess";

  int t;
  int r;

  // ... then interact with the attack target.
  interact(&t, &r, G);

  // Print all of the inputs and outputs.
  printf("G = %s\n", G);
  printf("t = %d\n", t);
  printf("r = %d\n", r);
}

char int_to_char(int n) {
  char c = n + 97;
  return c;
}

void denary_to_char(char* string, int n, int base) {
  int i = 0;

  if (n == 0) string[0] = int_to_char(0);

  while (n > 0) {
    int r = n % base;
    string[i++] = int_to_char(r);
    n = n / base;
  }
}

void brute_force_attack() {
  char G[8] = "";

  int t = 0;
  int r = 0;

  printf("Performing brute force attack...\n");

  // 26^8 = 208827064576.
  for (int i = 0; i < 208827064576; i++) {
    denary_to_char(G, i, 26);
    interact(&t, &r, G);
    if (r == 1) {
      printf("G = %s\n", G);
      printf("t = %d\n", t);
      printf("r = %d\n", r);
      return;
    }
  }
}

void cleanup( int s ){
  // Close the buffered communication handles.
  fclose(target_in);
  fclose(target_out);

  // Close the unbuffered communication handles.
  close(target_raw[0]);
  close(target_raw[1]);
  close(attack_raw[0]);
  close(attack_raw[1]);

  // Forcibly terminate the attack target process.
  if (pid > 0) {
    kill(pid, SIGKILL);
  }

  // Forcibly terminate the attacker process.
  exit(1);
}

int main(int argc, char* argv[]) {
  // Ensure we clean-up correctly if Control-C (or similar) is signalled.
  signal(SIGINT, &cleanup);

  // Create pipes to/from attack target
  if (pipe(target_raw) == -1) {
    abort();
  }
  if (pipe(attack_raw) == -1) {
    abort();
  }

  pid = fork();

  if (pid >  0) { // parent
    // Construct handles to attack target standard input and output.
    if ((target_out = fdopen(attack_raw[0], "r")) == NULL) {
      abort();
    }
    if ((target_in  = fdopen(target_raw[1], "w")) == NULL) {
      abort();
    }

    // Execute a function representing the attacker.
    brute_force_attack();

  } else if (pid == 0) { // child

    // (Re)connect standard input and output to pipes.
    close(STDOUT_FILENO);
    if (dup2(attack_raw[1], STDOUT_FILENO) == -1) {
      abort();
    }
    close(STDIN_FILENO);
    if (dup2(target_raw[0], STDIN_FILENO) == -1) {
      abort();
    }

    // Produce a sub-process representing the attack target.
    execl(argv[1], argv[0], NULL);

  } else if( pid <  0 ) { // error

    // The fork failed; reason is stored in errno, but we'll just abort.
    abort();
  }

  // Clean up any resources we've hung on to.
  cleanup(SIGINT);

  return 0;
}
