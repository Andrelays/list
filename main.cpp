#include "libraries/Stack/myassert.h"
#include "libraries/Stack/stack.h"
#include "libraries/Onegin/onegin.h"
#include "akinator.h"

int main(int argc, const char *argv[])
{
    if(!check_argc(argc, 2)) {
        return -1;
    }

    const char *file_name_logs = argv[1];

    Global_logs_pointer = check_isopen(file_name_logs, "w");

    MYASSERT(Global_logs_pointer != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);

    tree *tree_pointer = new_pointer_tree();

    TREE_CONSTRUCTOR(tree_pointer);

    tree_destructor(tree_pointer);

    MYASSERT(check_isclose (Global_logs_pointer), COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);

    return 0;
}
