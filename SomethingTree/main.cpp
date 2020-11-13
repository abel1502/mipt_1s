#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"


using namespace SomethingTree;  // I may do this, right?


int main(int argc, char **argv) {
    char dedStr[] = "Ded";
    char notDedStr[] = "IDK, not Ded I guess...";
    char dedQStr[] = "is Ded";

    ValueDTN<char *, char *> ded(dedStr);
    ValueDTN<char *, char *>  notDed(notDedStr);

    ChoiceDTN<char *, char *>  root(dedQStr, &ded, &notDed);

    ConsoleDecisionProvider<char *, char *> goal;

    ValueDTN<char *, char *> *ans = root.find(&goal);

    printf("%p\n", ans);

    return 0;
}
