#include <stdio.h> 
#include <stdint.h>
#include <string.h>
#include "bool.h"
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

/*****************************/
/* Variable Type Definitions */
/*****************************/

typedef enum {
    INIT,
    SILENT,
    PENDING_ACTIVITY,
    ACTIVITY_COMPLETED
} state;

typedef enum {
    CMD_NOW,
    CMD_DONE,
    CMD_NONE
} commands;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} t_time;

typedef enum {
    BREAK_FAST              = 0U,
    MORNING_WALK            = 1U,
    LUNCH                   = 2U,
    MEDICATION              = 3U,
    FEEDING_BIRDS           = 4U,
    BRUNCH                  = 5U,
    WATERING_PLANTS         = 6U,
    DINNER                  = 7U,
    TOTAL_ACTIVITIES        = 8U,
    IDLE                    = 9U
} activities;

typedef enum {
    UNDONE, 
    DONE
} status;

typedef struct {
    activities const toDo;
    t_time     const startTime;
    t_time     const endTime;
    status           state;
    char       const undone_msg[50U];
    char       const done_msg[50U];
} t_model;

/*************************/
/* Variables Definitions */
/*************************/

static t_model dataModel[TOTAL_ACTIVITIES] = {
    /*0*/ {BREAK_FAST      , /*start time*/ { 6U,30U}, /*end time*/ { 8U,30U },UNDONE,"Have you eaten breakfast ?",      "You have already eaten your breakfast"},
    /*1*/ {MORNING_WALK    , /*start time*/ { 9U, 0U}, /*end time*/ {10U, 0U },UNDONE,"Have you gone for morning walk ?","You have already done your morning walk"}, 
    /*2*/ {LUNCH           , /*start time*/ {12U, 0U}, /*end time*/ {13U, 0U },UNDONE,"Have you eaten lunch ?",          "You have already eaten your lunch"}, 
    /*3*/ {MEDICATION      , /*start time*/ {13U, 0U}, /*end time*/ {14U, 0U },UNDONE,"Have you taken medicines ?",      "You have already taken your medicines"}, 
    /*4*/ {FEEDING_BIRDS   , /*start time*/ {15U, 0U}, /*end time*/ {16U, 0U },UNDONE,"Have you fed the birds ?",        "You have already feed the birds"}, 
    /*5*/ {BRUNCH          , /*start time*/ {16U, 0U}, /*end time*/ {17U, 0U },UNDONE,"Have you eaten brunch ?",         "You have already eaten your brunch"},  
    /*6*/ {WATERING_PLANTS , /*start time*/ {17U, 0U}, /*end time*/ {18U, 0U },UNDONE,"Have you water your plants ?",    "You have already water your plants"}, 
    /*7*/ {DINNER          , /*start time*/ {18U, 0U}, /*end time*/ {19U, 0U },UNDONE,"Have you eaten dinner ?",         "You have already eaten your dinner"}
};

/*********************/
/* APIs Declarations */
/*********************/

void printWelcomeMessage(void);
bool isTimeValid(t_time const * const time);
bool getTime(t_time * const time);
void updateTime(t_time * const time);
bool isTimeAboutToEnd(t_time const * const end_time, t_time const * const time);
bool activityTimeOut(t_time const * const end_time, t_time const * const time);
bool isGreaterThan(t_time const * const activityTime, t_time const * const time);
bool isLessThan(t_time const * const activityTime, t_time const * const time);
bool checkActivity(uint8_t const index, t_time const * const time);
uint8_t checkAll(t_time const * const time);
void handle_non_blocking_input(commands * const cmd);

/********************/
/* APIs Definitions */
/********************/

void printWelcomeMessage(void) {
    printf("Please enter your Current Local Time 'hh:mm'\n");
}

bool isTimeValid(t_time const * const time) {
    return (0 <= time->hours) && (time->hours < 24U) && (0 <= time->minutes) && (time->minutes < 60U);
}

bool getTime(t_time * const time) {
    return (scanf("%hhu:%hhu", &time->hours, &time->minutes) == 2U);
}

void updateTime(t_time * const time) {
    /* Updating Time every 1 second */
    sleep(1U);
    
    /* Update Seconds */
    if (time->seconds != 59U) {
        time->seconds++;
    } 
    else {
        time->seconds = 0U;
        /* Update Minutes */
        if (time->minutes != 59U) {
            time->minutes++;
        } 
        else {
            time->minutes = 0U;
            /* Update Hours */
            if (time->hours != 23U) {
                time->hours++;
            } 
            else {
                time->hours = 0U;
            }
        }
    }
}

bool isTimeAboutToEnd(t_time const * const end_time, t_time const * const time) {
    /* Converting Time data in Minutes */
    uint16_t const end_time_minutes = end_time->hours * 60U + end_time->minutes;
    uint16_t const time_minutes     = time->hours     * 60U + time->minutes;

    /* Checking if difference is 10mins or less */
    bool const result = ((end_time_minutes - time_minutes) <= 10U);
    
    return result;
}

bool activityTimeOut(t_time const * const end_time, t_time const * const time) {
    return isGreaterThan(end_time, time);
}

bool isGreaterThan(t_time const * const activityTime, t_time const * const time) {
    bool result = (time->hours != activityTime->hours) ? 
                  (time->hours > activityTime->hours) :        /* Only checking Hours is sufficient */
                  (time->minutes > activityTime->minutes);     /* If hours Equal then Check Minutes */
    return result;
}

bool isLessThan(t_time const * const activityTime, t_time const * const time) {
    bool result = (time->hours != activityTime->hours) ? 
                  (time->hours < activityTime->hours) :        /* Only checking Hours is sufficient */
                  (time->minutes < activityTime->minutes);     /* If hours Equal then Check Minutes */
    return result;
}

bool checkActivity(uint8_t const index, t_time const * const time) {
    bool result = false;

    if (!isGreaterThan(&dataModel[index].startTime, time)) {
        /* Time lower than Activity Start Time */
    } else if (!isLessThan(&dataModel[index].endTime, time)) {
        /* Time More than Activity End Time */
    } else {
        result = true;
    }

    return result;
}

uint8_t checkAll(t_time const * const time) {
    uint8_t index = (uint8_t)IDLE;
    for (uint8_t i = 0; i < (uint8_t)TOTAL_ACTIVITIES; i++ ) {
        if (checkActivity(i, time)) {
            index = i;
            break;
        }
    }
    return index;
}

void handle_non_blocking_input(commands * const cmd) {
    uint8_t const LENGTH = 50U;
    char userInput[LENGTH];
    fd_set read;
    FD_ZERO(&read);
    FD_SET(STDIN_FILENO, &read);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int ready_fds = select(STDIN_FILENO + 1, &read, NULL, NULL, &timeout);
    if (ready_fds < 0) {
        perror("select");
        exit(EXIT_FAILURE);
    }

    if (FD_ISSET(STDIN_FILENO, &read)) {
        char *result = fgets(userInput, LENGTH, stdin);
        if (result == NULL) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        userInput[strcspn(userInput, "\n")] = '\0'; // Remove newline character
        if (strcmp(userInput, "now")) {
            *cmd = CMD_NOW;            
        } else if (strcmp(userInput, "done")) {
            *cmd = CMD_DONE;
        } else {
            *cmd = CMD_NONE;
        }
    }
}

/*****************/
/* Main Function */
/*****************/
int main(void) {
    t_time real_time = {0U, 0U, 0U};
    uint8_t current_activity = (uint8_t)IDLE;
    state stateMachine = INIT;
    bool printUnDoneMessage = false;
    bool printReminder = false;
    commands user_command = CMD_NONE;

    printWelcomeMessage();

    while(1) {
        switch (stateMachine) {
            case INIT:
                /* Taking Time Input from User on Startup */
                bool input = getTime(&real_time);
                bool isValid = isTimeValid(&real_time);
                if (!input) {
                    printf("[ERROR] Try again in correct format 'hh:mm'\n");
                }
                else if(!isValid) {
                    printf("[ERROR] Try again in correct range\n");
                    printf(" *** Hours      --> 0 to 23\n");
                    printf(" *** Minutes    --> 0 to 59\n");
                }
                else {
                    printf("[INFO] Time Accepted \n");
                    printf("[INFO] Program will keep you update about your daily activites \n");
                    stateMachine = SILENT;
                }
                break;
            
            case SILENT:
                current_activity = checkAll(&real_time);
                stateMachine = (current_activity != (uint8_t)IDLE) ? PENDING_ACTIVITY : SILENT;
                break;

            case PENDING_ACTIVITY:
                if (dataModel[current_activity].state == DONE) {
                    stateMachine = ACTIVITY_COMPLETED;
                }
                else {
                    if (!printUnDoneMessage) {
                        /* Printing Activity Start Message */
                        printf("%s\n",dataModel[current_activity].undone_msg);
                        printUnDoneMessage = true;
                    }
                    else if (!printReminder && isTimeAboutToEnd(&dataModel[current_activity].endTime, &real_time)) {
                        /* Printing Activity Reminder Message */
                        printf("%s\n",dataModel[current_activity].undone_msg);
                        printReminder = true;
                    }
                    else if (activityTimeOut(&dataModel[current_activity].endTime, &real_time)) {
                        /* Activity is undone but Activity timeout is Activated */
                        stateMachine = SILENT;
                    }
                    else {
                        /* Default case */
                    }
                }
                break;

            case ACTIVITY_COMPLETED:
                /* Activity is Complete but Activity timeout is still pending */
                if (activityTimeOut(&dataModel[current_activity].endTime, &real_time)) {
                    stateMachine = SILENT;
                }
                break;

            default : 
                break;
        }

        /* For INIT state, waiting for user to enter current_time */
        if (stateMachine != INIT) {
            /* Using Linux APIs for handling commands on runtime */
            handle_non_blocking_input(&user_command);

            /* Parsing user Input command on runtime */
            if (user_command != CMD_NONE) {

                if (user_command == CMD_DONE) {
                    dataModel[current_activity].state = DONE;
                } 
                else if (user_command == CMD_NOW) {
                    if (dataModel[current_activity].toDo == IDLE) {
                        printf("Nothing to do right now\n");
                    } 
                    else {
                        if (dataModel[current_activity].state == DONE) {
                            printf("%s\n",dataModel[current_activity].done_msg);
                        } else {
                            printf("%s\n",dataModel[current_activity].undone_msg);
                        }
                    }
                } 
                else {
                    /* Do Nothing - Default Case */
                }
                user_command = CMD_NONE;
                tcflush(STDIN_FILENO, TCIFLUSH);
            }
        }
    }
    return 0;
}
