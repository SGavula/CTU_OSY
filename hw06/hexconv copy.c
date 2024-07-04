#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_EXIT 1

// Custom syscalls
int custom_read(char *buf, int len) {
    int ret;
    asm volatile (
        // "mov $3, %%eax\n" // syscall number for read
        // "mov $0, %%ebx\n" // file descriptor 0 stdin
        // "mov %1, %%ecx\n" // buffer address
        // "mov %2, %%edx\n" // number of bytes to read
        // "int $0x80"        // interrupt to invoke syscall
        // : "=a" (ret)       // saved returned value from interruption (int $0x80)
        // : "r" (buf), "r" (len) // input values
        // : "ebx", "ecx", "memory"
        "int $0x80"
        : "=a" (ret)
        : "0"(SYS_READ), "b"(STDIN_FILENO), "c"(buf), "d"(len)
        : "memory"
    );
    return ret;
}

int custom_write(const char *buf, int len) {
    int ret;
    asm volatile (
        // "mov $4, %%eax\n"   // syscall number for write
        // "mov $1, %%ebx\n"   // file descriptor 1 stdout
        // "mov %1, %%ecx\n"   // buffer address
        // "mov %2, %%edx\n"   // number of bytes to write
        // "int $0x80"          // interrupt to invoke syscall
        // : "=r" (ret)         // saved returned value from interruption (int $0x80) 
        // : "r" (buf), "r" (len) // input values
        // : "eax", "ebx", "ecx", "edx", "memory"
        "int $0x80"
        : "=a" (ret)
        : "0"(SYS_WRITE), "b"(STDOUT_FILENO), "c"(buf), "d"(len)
        : "memory"
    );
    return ret;
}

void custom_exit(int ret_value) {
    asm volatile (
        "mov $1, %%eax\n" // syscall number for exit
        "mov %0, %%ebx\n" // move ret_value into EBX
        "int $0x80" // interrupt to invoke syscall
        :
        : "r"(ret_value) // input value
        : "eax", "ebx", "memory"
    );
}

int custom_strlen(const char *buf) {
    int length = 0;

    while(buf[length] != '\0') {
        length++;
    }

    return length;
}

void custom_sprintf(char *buf, int num) {
    // Add first two letters to indicate number in hexadecimal format
    buf[0] = '0';
    buf[1] = 'x';

    int i = 2;
    
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

int isnum(char ch)
{
    return ch >= '0' && ch <= '9';
}

int isspc(char ch)
{
    return ch == ' ' || ch == '\n';
}

static void print(unsigned num)
{
    char buf[20];
    custom_sprintf(buf, num);
    if(custom_strlen(buf) > 3) {
        int ret = custom_write(buf, custom_strlen(buf));
        if (ret == -1) {
            custom_exit(1);
        }
    }
}

void custom_print(char *str) {
    // Use custom_write to make a system call to write the string
    custom_write(str, custom_strlen(str));
}

void print_num(int number) {
    char *numbers[] = {
        "0\n", "1\n", "2\n", "3\n", "4\n", "5\n", "6\n", "7\n", "8\n", "9\n",
        "10\n", "11\n", "12\n", "13\n", "14\n", "15\n", "16\n", "17\n", "18\n", "19\n",
        "20\n", "21\n", "22\n", "23\n", "24\n", "25\n", "26\n", "27\n", "28\n", "29\n",
        "30\n", "31\n", "32\n", "33\n", "34\n", "35\n", "36\n", "37\n", "38\n", "39\n",
        "40\n", "41\n", "42\n", "43\n", "44\n", "45\n", "46\n", "47\n", "48\n", "49\n",
        "50\n", "51\n", "52\n", "53\n", "54\n", "55\n", "56\n", "57\n", "58\n", "59\n",
        "60\n", "61\n", "62\n", "63\n", "64\n", "65\n", "66\n", "67\n", "68\n", "69\n",
        "70\n", "71\n", "72\n", "73\n", "74\n", "75\n", "76\n", "77\n", "78\n", "79\n",
        "80\n", "81\n", "82\n", "83\n", "84\n", "85\n", "86\n", "87\n", "88\n", "89\n",
        "90\n", "91\n", "92\n", "93\n", "94\n", "95\n", "96\n", "97\n", "98\n", "99\n",
        "100\n"
    };
    custom_print(numbers[number]);
}

int _start()
{
    char buf[20];
    unsigned num = 0;
    int i;
    int num_digits = 0;
    unsigned chars_to_process = 0;

    for (; ; i++, chars_to_process--) {
        if (chars_to_process == 0) {
            int ret = custom_read( buf, sizeof(buf));
            if (ret < 0) {
                custom_exit(1);
            }
            i = 0;
            chars_to_process = ret;
        }
        

        if (
            num_digits > 0 && (chars_to_process == 0 /* EOF */ || !isnum(buf[i]))
        ) {
            // custom_print("Hello\n");
            print_num(num_digits);
            print(num);
            num_digits = 0;
            num = 0;
        }
        if ( chars_to_process == 0 /* EOF */ || (!isspc(buf[i]) && !isnum(buf[i]))) {
            custom_exit(0);
        }

        if (isnum(buf[i])) {
            num = num * 10 + buf[i] - '0';
            num_digits++;
        }
    }
}