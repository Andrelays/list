#include <stdio.h>
#include <stdlib.h>
#include "libraries/Stack/myassert.h"
#include "libraries/Stack/colors.h"
#include "list.h"

FILE *Global_logs_pointer = stderr;
FILE *Global_dot_pointer  = NULL;

bool Global_color_output  = true;

#ifdef DEBUG_OUTPUT_LIST_DUMP

    #define IF_ON_LIST_DUMP(...)   __VA_ARGS__

    #define LIST_DUMP(list_pointer)                                         \
    do {                                                                    \
        list_dump(list_pointer, __LINE__, __FILE__, __PRETTY_FUNCTION__);   \
    } while(0)

#else

    #define IF_ON_LIST_DUMP(...)

#endif

#ifdef DEBUG_OUTPUT_LIST_OK

    #define IF_ON_LIST_OK(...)     __VA_ARGS__

#else

    #define IF_ON_LIST_OK(...)

#endif

static ssize_t verify_list(list *list_pointer);

IF_ON_LIST_DUMP
(
    static void list_dump       (list *list_pointer, ssize_t line, const char *file, const char *func);
    static void print_errors    (const list *list_pointer);
    static void print_debug_info(const list *list_pointer, ssize_t line, const char *file, const char *func);
)

IF_ON_LIST_OK (static void list_ok  (const list *list_pointer));

list *get_pointer_list()
{
    struct list *list_pointer = (list *) calloc(1, sizeof(list));

    list_pointer->data           = NULL;
    list_pointer->next           = NULL;
    list_pointer->prev           = NULL;
    list_pointer->info           = NULL;
    list_pointer->capacity       = 0;
    list_pointer->error_code     = LIST_NO_ERROR;

    return list_pointer;
}

ssize_t list_constructor(list *list_pointer, debug_info_list *info)
{
    MYASSERT(list_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);         //TODO error_code
    MYASSERT(info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    list_pointer->info = info;

    list_pointer->capacity = INITIAL_CAPACITY_VALUE_LIST;

    MYASSERT(INITIAL_CAPACITY_VALUE_LIST > 0, NEGATIVE_VALUE_SIZE_T, return 0);

    list_pointer->data = (TYPE_ELEMENT_LIST *) calloc((size_t) list_pointer->capacity, sizeof(TYPE_ELEMENT_LIST));
    list_pointer->next = (ssize_t *)           calloc((size_t) list_pointer->capacity, sizeof(ssize_t));
    list_pointer->prev = (ssize_t *)           calloc((size_t) list_pointer->capacity, sizeof(ssize_t));

    MYASSERT(list_pointer->data != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_PREV_IS_NULL);

    list_pointer->free = 1;

    return (verify_list(list_pointer));
}

ssize_t list_destructor(list *list_pointer) //TODO static
{
    return 0;
}

ssize_t verify_list(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_IS_NULL);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_PREV_IS_NULL);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_INFO_IS_NULL);

    ssize_t error_code = LIST_NO_ERROR;

    #define SUMMARIZE_ERRORS_(condition, added_error)   \
    do {                                                \
        if((condition))                                 \
            error_code += added_error;                  \
    } while(0)

    // SUMMARIZE_ERRORS_(stk->size > stk->capacity, SIZE_MORE_THAN_CAPACITY);
    // SUMMARIZE_ERRORS_(stk->capacity < 0,         CAPACITY_LESS_THAN_ZERO);
    // SUMMARIZE_ERRORS_(stk->size     < 0,         SIZE_LESS_THAN_ZERO);

    #undef SUMMARIZE_ERRORS_

    list_pointer->error_code = error_code;

    IF_ON_LIST_DUMP
    (
        if (error_code != LIST_NO_ERROR)
            LIST_DUMP(list_pointer);
    )

    LIST_DUMP(list_pointer);

    IF_ON_LIST_OK
    (
        if (error_code == LIST_NO_ERROR)
            list_ok(list_pointer);
    )

    return list_pointer->error_code;
}

IF_ON_LIST_DUMP
(
    void list_dump(list *list_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        print_errors(list_pointer);

        print_debug_info(list_pointer, line, file, func);

        for(ssize_t index_list = 0; index_list < list_pointer->capacity; index_list++)                      //TODO graphic
        {
            fprintf(Global_logs_pointer, "data[%ld] = " FORMAT_SPECIFIERS_LIST " next[%ld] = %ld  prev[%ld] = %ld\n", index_list, list_pointer->data[index_list],
                                                                                                                      index_list, list_pointer->next[index_list],
                                                                                                                      index_list, list_pointer->prev[index_list]);
        }

        fprintf(Global_dot_pointer,  "digraph List {\n"
                                     "rankdir = LR;\n"
	                                 "node [shape = record];\n");


        MYASSERT(!system("dot grath.dot -T png -o grath.png"), SYSTEM_ERROR, return);
    }
)

IF_ON_LIST_DUMP
(
    void print_debug_info(const list *list_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        COLOR_PRINT(MediumBlue, "list[%p]\n", list_pointer);

        COLOR_PRINT(BlueViolet, "\"%s\"from %s(%ld) %s\n", list_pointer->info->name, list_pointer->info->file, list_pointer->info->line, list_pointer->info->func);

        COLOR_PRINT(DarkMagenta, "called from %s(%ld) %s\n", file, line, func);

        fprintf(Global_logs_pointer, "{\n");

        fprintf(Global_logs_pointer,  "\n\thead = ");
        COLOR_PRINT(Orange, "%ld\n", list_pointer->head);

        fprintf(Global_logs_pointer, "\ttail = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->tail);

        fprintf(Global_logs_pointer, "\tfree = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->free);

        fprintf(Global_logs_pointer, "\tcapacity = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->capacity);
    }
)

IF_ON_LIST_DUMP
(
    void print_errors(const list *list_pointer)
    {
        MYASSERT(list_pointer           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(Global_logs_pointer    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        #define GET_ERRORS_(error)                                                           \
        do {                                                                                 \
            if(list_pointer->error_code & error)                                             \
                COLOR_PRINT(Red, "Errors: %s\n", #error);                                    \
            } while(0)

        // GET_ERRORS_(CAPACITY_LESS_THAN_ZERO);
        // GET_ERRORS_(SIZE_LESS_THAN_ZERO);
        // GET_ERRORS_(SIZE_NULL_IN_POP);

        //#undef GET_ERRORS_
    }
)
