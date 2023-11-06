#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

extern FILE *Global_logs_pointer;
extern FILE *Global_dot_pointer;

extern bool  Global_color_output;

#define LIST_CONSTRUCTOR(list_pointer)                                                          \
do {                                                                                            \
    struct debug_info_list *info = (debug_info_list *) calloc(1, sizeof(debug_info_list));      \
                                                                                                \
    info->line = __LINE__;                                                                      \
    info->name = #list_pointer;                                                                 \
    info->file = __FILE__;                                                                      \
    info->func = __PRETTY_FUNCTION__;                                                           \
                                                                                                \
    list_constructor(list_pointer, info);                                                       \
} while(0)

#define FORMAT_SPECIFIERS_LIST   "%d"
typedef int TYPE_ELEMENT_LIST;

const ssize_t  CAPACITY_MULTIPLIER_LIST     = 2;
const ssize_t  INITIAL_CAPACITY_VALUE_LIST  = 10;
const int      POISON_LIST                  = 192;

enum errors_code_list {
    LIST_NO_ERROR                   = 0,
    POINTER_TO_LIST_IS_NULL         = 1,
    POINTER_TO_LIST_DATA_IS_NULL    = 1 <<  1,
    POINTER_TO_LIST_NEXT_IS_NULL    = 1 <<  2,
    POINTER_TO_LIST_PREV_IS_NULL    = 1 <<  3,
    POINTER_TO_LIST_INFO_IS_NULL    = 1 <<  4


};

struct debug_info_list {
    ssize_t      line;
    const char  *name;
    const char  *file;
    const char  *func;
};

struct list {
    TYPE_ELEMENT_LIST       *data;
    long int                *next;
    long int                *prev;
    ssize_t                  head;
    ssize_t                  tail;
    ssize_t                  free;
    ssize_t                  capacity;
    ssize_t                  error_code;
    struct debug_info_list  *info;
};

list *get_pointer_list();

ssize_t list_constructor(list *list_pointer, debug_info_list *info);
ssize_t list_destructor (list *list_pointer);

#endif //LIST_H_INCLUDED
