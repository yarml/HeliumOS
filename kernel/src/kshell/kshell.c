#include <ctype.h>
#include <kterm.h>
#include <stdio.h>
#include <string.h>
#include <userspace.h>

static void prompt(char *user, char *pwd, int super) {
  kterm_setfg(255, 255, 0);
  puts(user);
  kterm_setfg(255, 0, 0);
  printf(" [");
  kterm_setfg(0, 255, 255);
  puts(pwd);
  kterm_setfg(255, 0, 0);
  printf("] ");
  kterm_setfg(255, 255, 0);
  printf(super ? "# " : "$ ");
  kterm_setfg(255, 255, 255);
}

static void banner() {
  kterm_setfg(255, 128, 0);
  printf(
      "##   ## ####### ##      ## ##    ## ###    ###  ######  #######\n"
      "##   ## ##      ##      ## ##    ## ####  #### ##    ## ##     \n"
      "####### #####   ##      ## ##    ## ## #### ## ##    ## #######\n"
      "##   ## ##      ##      ## ##    ## ##  ##  ## ##    ##      ##\n"
      "##   ## ####### ####### ##  ######  ##      ##  ######  #######\n"
      "\n"
  );
}

void kshell_run() {
  kterm_clear();
  banner();

  char buf[256];
  while (1) {
    prompt("admin", "initrd:/", true);
    memset(buf, 0, 256);
    kterm_fgets(buf, 256);
    char *cursor = buf;

    while (*cursor && isspace(*cursor)) {
      ++cursor;
    }

    char  *cmd    = cursor;
    size_t cmdlen = 0;

    while (*cursor && !isspace(*cursor)) {
      ++cmdlen;
      cursor++;
    }

    if (!strcmp(cmd, "clear")) {
      kterm_clear();
    } else if (!strcmp(cmd, "version")) {
      puts("HeliumOS v0.1\n");
    } else if (!strcmp(cmd, "exec")) {
      int result = exec();
      kterm_setfg(0, 255, 0);
      printf("Process exited with result code: %d\n", result);
    } else {
      kterm_setfg(255, 0, 0);
      printf("Unknown command '%.*s'.\n", cmdlen, cmd);
    }
  }
}