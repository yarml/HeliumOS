#!/usr/bin/python3
# Check all project files and flags the ones which have:
#   - lines longer than MAX_LEN(80) characters
#   - hard coded 4096 page size
#   - Trailing space at the end

# PWD should be project directory

import os

MAX_LEN = 80

def main():
  for root, _, files in os.walk('./'):
    if root.startswith(('./.git',
                        './build',
                        './ext',
                        './sysroots',
                        './tests',
                        './.vscode',
                        './cache')):
      continue
    for file in files:
      with open(os.path.join(root, file)) as f:
        try:
          for lnum, line in enumerate(f.readlines()):
            fname_line = f'{os.path.join(root, file)}:{lnum+1}'
            if len(line) > MAX_LEN + 1:
              print(f'Too long: {fname_line}')
            if line[len(line) - 2].isspace() and len(line) != 1:
              print(f'Trailing space: {fname_line}')
            if '4096' in line and line != '#define MEM_PS (4096)\n':
              print(f'Hard coded page size: {fname_line}')
        except UnicodeDecodeError:
          pass


if __name__ == '__main__':
  main()
