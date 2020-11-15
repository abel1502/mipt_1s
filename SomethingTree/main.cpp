#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"


using namespace SomethingTree;  // I may do this, right?


int main(int argc, char **argv) {
    char dedStr[] = "Ded";
    char notDedStr[] = "IDK, not Ded I guess...";
    char dedQStr[] = "is Ded";

    ValueDTN ded(dedStr);
    ValueDTN notDed(notDedStr);

    ChoiceDTN root(dedQStr, &ded, &notDed);

    ConsoleDecisionProvider goal;

    ValueDTN *ans = root.find(&goal);

    printf("%p\n", ans);

    return 0;
}
