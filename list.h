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
    ATTEMPT_TO_DELETE_UNAVAILABLE_ELEM      = 1 <<  5,
    LOOP_IN_FREE_ELEMENTS                   = 1 <<  6,
    LOOP_IN_OCCUPIED_ELEMENTS               = 1 <<  7,
    CAPACITY_LESS_THAN_ZERO_LIST            = 1 <<  8,
    FREE_LESS_THAN_ZERO                     = 1 <<  9,
    HEAD_LESS_THAN_ZERO                     = 1 << 10,
    TAIL_LESS_THAN_ZERO                     = 1 << 11,
    FREE_POINT_TO_OCCUPIED_ELEMENT          = 1 << 12,
    PREV_FROM_NEXT_NOT_EQUAL_ELEMENT_INDEX =  1 << 13,
    HEAD_POINT_TO_FREE_ELEMENT              = 1 << 14,
    TAIL_POINT_TO_FREE_ELEMENT              = 1 << 15,
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
    ssize_t                  free;
    ssize_t                  capacity;
    ssize_t                  error_code;
    struct debug_info_list  *info;
};

list *new_pointer_list();

ssize_t           list_constructor      (list *list_pointer, debug_info_list *info);
ssize_t           list_destructor       (list *list_pointer);
ssize_t           push_front            (list *list_pointer, TYPE_ELEMENT_LIST value);
ssize_t           push_back             (list *list_pointer, TYPE_ELEMENT_LIST value);
ssize_t           insert_after          (list *list_pointer, ssize_t anchor_elem_index, TYPE_ELEMENT_LIST value);
TYPE_ELEMENT_LIST erase                 (list *list_pointer, ssize_t position);
TYPE_ELEMENT_LIST pop_front             (list *list_pointer);
TYPE_ELEMENT_LIST pop_back              (list *list_pointer);
ssize_t           clear                 (list *list_pointer);
ssize_t           find_elem_by_number   (list *list_pointer, ssize_t number_target_element_list);


#endif //LIST_H_INCLUDED
