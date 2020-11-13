#include "tree.h"

#include <cstdio>
#include <cassert>
#include <cstring>>


namespace SomethingTree {

    template <>
    bool ConsoleDecisionProvider<char *, char *>::ask(char *question) {
        printf("It %s? ([Y]es/[n]o)\n", question);

        char reply[81] = "";
        int res = scanf("%80s", &reply);
        assert(res == 1);

        return reply[0] != 'n' && reply[0] != 'N';
    }

    template <>
    bool ConsoleDecisionProvider<char *, char *>::verify(char *answer) {
        printf("Did you mean \"%s\"? ([Y]es/[n]o)\n", answer);

        char reply[81] = "";
        int res = scanf("%80s", &reply);
        assert(res == 1);

        return reply[0] != 'n' && reply[0] != 'N';
    }

    template <typename QT, typename AT>
    ValueDTN<QT, AT> *ChoiceDTN<QT, AT>::find(AbstractDecisionProvider<QT, AT> *goal) {
        return children[goal->ask(question)]->find(goal);
    }

    template <typename QT, typename AT>
    ValueDTN<QT, AT> *ValueDTN<QT, AT>::find(AbstractDecisionProvider<QT, AT> *goal) {
        if (goal->verify(value)) {
            return this;
        }
        return NULL;
    }

    template <typename QT, typename AT>
    ValueDTN<QT, AT>::ValueDTN(AT new_value) :
        value{new_value} {}

    template <typename QT, typename AT>
    ChoiceDTN<QT, AT>::ChoiceDTN(QT new_question, AbstractDTN<QT, AT> *childTrue, AbstractDTN<QT, AT> *childFalse) :
        question{new_question}, children{childFalse, childTrue} {}

    template class AbstractDTN<char *, char *>;
    template class ValueDTN<char *, char *>;
    template class ChoiceDTN<char *, char *>;

    template class AbstractDecisionProvider<char *, char *>;
    template class ConsoleDecisionProvider<char *, char *>;
}
