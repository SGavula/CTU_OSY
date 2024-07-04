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
    # Remove new line from input and use sed to extract sentences and print each sentence on a new line
    echo "$INPUT" | tr "\n" " " | grep -o -P "\p{Lu}[^.?!]*[.?!]"
}

process_d_flag() {
    local PREFIX CONTENT
    # Extracting the value of the '-d' option and storing it
    PREFIX="$OPTARG"
    # Reading content of the file
    CONTENT=$(cat)
    # Checking if the content starts with "// #i"
    
    if [[ $CONTENT =~ ^"// #i" ]]; then
        # If the condition is true, echo the content as is
        echo "$CONTENT"
    else
        # Modifying #include directives within double quotes (") and withing brackets (<) by adding PREFIX before the filenames
        echo "$CONTENT" | sed -E "s|#include\s*\"|\0$PREFIX|" | sed -E "s|#\s*include\s*<|\1$PREFIX\2|"
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
