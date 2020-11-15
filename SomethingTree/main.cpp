#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tree.h"


using namespace SomethingTree;  // I may do this, right?


int main(int argc, char **argv) {
    FILE *ifile = fopen("tree.dtf", "rb");

    DecisionTree dt;
    dt.deserialize(ifile);

    ValueDTN *ans = dt.root->find();

    if (ans != NULL) {
        printf("It's %s\n", ans->getValue());
    } else {
        printf("Wow...\n");
    }

    fclose(ifile);

    /*char dedStr[] = "Ded";
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

    FILE *ofile = fopen("tree.dtf", "wb");

    DecisionTree dt;
    dt.root = &root;

    dt.serialize(ofile);

    fclose(ofile);*/

    return 0;
}
