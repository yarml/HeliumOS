#include <stdbool.h>
#include <syscall.h>

static void prompt(char const *user, char const *pwd, bool admin) {
  syscall_print(user);
  syscall_print(" [");
  syscall_print(pwd);
  syscall_print("] ");
  syscall_print(admin ? "#" : "$");
  syscall_print(" ");
}

void memset(void *buf, int c, size_t len) {
  for (; len; --len) {
    ((char *)buf)[len - 1] = c;
  }
}

int strcmp(char *s1, char *s2) {
  while (*s1 && *s2 && *s1 == *s2) {
    ++s1;
    ++s2;
  }
  return *s2 - *s1;
}

bool isspace(char c) {
  return c == ' ';
}

void readline(char *buf, size_t len) {
  char rdbuf[1] = {0};
  memset(buf, 0, len);

  size_t read = 0;
  while (read < len - 1) {
    size_t itread = syscall_gets(rdbuf, 1);
    if (!itread) {
      continue;
    }
    if (*rdbuf == '\n') {
      return;
    }
    buf[read] = *rdbuf;
    ++read;
  }
}

int main() {
  syscall_clear();
  syscall_dbg("init\n");
  char buf[256];
  while (1) {
    prompt("admin", "initrd:/", true);
    memset(buf, 0, 256);
    readline(buf, 256);
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
      syscall_clear();
    } else if (!strcmp(cmd, "version")) {
      syscall_print("HeliumOS v0.1\n");
    } else if (!strcmp(cmd, "exit")) {
      syscall_exit(0);
    } else {
      syscall_print("Unknown command '");
      syscall_print(cmd);
      syscall_print("'\n");
    }
  }
}