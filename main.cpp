#include "libraries/Stack/myassert.h"
#include "libraries/Stack/stack.h"
#include "libraries/Onegin/onegin.h"
#include "list.h"

int main(int argc, const char *argv[])
{
    if(!check_argc(argc, 3))
        return -1;

    const char *file_name_logs = argv[1];
    const char *file_name_dot  = argv[2];

    Global_logs_pointer = check_isopen    (file_name_logs, "w");
    Global_dot_pointer  = check_isopen_old(file_name_dot,  "w");

    MYASSERT(Global_logs_pointer != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);
    MYASSERT(Global_dot_pointer  != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);

    list *list_pointer = get_pointer_list();

    LIST_CONSTRUCTOR(list_pointer);

    list_destructor(list_pointer);

    MYASSERT(check_isclose (Global_logs_pointer), COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);
    MYASSERT(check_isclose (Global_dot_pointer),  COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);

    return 0;
}
