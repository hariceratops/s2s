import os
import re
import sys

# Regex to match non-system #include directives (i.e., with quotes, not angle brackets)
include_pattern = r'^\s*#include\s*"([^"]+)"\s*$'
include_regex = re.compile(include_pattern)

def process_file(file_path):
    """Extracts include dependencies from a single file."""
    includes = []
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                match = include_regex.match(line)
                if match:
                    includes.append(match.group(1))
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
    print(f"{file_path} = {includes}")
    return includes

def process_directory(directory):
    """Recursively processes a directory to find and parse C/C++ files."""
    graph = {}

    for root, _, files in os.walk(directory):
        for filename in files:
            if filename.endswith(('.h', '.hpp', '.c', '.cpp')):
                file_path = os.path.join(root, filename)
                includes = process_file(file_path)

                # Convert include filenames to absolute paths
                include_paths = []
                for include in includes:
                    include_path = os.path.join(root, include)
                    if os.path.isfile(include_path):
                        include_paths.append(os.path.abspath(include_path))
                    else:
                        print(f"Warning: Include file not found: {include_path} (included in {file_path})")

                graph[os.path.abspath(file_path)] = include_paths

    return graph

def get_all_transitive_dependencies(file, graph, visited=None):
    """Recursively gets all dependencies a file brings in."""
    if visited is None:
        visited = set()
    if file in visited:
        return set()
    visited.add(file)

    transitive_deps = set(graph.get(file, []))
    for dep in graph.get(file, []):
        transitive_deps |= get_all_transitive_dependencies(dep, graph, visited)

    return transitive_deps

def remove_redundant_includes(graph):
    """Removes redundant includes while ensuring all dependencies remain resolved."""
    optimized_graph = {}

    for file, includes in graph.items():
        # Get all transitive dependencies that would be included indirectly
        transitive_deps = set()
        for inc in includes:
            transitive_deps |= get_all_transitive_dependencies(inc, graph)

        # Keep only those includes that aren't covered by transitive dependencies
        optimized_graph[file] = [inc for inc in includes if inc not in transitive_deps]

    return optimized_graph

def topological_sort(graph, start_file):
    """Performs a topological sort of the include graph starting from a given file."""
    visited = set()
    stack = []

    def visit(file):
        if file in visited:
            return
        visited.add(file)
        for include in graph.get(file, []):
            visit(include)
        stack.append(file)

    visit(start_file)
    # return stack[::-1]  # Reverse to get the correct include order
    return stack

def concatenate_files(file_order, output_path):
    """Concatenates file contents into a single output file."""
    seen_files = set()

    try:
        with open(output_path, 'w', encoding='utf-8') as output_file:
            for file_path in file_order:
                if file_path not in seen_files:
                    seen_files.add(file_path)
                    with open(file_path, 'r', encoding='utf-8') as input_file:
                        input_file_contents = input_file.read()
                        input_file_contents = re.sub(include_pattern, ' ', input_file_contents, flags=re.MULTILINE)
                        output_file.write(f"\n// Begin {file_path}\n")
                        output_file.write(input_file_contents)
                        output_file.write(f"\n// End {file_path}\n")
        print(f"\nSuccessfully wrote concatenated output to: {output_path}")
    except Exception as e:
        print(f"Error writing to output file {output_path}: {e}")

def main():
    print(sys.argv)
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <directory path> <entry file> <output file>")
        sys.exit(1)

    directory = sys.argv[1]
    entry_file = (sys.argv[2])
    output_file = (sys.argv[3])

    if not os.path.isdir(directory) or not os.path.isfile(entry_file):
        print(f"Invalid directory or entry file.")
        sys.exit(1)

    include_graph = process_directory(directory)

    if entry_file not in include_graph:
        print(f"Entry file {entry_file} not found in project.")
        sys.exit(1)

    print("\nOriginal Include Graph:")
    for file, includes in include_graph.items():
        print(f"{file} -> {includes}")

    optimized_graph = remove_redundant_includes(include_graph)

    print("\nOptimized Include Graph:")
    for file, includes in optimized_graph.items():
        print(f"{file} -> {includes}")

    # Get include order (topological sort)
    file_order = topological_sort(optimized_graph, entry_file)

    print("\nInclude Order:")
    for file in file_order:
        print(file)

    # Concatenate and write to output file
    concatenate_files(file_order, output_file)

if __name__ == '__main__':
    main()
