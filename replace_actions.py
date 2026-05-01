import os
import re

skip_pattern = re.compile(r"^libs/full/actions/")
include_pattern = re.compile(r'^[ \t]*#[ \t]*include[ \t]+<hpx/actions/[^>]+>.*$', re.MULTILINE)
module_include = "#include <hpx/modules/actions.hpp>"

def process_file(filepath):
    if skip_pattern.match(filepath): return
    try:
        with open(filepath, 'r', encoding='utf-8') as f: content = f.read()
    except UnicodeDecodeError:
        with open(filepath, 'r', encoding='latin-1') as f: content = f.read()

    if not include_pattern.search(content): return

    lines = content.split('\n')
    new_lines, added_module, modified = [], False, False

    for line in lines:
        if include_pattern.match(line):
            modified = True
            if not added_module:
                new_lines.append(module_include)
                added_module = True
        else:
            if line.strip() == module_include: added_module = True
            new_lines.append(line)

    if modified:
        with open(filepath, 'w', encoding='utf-8') as f: f.write('\n'.join(new_lines))

if __name__ == '__main__':
    for root, dirs, files in os.walk('.'):
        if '.git' in dirs: dirs.remove('.git')
        if 'build' in dirs: dirs.remove('build')
        for file in files:
            if file.endswith(('.hpp', '.cpp')):
                filepath = os.path.relpath(os.path.join(root, file), '.')
                process_file(filepath)
