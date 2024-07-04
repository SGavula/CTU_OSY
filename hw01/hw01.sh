#!/bin/bash

# Function to process a symlink
process_symlink() {
    local PATH_TO TARGET_PATH
    PATH_TO="$1"
    TARGET_PATH=$(readlink "$PATH_TO")
    echo "LINK '$PATH_TO' '$TARGET_PATH'"
}

# Function to process a file
process_file() {
    local PATH_TO FIRST_LINE HEAD_EXIT_STATUS NUM_OF_LINES
    PATH_TO="$1"
    FILES_TO_ARCHIVE+=("$PATH_TO")
    if [[ -s $PATH_TO ]]; then
        FIRST_LINE=$(head -n 1 "$PATH_TO")
        # Check if reading file is available
        HEAD_EXIT_STATUS=$?
        if [[ HEAD_EXIT_STATUS -ne 0 ]]; then
            exit 2
        fi
        NUM_OF_LINES=$(wc -l < "$PATH_TO")
        echo "FILE '$PATH_TO' $NUM_OF_LINES '$FIRST_LINE'"
    else
        echo "FILE '$PATH_TO' 0 ''"
    fi
}

# Function to process a directory
process_directory() {
    local PATH_TO
    PATH_TO="$1"
    echo "DIR '$PATH_TO'"
}

# Function to process an error
process_error() {
    local PATH_TO
    PATH_TO="$1"
    echo "ERROR '$PATH_TO'" 1>&2
    exit 1
}

# Function to process input lines
process_lines() {
    local LINE PATH_TO
    # Read lines from standard input until end-of-file (EOF) signal is received
    while read -r LINE; do
        # Check if line starts with PATH
        if [[ "$LINE" = "PATH "* ]]; then
            # Check type of path
            PATH_TO=${LINE#PATH }
            if [[ -L "$PATH_TO" ]]; then
                process_symlink "$PATH_TO"
            elif [[ -f "$PATH_TO" ]]; then
                process_file "$PATH_TO"
            elif [[ -d "$PATH_TO" ]]; then
                process_directory "$PATH_TO"
            else
                process_error "$PATH_TO"
            fi
        fi
    done
}

process_h_flag() {
    echo "Simple BASH script that reads lines from standard input, processes filesystem paths and can be run with the -h or with -z flags"
    exit 0
}

process_z_flag() {
    process_lines
    # Check if the array is empty
    if [ ${#FILES_TO_ARCHIVE[@]} -eq 0 ]; then
        echo "No files to archive."
        exit 0
    fi
    # Archive the file with tar function
    tar czf output.tgz "${FILES_TO_ARCHIVE[@]}"
    exit $?
}

#Global array
declare -a FILES_TO_ARCHIVE=()

# Check for the -h and -z flags
while getopts "hz" opt; do
    case $opt in
        h)
            process_h_flag
            ;;
        z)
            process_z_flag
            ;;
        \?)
            exit 2
            ;;
    esac
done

# If no flag is provided, process input lines
process_lines
exit $?
