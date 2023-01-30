#!/usr/bin/python3
# Check all project files and flags the ones which have a line longer than
# MAX_LEN(80) characters

# PWD should be project directory

import os

MAX_LEN = 80

def main():
    for root, dirs, files in os.walk('./'):
        if root.startswith(('./.git', './build', './test/build', './ext', './sysroots', './tests', './.vscode')):
            continue
        for file in files:
            with open(root + '/' + file) as f:
                for lnum, line in enumerate(f.readlines()):
                    if len(line) > MAX_LEN + 1:
                        print(f'Too long: {root + "/" + file}:{lnum+1}')
                    if line[len(line) - 2].isspace() and len(line) != 1:
                        print(f'Trailing space: {root + "/" + file}:{lnum+1}')


if __name__ == '__main__':
    main()
