#!/bin/bash
# find -name "*.sh" | sort

# INPUT=$(cat)
# INPUT_WITHOUT_NEWLINE=$(echo "$INPUT" | tr -d '\n')
# # Use sed to extract sentences and print each sentence on a new line
# echo "$INPUT_WITHOUT_NEWLINE" | grep -o -E '[A-Z][^.?!]*[.?!]'

# Set the PREFIX variable (make sure to set it to your desired value)
# PREFIX="lib/"

# # Example content with #include "foo.h"
# CONTENT='Some content #include "foo.h"'

# # Replace #include "foo.h" with #include "$PREFIX"
# # NEW_CONTENT=$(echo "$CONTENT" | sed "s|#include \"foo.h\"|#include \"$PREFIX\"|g")
# NEW_CONTENT=$(echo "$CONTENT" | sed "s|#include \"foo.h\"|#include \"$PREFIX\"|g")

# echo "$NEW_CONTENT"

RESULT=$(echo "$LINE" | sed "s/[+-]\?[0-9]*/_/")
echo "$RESULT"