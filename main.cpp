#include "libraries/Stack/myassert.h"
#include "libraries/Stack/stack.h"
#include "libraries/Onegin/onegin.h"
#include "list.h"

int main(int argc, const char *argv[])
{
    if(!check_argc(argc, 2)) {
        return -1;
    }

    const char *file_name_logs = argv[1];

    Global_logs_pointer = check_isopen(file_name_logs, "w");

    MYASSERT(Global_logs_pointer != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);

    list *list_pointer = new_pointer_list();

    LIST_CONSTRUCTOR(list_pointer);

    printf("%ld\n", push_back(list_pointer, 10));
    push_back(list_pointer, 20);
    erase(list_pointer, 2);
    push_back(list_pointer, 30);
    push_back(list_pointer, 40);
    push_back(list_pointer, 50);
    printf("%ld\n", insert_after(list_pointer, 5, 75));
    push_front(list_pointer, 50);
    pop_back(list_pointer);
    printf("%ld\n", find_elem_by_number(list_pointer, 2));
    clear(list_pointer);

    list_destructor(list_pointer);

    MYASSERT(check_isclose (Global_logs_pointer), COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);

    return 0;
}
