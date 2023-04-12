# Checks which modules are enabled and generates a makefile to build them

import sys
import re
import os

ROOT_DIR = f'{os.getcwd()}/'
MODULE_TEMPLATE = f'{ROOT_DIR}buildsys/templates/module.mk'
BUILD_DIR = f'{ROOT_DIR}build/'
MODULES_DIR = f'{ROOT_DIR}modules/'
MODULES_CFG = f'{MODULES_DIR}modules.cfg'

def main():
  # Read template file
  template = str()
  with open(MODULE_TEMPLATE, 'r') as mt:
    template = mt.read()
  modules = []
  with open(MODULES_CFG, 'r') as mcfg:
    for i, line in enumerate(mcfg.readlines()):
      line = line.strip()
      if len(line) == 0:
        continue
      m = re.search('([a-zA-Z0-9_-]+)\\s*=\\s*([MU])', line)
      if m == None:
        sys.stderr.write(f'Invalid line in {MODULES_CFG}:{line} \'{line}\'')
        exit(1)
      mod_name = m.group(1)
      mod_en = m.group(2)
      match mod_en:
        case 'M':
          print(f'Including module: \'{mod_name}\'')
          modules.append(mod_name)
        case 'U':
          print(f'Skipping module: \'{mod_name}\'')
  for mod in modules:
    with open(f'{BUILD_DIR}buildsys/{mod}.mk', 'w') as mmk:
      mmk.write(template.replace('<mod-name>', mod))

if __name__ == '__main__':
  main()
