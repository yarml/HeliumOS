#include <error.h>
#include <stdio.h>
#include <kmod.h>
#include <term.h>

#include <fs/tar.h>

static void prompt(char *user, char *pwd, int super)
{
  term_setfg(255, 255, 0);
  fputs(user, stdout);
  term_setfg(255, 0, 0);
  printf(" [");
  term_setfg(0, 255, 255);
  fputs(pwd, stdout);
  term_setfg(255, 0, 0);
  printf("] ");
  term_setfg(255, 255, 0);
  printf(super ? "# " : "$ ");
  term_setfg(255, 255, 255);
}

int kmain()
{
  term_setfg(255, 128, 0);
  printf(
    "##   ## ####### ##      ## ##    ## ###    ###  ######  #######\n"
    "##   ## ##      ##      ## ##    ## ####  #### ##    ## ##     \n"
    "####### #####   ##      ## ##    ## ## #### ## ##    ## #######\n"
    "##   ## ##      ##      ## ##    ## ##  ##  ## ##    ##      ##\n"
    "##   ## ####### ####### ##  ######  ##      ##  ######  #######\n"
    "\n"
  );
  prompt("kernel", "initrd://sys/", 1);
  printf("\n");

  if(ksym_loadf("initrd://sys/ksym"))
    error_inv_state("Could not load kernel symbols.");

  kmod_loadf("initrd://modules/test1.mod");
  kmod_loadf("initrd://modules/test2.mod");

  return 0;
}
