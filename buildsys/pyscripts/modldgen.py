import sys

def main():
  if len(sys.argv) != 3:
    sys.stderr.write(f'Usage: {sys.argv[0]} <template> <out-ld-file>\n')
    exit(1)

  content = str()
  out_path = sys.argv[2]
  template_path = sys.argv[1]

  types = ('abs', 'bss', 'com', 'data', 'text', 'rodata', 'unk')

  with open(template_path, 'r') as template:
    content = template.read()
    for t in types:
      with open(f'{out_path}.{t}', 'r') as input_ksym:
        content = content.replace(f'<{t}-ksym>', input_ksym.read())
  with open(out_path, 'w') as out_file:
    out_file.write(content)

if __name__ == '__main__':
  main()
