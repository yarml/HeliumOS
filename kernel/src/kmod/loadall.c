#include <kmod.h>

void kmod_loadall() {
  if (ksym_loadp("initrd://sys/ksym")) {
    error_inv_state("Could not load kernel symbols.");
  }

  fsnode *moddir = fs_search("initrd://modules/");
  if (!moddir) {  // No modules to load
    return;
  }

  fsnode *modit = fs_nextnode(moddir, 0);
  while (modit) {
    fsnode *current = modit;
    modit           = fs_nextnode(moddir, modit);
    if (current->type == FSNODE_LINK) {
      current = current->link.target;
    }
    if (!current) {  // This condition can be reached if current node was a link
      continue;
    }
    if (current->type == FSNODE_DIR) {
      continue;
    }
    kmod_loadf(current);
  }
}