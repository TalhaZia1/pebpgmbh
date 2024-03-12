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

/* Stating the Number of Seconds Increment */
typedef enum {
    NORMAL      = 1U,   
    SPEED_10X   = 10U, 
    SPEED_100X  = 100U,
    SPEED_1000X = 1000U,
} speed;

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

typedef struct {
    t_time real_time;
    uint8_t current_activity;
    state stateMachine;
    bool printUnDoneMessage;
    bool printReminder;
    commands user_command;
    uint8_t timeCounter; 
    speed const exe_speed;
} t_params;

/*************************/
/* Variables Definitions */
/*************************/

static const t_params RESET_PARAMS = {{0U, 0U, 0U}, (uint8_t)IDLE, INIT, false, false, CMD_NONE, 0U, NORMAL };

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
void updateTime(t_time * const time, uint16_t const step);
bool isTimeAboutToEnd(t_time const * const end_time, t_time const * const time);
bool activityTimeOut(t_time const * const end_time, t_time const * const time);
bool isGreaterThan(t_time const * const activityTime, t_time const * const time);
bool isLessThan(t_time const * const activityTime, t_time const * const time);
bool checkActivity(uint8_t const index, t_time const * const time);
uint8_t checkAll(t_time const * const time);
void handle_non_blocking_input(commands * const cmd);
bool userDelay(uint8_t * const timeCounter, commands * const cmd);

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

void updateTime(t_time * const time, uint16_t const step) {
    /* Converting steps to microseconds */
    /* In case of following speed scenarios */
    /* NORMAL       ==> 1sec == 1sec*/
    /* SPEED_10X    ==> 1sec == (1/10)sec*/
    /* SPEED_100X   ==> 1sec == (1/100)sec*/
    /* SPEED_1000X  ==> 1sec == (1/1000)sec*/
    /* Thus speeding up actual 1sec as per scenario */

    uint16_t sleep_duration = 1000000/step; 
    usleep(sleep_duration);

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

bool userDelay(uint8_t * const timeCounter, commands * const cmd) {
    bool result = false;
    if (*timeCounter >= 3U) {
        *timeCounter = 0U;
        *cmd = CMD_NONE;
        result = true;
    } else {
        *timeCounter = *timeCounter + 1U;
    }
    return result;
}

/*****************/
/* Main Function */
/*****************/
int main(void) {
    t_params op_params = RESET_PARAMS;
    printWelcomeMessage();

    while(1) {
        switch (op_params.stateMachine) {
            case INIT:
                /* Taking Time Input from User on Startup */
                bool input = getTime(&op_params.real_time);
                bool isValid = isTimeValid(&op_params.real_time);
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
                    op_params.stateMachine = SILENT;
                }
                break;
            
            case SILENT:
                op_params.current_activity = checkAll(&op_params.real_time);
                op_params.stateMachine = (op_params.current_activity != (uint8_t)IDLE) ? PENDING_ACTIVITY : SILENT;
                break;

            case PENDING_ACTIVITY:
                if (dataModel[op_params.current_activity].state == DONE) {
                    op_params.stateMachine = ACTIVITY_COMPLETED;
                }
                else {
                    if (!op_params.printUnDoneMessage) {
                        /* Printing Activity Start Message */
                        printf("%s\n",dataModel[op_params.current_activity].undone_msg);
                        op_params.printUnDoneMessage = true;
                    }
                    else if (!op_params.printReminder && isTimeAboutToEnd(&dataModel[op_params.current_activity].endTime, &op_params.real_time)) {
                        /* Printing Activity Reminder Message */
                        printf("%s\n",dataModel[op_params.current_activity].undone_msg);
                        op_params.printReminder = true;
                    }
                    else if (activityTimeOut(&dataModel[op_params.current_activity].endTime, &op_params.real_time)) {
                        /* Activity is undone but Activity timeout is Activated */
                        op_params.stateMachine = SILENT;
                    }
                    else {
                        /* Default case */
                    }
                }
                break;

            case ACTIVITY_COMPLETED:
                /* Activity is Complete but Activity timeout is still pending */
                if (activityTimeOut(&dataModel[op_params.current_activity].endTime, &op_params.real_time)) {
                    op_params.stateMachine = SILENT;
                }
                break;

            default : 
                break;
        }

        /* For INIT state, waiting for user to enter current_time */
        if (op_params.stateMachine != INIT) {
            
            /* Updating Time */
            updateTime(&op_params.real_time, op_params.exe_speed);

            /* Using Linux APIs for handling commands on runtime */
            handle_non_blocking_input(&op_params.user_command);

            /* Parsing user Input command on runtime */
            if (op_params.user_command != CMD_NONE) {
                if (op_params.user_command == CMD_DONE) {
                    dataModel[op_params.current_activity].state = DONE;
                } 
                else if (op_params.user_command == CMD_NOW) {
                    if (dataModel[op_params.current_activity].toDo == IDLE) {
                        if (userDelay(&op_params.timeCounter, &op_params.user_command)) {
                            printf("Nothing to do right now\n");
                        }
                    } 
                    else {
                        if (dataModel[op_params.current_activity].state == DONE) {
                            if (userDelay(&op_params.timeCounter, &op_params.user_command)) {
                                printf("%s\n",dataModel[op_params.current_activity].done_msg);
                            }
                        } else {
                            if (userDelay(&op_params.timeCounter, &op_params.user_command)) {
                                printf("%s\n",dataModel[op_params.current_activity].undone_msg);
                            }
                        }
                    }
                } 
                else {
                    /* Do Nothing - Default Case */
                }
            }
        }
    }
    return 0;
}
