#include <stdio.h>

int main()
{
    unsigned num;
    while (scanf("%u", &num) == 1)
        printf("0x%x\n", num);
    return 0;
}

#include <stdio.h>

void custom_sprintf(char *buf, int num) {
    // Add first two letters to indicate number in hexadecimal format
    buf[0] = '0';
    buf[1] = 'x';

    int i = 2;

    // Handle the case when num is zero
    if (num == 0) {
        buf[i++] = '0';
    } else {
        // Convert decimal number to hexadecimal
        while (num > 0) {
            int remainder = num % 16;
            buf[i] = (remainder < 10) ? (remainder + '0') : (remainder - 10 + 'a');
            i++;
            num /= 16;
        }

        // Reverse hex number
        int start = 2;
        int end = i - 1;

        while (start < end) {
            // Swap characters at start and end indices
            char temp = buf[start];
            buf[start] = buf[end];
            buf[end] = temp;

            // Move towards the center
            start++;
            end--;
        }
    }

    // Add newline and null terminating character
    buf[i] = '\n';
    i++;
    buf[i] = '\0';
}

int main() {
    char buffer[20];
    
    custom_sprintf(buffer, 0);
    printf("%s", buffer);

    return 0;
}
