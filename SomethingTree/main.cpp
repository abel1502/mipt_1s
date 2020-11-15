#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"


using namespace SomethingTree;  // I may do this, right?


int main(int argc, char **argv) {
    char dedStr[] = "Ded";
    char notDedStr[] = "IDK, not Ded I guess...";
    char dedQStr[] = "likes cats";

    ValueDTN ded(dedStr);
    ValueDTN notDed(notDedStr);

    ChoiceDTN root(dedQStr, &ded, &notDed);

    ValueDTN *ans = root.find();

    if (ans != NULL) {
        printf("It's %s\n", ans->getValue());
    } else {
        printf("Wow...\n");
    }

    return 0;
}
