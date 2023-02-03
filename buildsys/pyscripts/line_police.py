#!/usr/bin/python3
# Check all project files and flags the ones which have a line longer than
# MAX_LEN(80) characters

# PWD should be project directory

import os

MAX_LEN = 80

def main():
    for root, dirs, files in os.walk('./'):
        if root.startswith(('./.git', './build', './ext', './sysroots', './tests', './.vscode')):
            continue
        for file in files:
            with open(root + '/' + file) as f:
                for lnum, line in enumerate(f.readlines()):
                    fname_line = f'{root + "/" + file}:{lnum+1}'
                    if len(line) > MAX_LEN + 1:
                        print(f'Too long: {fname_line}')
                    if line[len(line) - 2].isspace() and len(line) != 1:
                        print(f'Trailing space: {fname_line}')
                    if '4096' in line:
                        print(f'Potentially hard coded page size: {fname_line}')


if __name__ == '__main__':
    main()
