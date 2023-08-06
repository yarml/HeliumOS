#include <error.h>
#include <stdio.h>
#include <string.h>
#include <sys.h>

noreturn void error_general(char *prefix, char *details) {
  fprintf(stddbg, "[%s] %s\n", prefix, details);
  stop();
}

noreturn void error_out_of_memory(char *details) {
  error_general("Out of memory", details);
}

noreturn void error_inv_state(char *details) {
  error_general("Invalid state", details);
}

noreturn void error_feature(char *feature) {
  error_general("Feature Unsupported", feature);
}
