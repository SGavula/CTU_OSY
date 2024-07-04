#include <stdio.h>

int main() {
    int userInput;

    printf("Enter an integer: ");
    
    if (scanf("%d", &userInput) == 1) {
        printf("You entered: %d\n", userInput);
    } else {
        printf("Invalid input.\n");
    }

    return 0;
}
