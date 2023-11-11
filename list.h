#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

extern FILE *Global_logs_pointer;
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
const ssize_t  INITIAL_CAPACITY_VALUE_LIST  = 2;
const int      POISON_LIST                  = 192;

enum errors_code_list {
    LIST_NO_ERROR                           = 0,
    POINTER_TO_LIST_IS_NULL                 = 1,
    POINTER_TO_LIST_DATA_IS_NULL            = 1 <<  1,
    POINTER_TO_LIST_NEXT_IS_NULL            = 1 <<  2,
    POINTER_TO_LIST_PREV_IS_NULL            = 1 <<  3,
    POINTER_TO_LIST_INFO_IS_NULL            = 1 <<  4,
    ATTEMPT_TO_DELETE_UNAVAILABLE_ELEM      = 1 <<  5
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
    ssize_t                  number_graph; //TODO: Move to graph graph
    struct debug_info_list  *info;
};

list *new_pointer_list();

ssize_t list_constructor(list *list_pointer, debug_info_list *info);
ssize_t list_destructor (list *list_pointer);
ssize_t push_front(list *list_pointer, TYPE_ELEMENT_LIST value);
ssize_t push_back(list *list_pointer, TYPE_ELEMENT_LIST value);
ssize_t insert_after(list *list_pointer, ssize_t anchor_elem_index, TYPE_ELEMENT_LIST value);
TYPE_ELEMENT_LIST erase(list *list_pointer, ssize_t position);

#endif //LIST_H_INCLUDED
