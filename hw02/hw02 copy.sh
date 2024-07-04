#!/bin/bash

process_h_flag() {
    echo "Simple BASH script that reads lines from standard input, processes filesystem paths and can be run with the -h or with -z flags"
    exit 0
}

process_b_flag() {
    local LINE RESULT
    # Read lines from standard input until end-of-file (EOF) signal is received
    while read -r LINE; do
        # Check if line starts with + or - sign followed by numbers
        if [[ "$LINE" =~ ^[+-]?[0-9]+ ]]; then
            # Remove + or - sign with number untill nonnumerical value 
            RESULT=$(echo "$LINE" | sed -E 's|^[-+]?[0-9]+||')
            echo "$RESULT"
        fi
    done
}

process_c_flag() {
    local INPUT RESULT
    # Get input
    INPUT=$(cat)
    # Remove new line from input
    # INPUT_WITHOUT_NEWLINE=$(echo "$INPUT" | tr "\n" " ")
    # Use sed to extract sentences and print each sentence on a new line
    # RESULT=$(echo "$INPUT_WITHOUT_NEWLINE" | grep -o -E "[A-ZČŠŽ][^.?!]*[.?!]")
    RESULT=$(echo "$INPUT" | tr "\n" " " | grep -o -E "[A-ZČŠŽ][^.?!]*[.?!]")
    echo "$RESULT"
}

process_d_flag() {
    local PREFIX CONTENT MODIFIED_INPUT_DQ MODIFIED_INPUT_LG_SIGNS
    # Extracting the value of the '-d' option and storing it
    PREFIX="$OPTARG"
    # Reading content of the file
    CONTENT=$(cat)
    # Checking if the content starts with "// #i"
    if [[ $CONTENT =~ ^"// #i" ]]; then
        # If the condition is true, echo the content as is
        echo "$CONTENT"
    else
        # Modifying #include directives within double quotes (") by adding PREFIX before the filenames
        # MODIFIED_INPUT_DQ=$(echo "$CONTENT" | sed -E "s|#include\s*\"|\0$PREFIX|")
        # Modifying #include directives within angle brackets (<) by adding PREFIX before the filenames
        # MODIFIED_INPUT_LG_SIGNS=$(echo "$MODIFIED_INPUT_DQ" | sed -E "s|#\s*include\s*<|\0$PREFIX|")
        # echo "$MODIFIED_INPUT_LG_SIGNS"
        echo "$CONTENT" | sed -E "s|#include\s*\"|\0$PREFIX|" | sed -E "s|#\s*include\s*<|\0$PREFIX|"
    fi
}


# Check for the flags
while getopts "habcd:" opt; do
    case $opt in
        h)
            process_h_flag
            ;;
        a)
            find . -iname "*.pdf" | sort | sed "s|^./||"
            exit $?
            ;;
        b)
            process_b_flag
            exit $?
            ;;
        c)
            process_c_flag
            exit $?
            ;;
        d)
            process_d_flag
            exit $?
            ;;
        \?)
            exit 1
            ;;
    esac
done
