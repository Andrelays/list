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
static ssize_t insert_first_element(list *list_pointer, TYPE_ELEMENT_LIST value);
static ssize_t check_capacity(list *list_pointer);
static ssize_t fill_data_new_free(list *list_pointer, ssize_t index_last_element_list);



IF_ON_LIST_DUMP
(
    static void list_dump       (list *list_pointer, ssize_t line, const char *file, const char *func);
    static void print_errors    (const list *list_pointer);
    static void print_debug_info(const list *list_pointer, ssize_t line, const char *file, const char *func);
    static void generate_graph_of_list(list *list_pointer);
    static void generate_image (const list *list_pointer, const char *name_dot_file);
)

IF_ON_LIST_OK (static void list_ok  (const list *list_pointer));

#define CHECK_ERRORS(list)                                          \
do {                                                                \
    if (((list)->error_code = verify_list(list)) != LIST_NO_ERROR)  \
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

    fill_data_new_free(list_pointer, list_pointer->free);

    list_pointer->data[0] = POISON_LIST;

    return (verify_list(list_pointer));
}

ssize_t list_destructor(list *list_pointer)
{
    return 0;
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

    if (list_pointer->head == 0)
    {
        list_pointer->error_code = insert_first_element(list_pointer, value);
        return list_pointer->head;
    }

    if (anchor_elem_index == list_pointer->tail)
    {
        push_back(list_pointer, value);
        return list_pointer->tail;
    }

    if (anchor_elem_index == 0)
    {
        push_front(list_pointer, value);
        return list_pointer->head;
    }

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

    check_capacity(list_pointer);

    if (list_pointer->head == 0)
    {
        list_pointer->error_code = insert_first_element(list_pointer, value);
        return list_pointer->error_code;
    }

    ssize_t element_index = list_pointer->free;
    list_pointer->free = list_pointer->next[list_pointer->free];

    list_pointer->data[element_index] = value;
    list_pointer->next[element_index] = list_pointer->head;
    list_pointer->prev[element_index] = 0;

    list_pointer->prev[list_pointer->head] = element_index;

    list_pointer->head = element_index;

    CHECK_ERRORS(list_pointer);

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

    check_capacity(list_pointer);

    if (list_pointer->head == 0)
    {
        list_pointer->error_code = insert_first_element(list_pointer, value);
        return list_pointer->error_code;
    }

    ssize_t element_index = list_pointer->free;
    list_pointer->free = list_pointer->next[list_pointer->free];

    list_pointer->data[element_index] = value;
    list_pointer->next[element_index] = 0;
    list_pointer->prev[element_index] = list_pointer->tail;

    list_pointer->next[list_pointer->tail] = element_index;

    list_pointer->tail = element_index;

    CHECK_ERRORS(list_pointer);

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

    list_pointer->head = list_pointer->free;
    list_pointer->tail = list_pointer->head;

    list_pointer->free = list_pointer->next[list_pointer->free];

    list_pointer->data[list_pointer->head] = value;
    list_pointer->next[list_pointer->head] = 0;
    list_pointer->prev[list_pointer->head] = 0;

    return (verify_list(list_pointer));
}

TYPE_ELEMENT_LIST erase(list *list_pointer, ssize_t position)
{
    MYASSERT(list_pointer          != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->next    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->prev    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(list_pointer->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(position > 0,                  NEGATIVE_VALUE_SIZE_T,       return 0);

    CHECK_ERRORS(list_pointer);

    if (position == list_pointer->head) {
        list_pointer->head = list_pointer->next[position];
    }

    if (position == list_pointer->tail) {
        list_pointer->tail = list_pointer->prev[position];
    }

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

    fill_data_new_free(list_pointer, index_last_element_list);

    return (verify_list(list_pointer));
}

ssize_t fill_data_new_free(list *list_pointer, ssize_t index_last_element_list)
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

        print_errors(list_pointer);

        print_debug_info(list_pointer, line, file, func);

        generate_graph_of_list(list_pointer);
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
        COLOR_PRINT(Orange, "%ld\n", list_pointer->head);

        fprintf(Global_logs_pointer, "tail = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->tail);

        fprintf(Global_logs_pointer, "free = ");
        COLOR_PRINT(Crimson, "%ld\n", list_pointer->free);

        fprintf(Global_logs_pointer, "capacity = ");
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

IF_ON_LIST_DUMP
(
    static void generate_graph_of_list(list *list_pointer)
    {
        MYASSERT(list_pointer                           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        const char  *NAME_DOT_FILE   = "list.dot";

        ++list_pointer->number_graph;

        FILE *dot_file = check_isopen_old(NAME_DOT_FILE, "w");

        fprintf(dot_file,  "digraph List {\n"
                                                "\trankdir = LR;\n"
	                                            "\tnode [shape = record];\n"
                                                "\tsplines=ortho;\n"
                                                "\t0");

        for (ssize_t index_list = 1; index_list < list_pointer->capacity; index_list++) {
            fprintf(dot_file, " -> %ld", index_list);
        }

        fprintf(dot_file,  "\t[arrowsize = 0.0, weight = 10000, color = \"#FFFFFF\"];\n"
                                     "\tsubgraph cluster0 {\n");

        for (ssize_t index_list = 0; index_list < list_pointer->capacity; index_list++) {
            if (list_pointer->prev[index_list] == -1) {
                fprintf(dot_file,  "\t\t%-3ld  [shape = Mrecord, style = filled, fillcolor = red, label = \"idx: %ld | data:" FORMAT_SPECIFIERS_LIST "| next: %ld | prev: %ld\"];\n",
                        index_list, index_list, list_pointer->data[index_list], list_pointer->next[index_list], list_pointer->prev[index_list]);
            }

            else {
                fprintf(dot_file,  "\t\t%-3ld  [shape = Mrecord, style = filled, fillcolor = cyan, label = \"idx: %ld | data:" FORMAT_SPECIFIERS_LIST "| next: %ld | prev: %ld\"];\n",
                        index_list, index_list, list_pointer->data[index_list], list_pointer->next[index_list], list_pointer->prev[index_list]);
            }
        }

        for (ssize_t index_list = 0; index_list < list_pointer->capacity; index_list++) {
            fprintf(dot_file,  "\n\t\t%-3ld -> %-3ld", index_list, list_pointer->next[index_list]);
        }

        fprintf(dot_file, "\n\n\t\tAll[shape = Mrecord, label = \"size = %ld | head = %ld | tail = %ld | free = %ld | capacity = %ld\"];\n"
                                    "\t}\n"
                                    "}\n",
                list_pointer->capacity, list_pointer->head, list_pointer->tail, list_pointer->free, list_pointer->capacity);

        MYASSERT(check_isclose_old(dot_file), COULD_NOT_CLOSE_THE_FILE , return);

        generate_image (list_pointer, NAME_DOT_FILE);

        fprintf(Global_logs_pointer, "<img src = graph/list_%ld.png width = 25%%>\n\n", list_pointer->number_graph);
    }
)

IF_ON_LIST_DUMP
(
    static void generate_image (const list *list_pointer, const char *name_dot_file)
    {
        MYASSERT(list_pointer                           != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->data                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->next                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->prev                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(list_pointer->info                     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        const size_t SIZE_NAME_GRAPH = 50;

        char image_generation_command[SIZE_NAME_GRAPH] = "";

        snprintf(image_generation_command, SIZE_NAME_GRAPH, "dot %s -T png -o graph/list_%ld.png", name_dot_file, list_pointer->number_graph);

        MYASSERT(!system(image_generation_command), SYSTEM_ERROR, return);
    }
)
