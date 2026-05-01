import os
import re
import glob

def process_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    if "GLOBAL_HEADER_MODULE_GEN" in content:
        return

    # Find the add_hpx_module block
    add_module_match = re.search(r'add_hpx_module\s*\([^)]+\)', content, re.DOTALL)
    if not add_module_match:
        return
    
    add_module_block = add_module_match.group(0)

    # find detail headers
    detail_headers = set()
    # matches set(module_headers ...) and also the headers inside
    headers_matches = re.findall(r'hpx/[a-zA-Z0-9_]+/detail/[a-zA-Z0-9_./]+\.hpp', content)
    for h in headers_matches:
        detail_headers.add(h)

    # Prepare replacements
    add_lines = ["  GLOBAL_HEADER_MODULE_GEN ON"]
    if detail_headers:
        add_lines.append(f'  ADD_TO_GLOBAL_HEADER {" ".join(sorted(detail_headers))}')

    # Insert into add_hpx_module block
    # We will insert right after GLOBAL_HEADER_GEN ON
    if 'GLOBAL_HEADER_GEN ON' in add_module_block:
        replacement = add_module_block.replace('GLOBAL_HEADER_GEN ON', 'GLOBAL_HEADER_GEN ON\n' + '\n'.join(add_lines))
        new_content = content.replace(add_module_block, replacement)
    else:
        # insert after the first line of add_hpx_module which is the module type and name
        # e.g. core async_sycl
        lines = add_module_block.split('\n')
        # lines[1] is usually the type and name
        lines.insert(2, '\n'.join(add_lines))
        replacement = '\n'.join(lines)
        new_content = content.replace(add_module_block, replacement)

    with open(filepath, 'w') as f:
        f.write(new_content)
    print(f"Modularized {filepath}")

for cmake_file in glob.glob('libs/*/*/CMakeLists.txt'):
    process_file(cmake_file)

