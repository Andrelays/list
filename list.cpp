#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libraries/Stack/myassert.h"
#include "libraries/Stack/colors.h"
#include "libraries/Onegin/onegin.h"
#include "list.h"

FILE *Global_logs_pointer  = stderr;
bool  Global_color_output  = true;

#ifdef DEBUG_OUTPUT_LIST_DUMP

    #define IF_ON_LIST_DUMP(...)   __VA_ARGS__

#else

    #define IF_ON_LIST_DUMP(...)

#endif

#ifdef DEBUG_OUTPUT_LIST_OK

    #define IF_ON_LIST_OK(...)     __VA_ARGS__

#else

    #define IF_ON_LIST_OK(...)

#endif

static void    pour_poison_into_list    (list *list_pointer);
static ssize_t verify_list              (list *list_pointer, ssize_t line, const char *file, const char *func);
static ssize_t insert_first_element     (list *list_pointer, TYPE_ELEMENT_LIST value);
static ssize_t check_capacity           (list *list_pointer);
static ssize_t prepare_new_free_elements(list *list_pointer, ssize_t index_last_element_list);
static bool    check_is_not_loop        (list *list_pointer, ssize_t index_first_element);
static bool check_prev_from_next(list *list_pointer);


IF_ON_LIST_DUMP
(
    static void list_dump               (list *list_pointer, ssize_t line, const char *file, const char *func);
    static void print_errors            (const list *list_pointer);
    static void print_debug_info        (const list *list_pointer, ssize_t line, const char *file, const char *func);
    static void generate_graph_of_list  (list *list_pointer, ssize_t line, const char *file, const char *func);
    static void write_log_to_dot(const list *list_pointer, FILE *dot_file, ssize_t line, const char *file, const char *func);
    static void generate_image          (const list *list_pointer, const char *name_dot_file, ssize_t number_graph);
)

IF_ON_LIST_OK (static void list_ok  (const list *list_pointer));

#define VERIFY_LIST(list_pointer) verify_list(list_pointer, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define CHECK_ERRORS(list)                                          \
do {                                                                \
    if (((list)->error_code = VERIFY_LIST(list)) != LIST_NO_ERROR)  \
        return 0;                                                   \
} while(0)

list *new_pointer_list()
{
    struct list *list_pointer = (list *) calloc(1, sizeof(list));

    MYASSERT(list_pointer  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    return list_pointer;
}

ssize_t list_constructor(list *list_pointer, debug_info_list *info)
{
    MYASSERT(list_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_IS_NULL);
    MYASSERT(info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_INFO_IS_NULL);

    list_pointer->info = info;

    list_pointer->capacity = INITIAL_CAPACITY_VALUE_LIST;

    MYASSERT(INITIAL_CAPACITY_VALUE_LIST > 0, NEGATIVE_VALUE_SIZE_T, return 0);

    list_pointer->data  = (TYPE_ELEMENT_LIST *) calloc((size_t) list_pointer->capacity, sizeof(TYPE_ELEMENT_LIST));
    list_pointer->next  = (ssize_t *)           calloc((size_t) list_pointer->capacity, sizeof(ssize_t));
    list_pointer->prev  = (ssize_t *)           calloc((size_t) list_pointer->capacity, sizeof(ssize_t));
    //FIXME: Memleak
    MYASSERT(list_pointer->data  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_PREV_IS_NULL);

    list_pointer->free = 1;

    prepare_new_free_elements(list_pointer, list_pointer->free);

    list_pointer->data[0] = POISON_LIST;

    return (VERIFY_LIST(list_pointer));
}

ssize_t list_destructor(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_IS_NULL);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_PREV_IS_NULL);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_INFO_IS_NULL);

    CHECK_ERRORS(list_pointer);

    pour_poison_into_list(list_pointer);

    free(list_pointer->data);
    free(list_pointer->next);
    free(list_pointer->prev);
    free(list_pointer->info);

    list_pointer->data = NULL;
    list_pointer->next = NULL;
    list_pointer->prev = NULL;
    list_pointer->info = NULL;

    free(list_pointer);
    list_pointer = NULL;

    return LIST_NO_ERROR;
}

void pour_poison_into_list(list *list_pointer)
{
    for (ssize_t index_list = 0; index_list < list_pointer->capacity; index_list++)
    {
        list_pointer->data[index_list] = POISON_LIST;
        list_pointer->next[index_list] = POISON_LIST;
        list_pointer->prev[index_list] = POISON_LIST;
    }

    list_pointer->free       = POISON_LIST;
    list_pointer->capacity   = POISON_LIST;

    list_pointer->info->line = POISON_LIST;
    list_pointer->info->func = "POISON";
    list_pointer->info->file = "POISON";
    list_pointer->info->name = "POISON";
}

ssize_t insert_after(list *list_pointer, ssize_t anchor_elem_index, TYPE_ELEMENT_LIST value)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(list_pointer);

    if (anchor_elem_index >= list_pointer->capacity || list_pointer->prev[anchor_elem_index] == -1) {
        return 0;
    }

    check_capacity(list_pointer);

    ssize_t element_index = list_pointer->free;
    list_pointer->free = list_pointer->next[list_pointer->free];

    list_pointer->data[element_index] = value;
    list_pointer->next[element_index] = list_pointer->next[anchor_elem_index];
    list_pointer->prev[element_index] = anchor_elem_index;

    list_pointer->next[anchor_elem_index] = element_index;

    list_pointer->prev[list_pointer->next[element_index]] = element_index;

    CHECK_ERRORS(list_pointer);

    return element_index;
}

ssize_t push_front(list *list_pointer, TYPE_ELEMENT_LIST value)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(list_pointer);

    ssize_t element_index = insert_after(list_pointer, 0, value);

    return element_index;
}

ssize_t push_back(list *list_pointer, TYPE_ELEMENT_LIST value)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(list_pointer);

    ssize_t element_index = insert_after(list_pointer, list_pointer->prev[0], value);

    return element_index;
}

ssize_t insert_first_element(list *list_pointer, TYPE_ELEMENT_LIST value)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_IS_NULL);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_PREV_IS_NULL);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_INFO_IS_NULL);

    CHECK_ERRORS(list_pointer);

    list_pointer->next[0] = list_pointer->free;
    list_pointer->prev[0] = list_pointer->next[0];

    list_pointer->free = list_pointer->next[list_pointer->free];

    list_pointer->data[list_pointer->next[0]] = value;
    list_pointer->next[list_pointer->next[0]] = 0;
    list_pointer->prev[list_pointer->next[0]] = 0;

    return (VERIFY_LIST(list_pointer));
}

TYPE_ELEMENT_LIST erase(list *list_pointer, ssize_t position)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(position > 0,                  NEGATIVE_VALUE_SIZE_T,       return POISON_LIST);

    CHECK_ERRORS(list_pointer);

    TYPE_ELEMENT_LIST return_value = list_pointer->data[position];
    list_pointer->next[list_pointer->prev[position]] = list_pointer->next[position];
    list_pointer->prev[list_pointer->next[position]] = list_pointer->prev[position];

    list_pointer->data[position] = POISON_LIST;
    list_pointer->prev[position] = -1;
    list_pointer->next[position] = list_pointer->free;

    list_pointer->free = position;

    CHECK_ERRORS(list_pointer);

    return return_value;
}

TYPE_ELEMENT_LIST pop_back(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POISON_LIST);

    CHECK_ERRORS(list_pointer);

    TYPE_ELEMENT_LIST return_value = erase(list_pointer, list_pointer->prev[0]);

    return return_value;
}

TYPE_ELEMENT_LIST pop_front(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(list_pointer);

    TYPE_ELEMENT_LIST return_value = erase(list_pointer, list_pointer->next[0]);

    return return_value;
}

ssize_t clear(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(list_pointer);

    prepare_new_free_elements(list_pointer, 0);

    list_pointer->free    = 1;
    list_pointer->prev[0] = 0;
    list_pointer->next[0] = 0;

    return (VERIFY_LIST(list_pointer));
}

ssize_t find_elem_by_number(list *list_pointer, ssize_t number_target_element_list)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(list_pointer);

    ssize_t index_element_list = list_pointer->next[0];

    for (ssize_t number_element_list = 0; number_element_list < number_target_element_list; number_element_list++)
    {
        index_element_list = list_pointer->next[index_element_list];
    }

    return index_element_list;
}

ssize_t check_capacity(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_IS_NULL);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_PREV_IS_NULL);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_INFO_IS_NULL);

    if (list_pointer->free != 0) {
        return list_pointer->error_code;
    }

    CHECK_ERRORS(list_pointer);

    ssize_t index_last_element_list = list_pointer->capacity - 1;

    list_pointer->capacity *= CAPACITY_MULTIPLIER_LIST;

    list_pointer->data  = (TYPE_ELEMENT_LIST *) realloc(list_pointer->data, (size_t) list_pointer->capacity * sizeof(TYPE_ELEMENT_LIST));
    list_pointer->next  = (ssize_t *)           realloc(list_pointer->next, (size_t) list_pointer->capacity * sizeof(ssize_t));
    list_pointer->prev  = (ssize_t *)           realloc(list_pointer->prev, (size_t) list_pointer->capacity * sizeof(ssize_t));
    //FIXME:
    MYASSERT(list_pointer->data  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev  != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_LIST_PREV_IS_NULL);

    list_pointer->free = index_last_element_list + 1;

    prepare_new_free_elements(list_pointer, index_last_element_list);

    return (VERIFY_LIST(list_pointer));
}

ssize_t prepare_new_free_elements(list *list_pointer, ssize_t index_last_element_list)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_IS_NULL);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_DATA_IS_NULL);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_NEXT_IS_NULL);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_PREV_IS_NULL);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_LIST_INFO_IS_NULL);

    for (ssize_t index_free_element = index_last_element_list + 1; index_free_element < list_pointer->capacity - 1; index_free_element++)
    {
        list_pointer->data[index_free_element] = POISON_LIST;
        list_pointer->next[index_free_element] = index_free_element + 1;
        list_pointer->prev[index_free_element] = -1;
    }

    list_pointer->data[list_pointer->capacity - 1] = POISON_LIST;
    list_pointer->next[list_pointer->capacity - 1] = 0;
    list_pointer->prev[list_pointer->capacity - 1] = -1;

    return LIST_NO_ERROR;
}

ssize_t verify_list(list *list_pointer, ssize_t line, const char *file, const char *func)
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

    SUMMARIZE_ERRORS_(!check_is_not_loop(list_pointer, list_pointer->free),                         LOOP_IN_FREE_ELEMENTS);
    SUMMARIZE_ERRORS_(!check_is_not_loop(list_pointer, list_pointer->next[0]),                      LOOP_IN_OCCUPIED_ELEMENTS);
    SUMMARIZE_ERRORS_(!check_prev_from_next(list_pointer),                                          PREV_FROM_NEXT_NOT_EQUAL_ELEMENT_INDEX);
    SUMMARIZE_ERRORS_(list_pointer->capacity                                              < 0,      CAPACITY_LESS_THAN_ZERO_LIST);
    SUMMARIZE_ERRORS_(list_pointer->free                                                  < 0,      FREE_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(list_pointer->next[0]                                               < 0,      HEAD_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(list_pointer->prev[0]                                               < 0,      TAIL_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(list_pointer->prev[list_pointer->free] != -1 && list_pointer->free != 0,      FREE_POINT_TO_OCCUPIED_ELEMENT);
    SUMMARIZE_ERRORS_(list_pointer->prev[list_pointer->prev[0]]                         == -1,      TAIL_POINT_TO_FREE_ELEMENT);
    SUMMARIZE_ERRORS_(list_pointer->prev[list_pointer->next[0]]                         == -1,      HEAD_POINT_TO_FREE_ELEMENT);

    #undef SUMMARIZE_ERRORS_

    list_pointer->error_code = error_code;

    IF_ON_LIST_DUMP
    (
        if (error_code != LIST_NO_ERROR)
            list_dump(list_pointer, line, file, func);
    )

    list_dump(list_pointer, line, file, func);

    IF_ON_LIST_OK
    (
        if (error_code == LIST_NO_ERROR)
            list_ok(list_pointer);
    )

    return list_pointer->error_code;
}

bool check_is_not_loop(list *list_pointer, ssize_t index_first_element)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    ssize_t slow_index_counter = index_first_element;
    ssize_t fast_index_counter = index_first_element;

    while (slow_index_counter != 0 && fast_index_counter != 0)
    {
        // printf("slow_index_counter = %ld fast_index_counter = %ld\n", slow_index_counter, fast_index_counter);

        slow_index_counter = list_pointer->next[slow_index_counter];
        fast_index_counter = list_pointer->next[fast_index_counter];
        fast_index_counter = list_pointer->next[fast_index_counter];

        // printf("slow_index_counter = %ld fast_index_counter = %ld\n", slow_index_counter, fast_index_counter);

        if (fast_index_counter == slow_index_counter && fast_index_counter != 0) {
            return false;
        }

    }

    return true;
}

bool check_prev_from_next(list *list_pointer)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    for (ssize_t index_element_list = 0; index_element_list < list_pointer->capacity; index_element_list++)
    {
        if (list_pointer->prev[index_element_list] == -1) {
            continue;
        }

        if (index_element_list != list_pointer->prev[list_pointer->next[index_element_list]]) {
            return false;
        }
    }

    return true;
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

        print_errors(list_pointer);

        print_debug_info(list_pointer, line, file, func);

        generate_graph_of_list(list_pointer, line, file, func);
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

        COLOR_PRINT(MediumBlue, "list[%p]\n", list_pointer);

        COLOR_PRINT(BlueViolet, "\"%s\"from %s(%ld) %s\n", list_pointer->info->name, list_pointer->info->file, list_pointer->info->line, list_pointer->info->func);

        COLOR_PRINT(DarkMagenta, "called from %s(%ld) %s\n", file, line, func);

        fprintf(Global_logs_pointer,  "\nhead = ");
        COLOR_PRINT(Orange, "%ld\n", list_pointer->next[0]);

        fprintf(Global_logs_pointer, "tail = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->prev[0]);

        fprintf(Global_logs_pointer, "free = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->free);

        fprintf(Global_logs_pointer, "capacity = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->capacity);
    }
)

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

    GET_ERRORS_(CAPACITY_LESS_THAN_ZERO_LIST);
    GET_ERRORS_(LOOP_IN_FREE_ELEMENTS);
    GET_ERRORS_(LOOP_IN_OCCUPIED_ELEMENTS);
    GET_ERRORS_(FREE_LESS_THAN_ZERO);
    GET_ERRORS_(HEAD_LESS_THAN_ZERO);
    GET_ERRORS_(TAIL_LESS_THAN_ZERO);
    GET_ERRORS_(FREE_POINT_TO_OCCUPIED_ELEMENT);
    GET_ERRORS_(PREV_FROM_NEXT_NOT_EQUAL_ELEMENT_INDEX);
    GET_ERRORS_(HEAD_POINT_TO_FREE_ELEMENT);
    GET_ERRORS_(TAIL_POINT_TO_FREE_ELEMENT);

    #undef GET_ERRORS_
}

IF_ON_LIST_DUMP
(
    static void generate_graph_of_list(list *list_pointer, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(list_pointer                           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                                   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                                   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        const char  *NAME_DOT_FILE   = "list.dot";
        static ssize_t number_graph = 0;

        ++number_graph;

        FILE *dot_file = check_isopen_old(NAME_DOT_FILE, "w");

        write_log_to_dot(list_pointer, dot_file, line, file, func);

        MYASSERT(check_isclose_old(dot_file), COULD_NOT_CLOSE_THE_FILE , return);

        generate_image(list_pointer, NAME_DOT_FILE, number_graph);

        fprintf(Global_logs_pointer, "<img src = graph/list_%ld.png height= 25%% width = 25%%>\n\n", number_graph);
    }
)

IF_ON_LIST_DUMP
(
    static void write_log_to_dot(const list *list_pointer, FILE *dot_file, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(list_pointer                           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(dot_file                               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file                                   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(func                                   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        fprintf(dot_file,   "digraph List {\n"
                            "\trankdir = LR;\n"
	                        "\tnode [shape = record];\n"
                            "\tsplines=ortho;\n"
                            "\t0");

        for (ssize_t index_list = 1; index_list < list_pointer->capacity; index_list++) {
            fprintf(dot_file, " -> %ld", index_list);
        }

        fprintf(dot_file,   "\t[arrowsize = 0.0, weight = 10000, color = \"#FFFFFF\"];\n"
                            "\tsubgraph cluster0 {\n"
                            "\t\tlabel = \"called  from:    %s(%ld)  %s\";\n", file, line, func);

        for (ssize_t index_list = 0; index_list < list_pointer->capacity; index_list++) {
            if (list_pointer->prev[index_list] == -1) {
                fprintf(dot_file,  "\t\t%-3ld  [shape = Mrecord, style = filled, fillcolor = \"#BC5D58\", label = \"idx: %ld | data:" FORMAT_SPECIFIERS_LIST "| next: %ld | prev: %ld\"];\n",
                        index_list, index_list, list_pointer->data[index_list], list_pointer->next[index_list], list_pointer->prev[index_list]);
            }

            else {
                fprintf(dot_file,  "\t\t%-3ld  [shape = Mrecord, style = filled, fillcolor = \"#9ACEEB\", label = \"idx: %ld | data:" FORMAT_SPECIFIERS_LIST "| next: %ld | prev: %ld\"];\n",
                        index_list, index_list, list_pointer->data[index_list], list_pointer->next[index_list], list_pointer->prev[index_list]);
            }
        }

        for (ssize_t index_list = 0; index_list < list_pointer->capacity; index_list++) {
            fprintf(dot_file,  "\n\t\t%-3ld -> %-3ld \t[color = blue];", index_list, list_pointer->next[index_list]);
        }

        fprintf(dot_file,   "\n\n\t\tInfo[shape = Mrecord, label = \"capacity = %ld | head = %ld | tail = %ld | free = %ld\"];\n"
                            "\t}\n"
                            "}\n",
                list_pointer->capacity, list_pointer->next[0], list_pointer->prev[0], list_pointer->free);
    }
)

IF_ON_LIST_DUMP
(
    static void generate_image(const list *list_pointer, const char *name_dot_file, ssize_t number_graph)
    {
        MYASSERT(list_pointer                           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(name_dot_file                          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);


        const size_t SIZE_NAME_GRAPH = 50;

        char image_generation_command[SIZE_NAME_GRAPH] = "";

        snprintf(image_generation_command, SIZE_NAME_GRAPH, "dot %s -T png -o graph/list_%ld.png", name_dot_file, number_graph);

        MYASSERT(!system(image_generation_command), SYSTEM_ERROR, return);
    }
)
