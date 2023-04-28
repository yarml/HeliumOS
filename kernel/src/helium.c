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

  if(ksym_loadp("initrd://sys/ksym"))
    error_inv_state("Could not load kernel symbols.");

  fsnode *moddir = fs_search("initrd://modules/");
  if(!moddir) // No modules to load
    return 0;

  fsnode *modit = fs_nextnode(moddir, 0);
  while(modit)
  {
    fsnode *current = modit;
    modit = fs_nextnode(moddir, modit);
    if(current->type == FSNODE_LINK)
      current = current->link.target;
    if(!current) // This condition can be reached if current node was a link
      continue;
    if(current->type == FSNODE_DIR)
      continue;
    kmod_loadf(current);
  }

  return 0;
}
