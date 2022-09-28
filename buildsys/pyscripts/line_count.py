import os

# TODO: Only count text files

def main():
    total_lines = 0

    ext_size = dict()

    for (dirpath, _, filenames) in os.walk("kernel/"):
        for filename in filenames:
            with open(dirpath + '/' + filename, 'r') as f:
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

    print(f'Total lines\t: {total_lines}')

if __name__ == '__main__':
    main()