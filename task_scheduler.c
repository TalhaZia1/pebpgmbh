#include <stdio.h> 
#include <time.h>
#include <string.h>

int main(void) {
    time_t real_time; 
    struct tm *info; 
    char buffer[80U];

    while(1) {
        time(&real_time);
        info = localtime(&real_time);

        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", info);
        printf("Current time: %s\n", buffer);
        printf("[Command #]: ");
        char input[100];

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        if (strcmp(input, "exit\n") == 0U) {
            break;
        }
    }

    return 0;
}