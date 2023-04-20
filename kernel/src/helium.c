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


  fsnode *fkmod = fs_search("initrd://modules/test.mod");
  void *kmodf = tarfs_direct_access(fkmod);
  fs_close(fkmod);

  kmod_load(kmodf);

  return 0;
}
