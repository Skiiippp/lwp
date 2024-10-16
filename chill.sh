#!/bin/bash

# Directory containing the files you want to process
DIRECTORY="./"  # Change this to your directory path

# clang-format style configuration
CLANG_FORMAT_STYLE="{BasedOnStyle: llvm, UseTab: Never, TabWidth: 4, IndentWidth: 4, ColumnLimit: 79}"

# Process each .c and .h file in the directory
for file in "$DIRECTORY"/*; do
    # Check if the file has a .c or .h extension
    if [[ "$file" == *.c || "$file" == *.h ]]; then
        echo "Processing file: $file"
        clang-format -i -style="$CLANG_FORMAT_STYLE" "$file"
    fi
done

echo "All .c and .h files processed."
