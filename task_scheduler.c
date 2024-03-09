#include <stdio.h> 
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "unistd.h"
#include "data_structure.h"
#include "bool.h"

#ifdef DISABLE

enum activites {
    BREAK_FAST              = 0U,
    MORNING_WALK            = 1U,
    LUNCH                   = 2U,
    MEDICATION              = 3U,
    FEEDING_BIRDS           = 4U,
    BRUNCH                  = 5U,
    WATERING_PLANTS         = 6U,
    DINNER                  = 7U,
    TOTAL_ACTIVITIES        = 8U
};

enum status {
    UNDONE, 
    INPROGRESS,
    DONE
};

struct t_time {
    uint8_t hours;
    uint8_t minutes;
};

struct t_model {
    activities const toDo,
    t_time     const startTime,
    t_time     const endTime,
    status           state
    bool             isDone,
};

static t_model dataModel[TOTAL_ACTIVITIES] = {
    /* 0 */     {BREAK_FAST      , /* start time */ {  6U, 30U }, /* end time */ {  8U, 30U }, UNDONE, false},
    /* 1 */     {MORNING_WALK    , /* start time */ {  9U,  0U }, /* end time */ { 10U,  0U }, UNDONE, false}, 
    /* 2 */     {LUNCH           , /* start time */ { 12U, 13U }, /* end time */ { 13U,  0U }, UNDONE, false}, 
    /* 3 */     {MEDICATION      , /* start time */ { 13U, 14U }, /* end time */ { 14U,  0U }, UNDONE, false}, 
    /* 4 */     {FEEDING_BIRDS   , /* start time */ { 15U, 16U }, /* end time */ { 16U,  0U }, UNDONE, false}, 
    /* 5 */     {BRUNCH          , /* start time */ { 16U, 17U }, /* end time */ { 17U,  0U }, UNDONE, false},  
    /* 6 */     {WATERING_PLANTS , /* start time */ { 17U, 18U }, /* end time */ { 18U,  0U }, UNDONE, false}, 
    /* 7 */     {DINNER          , /* start time */ { 18U, 19U }, /* end time */ { 19U,  0U }, UNDONE, false} 
};

#endif

bool isActivityPending(time_t time);

int main(void) {
    uint8_t const delay = 3U;
    time_t real_time_clock; 
    struct tm *info; 
    char buffer[80U];
    state stateMachine = SILENT;

    while(1) {

        time(&real_time_clock);
        info = localtime(&real_time_clock);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", info);
        printf("Current time: %s\n", buffer);
        sleep(delay); 
        
        switch (stateMachine) {
            case SILENT: 
                printf("State - 1\n");
                break;
            case NEW_ACTIVITY: 
                printf("State - 1\n");
                break;
            case STATUS_POLL: 
                printf("State - 1\n");
                break;
            case REMINDER: 
                printf("State - 1\n");
                break;
            default : 
                break;
        }
    }
    return 0;
}