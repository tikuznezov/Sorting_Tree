#ifndef _LOG
#define _LOG

#include <stdio.h>
#include "term_colors.h"

const size_t MAX_STR_LEN = 100;

void ClearTerm(void);

// WRITE LOG
//---------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
// const char *LOG_FILE_NAME = "LOG.log";
// FILE *const log_file = fopen(LOG_FILE_NAME, "w");

    #define LOG(name, ...)  fprintf(log_file, name, ## __VA_ARGS__);
    #define LOG_FUNC_INFO   LOG("\n[INFO] [%s:%d] (%s) --> started...\n", file_name, line_num, __FUNCTION__)
    #define END_FUNC_LOG    LOG("[INFO] (%s) --> completed.\n\n", __FUNCTION__)
    #define CRUSH_FUNC_LOG  LOG("[WARN] (%s) --> CRUSHED!!!\n\n", __FUNCTION__)
    #define PAUSE ClearTerm();
    #define PR(name, ...)                                                                               \
        printf(name, ## __VA_ARGS__)
#else
    #define LOG(name, ...)
    #define LOG_FUNC_INFO
    #define END_FUNC_LOG
    #define CRUSH_FUNC_LOG
    #define PAUSE
    #define PR(name, ...)
#endif

// PRINTING
//---------------------------------------------------------------------------------------------------------------------

#ifdef _FUNCTION_STATE

#define PR_FST                                                                                      \
    printf("\\/----------------------------------------------------------------------------------\\/\n"); \
    printf(PYELLOW "- RUN %s:\n\n" DEF_COL, __FUNCTION__)
#define PR_FED                                                                                      \
    printf(PCYAN "\n- END %s;" DEF_COL, __FUNCTION__); ClearTerm();                               \
    printf("/\\----------------------------------------------------------------------------------/\\\n")
#else
    #define PR_FST
    #define PR_FED
#endif

//---------------------------------------------------------------------------------------------------------------------

int global_error = 0;

#ifdef _DEBUG
#define NAMED_ASSERT(pointer, err_code, name_of_element)                                                \
    char user_reply[MAX_STR_LEN] = "";                                                                  \
    if (global_error != 0)                                                                              \
        {                                                                                               \
            PR("\n\n--> Assert global error not equal 0.\n");                                           \
            PR("Called function \"%s\".\n", __PRETTY_FUNCTION__);                                       \
            PR(PRED "--> Global error = %d. Do you want to continue? (y/n)\n" DEF_COL, global_error);   \
            scanf("%s", user_reply);                                                                    \
            if (strncmp(user_reply, "yes", 1) == 0)                                                     \
                exit(global_error);                                                                     \
        }                                                                                               \
        if (pointer == NULL)                                                                            \
        {                                                                                               \
            global_error = err_code;                                                                    \
                                                                                                        \
            PR(PRED "\n\n--> \"%s\" get %s as null pointer!\n" DEF_COL, __FUNCTION__, name_of_element); \
            PR("Do you want crush program? (y/n)\n");                                                   \
            scanf("%s", user_reply);                                                                    \
            if (strncmp(user_reply, "yes", 1) == 0)                                                     \
                exit(err_code);                                                                         \
        }
#else
    #define NAMED_ASSERT(pointer, err_code, name_of_element) assert(pointer);
#endif

// clear stdin
void ClearTerm(void)
{
    int symbol = 0;
    while ((symbol != EOF) && (symbol != '\n'))
        {symbol = getchar();}
}

#endif