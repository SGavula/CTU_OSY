#!/bin/bash

# Function to process input lines
process_lines() {
    # Read lines from standard input until end-of-file (EOF) signal is received
    while read -r LINE; do
        if [[ "$LINE" = "PATH "* ]]; then
            # echo $LINE
            PATH_TO=${LINE#PATH }
            if [[ -L "$PATH_TO" ]]; then
                TARGET_LINK_PATH=$(readlink "$PATH_TO")
                echo "LINK '$PATH_TO' '$TARGET_LINK_PATH'"
            elif [[ -f "$PATH_TO" ]]; then
                FILES_TO_ARCHIVE+=("$PATH_TO")
                if [[ -s $PATH_TO ]]; then
                    FIRST_LINE=$(head -n 1 "$PATH_TO")
                    HEAD_EXIT_STATUS=$?
                    if [[ HEAD_EXIT_STATUS -ne 0 ]]; then
                        exit 2
                    fi
                    NUM_OF_LINES=$(wc -l < "$PATH_TO")
                    echo "FILE '$PATH_TO' $NUM_OF_LINES '$FIRST_LINE'"
                else
                    FIRST_LINE=""
                    echo "FILE '$PATH_TO' 0 '$FIRST_LINE'"
                fi
            elif [[ -d "$PATH_TO" ]]; then
                echo "DIR '$PATH_TO'"
            else
                echo "ERROR '$PATH_TO'" 1>&2
                exit 1
            fi
        fi
    done
}

#Global array
declare -a FILES_TO_ARCHIVE=()

# Check for the -h and -z flags
while getopts "hz" opt; do
    case $opt in
        h) 
            echo "Simple BASH script that reads lines from standard input, processes filesystem paths and can be run with the -h or with -z flags"
            exit 0
            ;;
        z)
            process_lines
            # Check if the array is empty
            if [ ${#FILES_TO_ARCHIVE[@]} -eq 0 ]; then
                echo "No files to archive."
                exit 0
            fi
            tar czf output.tgz "${FILES_TO_ARCHIVE[@]}"
            exit $?
            ;;
        \?)
            echo "Invalid flag."
            exit 2
            ;;
    esac
done

# If no flag is provided, process input lines
process_lines
exit $?
