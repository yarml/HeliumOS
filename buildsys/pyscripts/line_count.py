import os

from itertools import chain

# TODO: Only count text files

collections = {
    'C source code': ('c', 'h'),
    'Assembly source code': ('asm',),
    'Source code': ('c', 'h', 'asm'),
    'Build system': ('mk', 'py', 'sh', 'sed'),
    'Configuration': ('ld', 'config', 'json', 'gdbinit'),
    'Documentation': ('md',)
}

paths = ('kernel/', 'buildsys/', 'docs/')
files = ['README.md', 'LICENSE.md','.gdbinit']

def main():
  total_lines = 0

  ext_size = dict()

  for (dirpath, _, filenames) in chain.from_iterable(
    os.walk(path) for path in paths
  ):
    for filename in filenames:
      files.append(f'{dirpath}/{filename}')

  for filename in files:
    with open(filename, 'r') as f:
      content = f.read()
      split_name = (filename).split('.')
      line_count = content.count('\n') + 1
      total_lines += line_count
      if len(split_name) <= 1:
        ext_size[''] = ext_size.get('', 0) + line_count
      else:
        ext_size[split_name[-1]] = ext_size.get(split_name[-1], 0) + line_count

  for ext, l in sorted(ext_size.items()):
    if ext != '':
      print(f'.{ext}\t\t: {l}')

  print(f'No extention\t: {ext_size.get("", 0)}')

  print()

  print('---Collections---')

  for name, exts in (collections.items()):
    sum = 0
    for ext in exts:
      sum += ext_size[ext]
    print(f'{name}\t\t: {sum}')

  print()

  print(f'Total lines\t: {total_lines}')

if __name__ == '__main__':
  main()