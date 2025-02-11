import os
import re

def find_includes(file_path):
    includes = []
    include_pattern = r'#include\s*<([^>]+)>'
    try:
        with open(file_path, 'r') as file:  # No encoding specified
            for line in file:
                #print(f"Line content: {line.strip()}")  # Output the content of each line
                match = re.search(include_pattern, line)
                if match:
                    include_file = match.group(1)
                    includes.append(include_file)
                    print(f"Found include: {include_file} in file: {file_path}")  # Debugging output
    except Exception as e:
        print(f"Error processing file: {file_path}, Error: {e}")
    return includes

def generate_dot(source_dir):
    dot_content = 'digraph dependencies {\n'
    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith('.h') or file.endswith('.cpp'):
                file_path = os.path.join(root, file)
                print(f"Processing file: {file_path}")  # Debugging output
                includes = find_includes(file_path)
                if includes:
                    for include in includes:
                        dot_content += f'"{file}" -> "{include}";\n'
                else:
                    print(f"No includes found in file: {file_path}")  # Debugging output
                #input("Press Enter to continue...")  # Pause after processing each file
    dot_content += '}'
    return dot_content

source_directory = 'E:/PROJECTS/FLUORENDER/fluorender_work/FluoRender/FluoRender/Progress'
dot_file_content = generate_dot(source_directory)

with open('header_dependencies.dot', 'w') as dot_file:
    dot_file.write(dot_file_content)