#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_EXIT 1

// Deklarations of functions
int custom_read(char *buf, int len);
int custom_write(const char *buf, int len);
void custom_exit(int ret_value);
int custom_strlen(const char *buf);
void custom_sprintf(char *buf, unsigned num);
int isnum(char ch);
int isspc(char ch);
static void print(unsigned num);

/* MAIN*/
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

// Definitions of functions
// Custom syscalls
int custom_read(char *buf, int len) {
    int ret;
    asm volatile (
        "mov $3, %%eax\n" // syscall number for read
        "mov $0, %%ebx\n" // file descriptor 0 stdin
        "mov %1, %%ecx\n" // buffer address
        "mov %2, %%edx\n" // number of bytes to write
        "int $0x80" // interrupt to invoke syscall
        : "=r" (ret) // saved returned value from interruption (int $0x80)
        : "r"(buf), "r"(len) //input values
        : "ecx", "edx" ,"memory"
    );
    return ret;
}

int custom_write(const char *buf, int len) {
    int ret;
    asm volatile (
        "mov $4, %%eax\n" // syscall number for write
        "mov $1, %%ebx\n" // file descriptor 1 stdout
        "mov %1, %%ecx\n" // buffer address
        "mov %2, %%edx\n" // number of bytes to write
        "int $0x80" // interrupt to invoke syscall
        : "=r" (ret) // saved returned value from interruption (int $0x80) 
        : "r"(buf), "r"(len) // input values
        : "eax", "ebx", "ecx", "edx", "memory"
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

void custom_sprintf(char *buf, unsigned num) {
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
    int ret = custom_write(buf, custom_strlen(buf));
    if (ret == -1) {
        custom_exit(1);
    }
}
