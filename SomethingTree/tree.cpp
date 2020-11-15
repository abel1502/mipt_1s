#include "tree.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include "general.h"


namespace SomethingTree {

    bool ask(const char *question) {
        printf("It %s? ([Y]es/[n]o)\n", question);

        char reply[64] = "";
        fgets(reply, 63, stdin);

        return reply[0] != 'n' && reply[0] != 'N';
    }

    bool verify(const char *answer) {
        printf("Did you mean \"%s\"? ([Y]es/[n]o)\n", answer);

        char reply[64] = "";
        fgets(reply, 63, stdin);

        return reply[0] != 'n' && reply[0] != 'N';
    }


    AbstractDTN *AbstractDTN::deserialize(FILE *ifile) {
        DTFNodeHeader header{ifile};

        switch (header.type) {
        case VALUE_NODE: {
            char answer[MAX_VALUE_LEN] = "";

            fread(answer, 1, header.valueLen, ifile);

            ValueDTN *result = new ValueDTN(answer);

            return result;
        }
        case CHOICE_NODE: {
            char question[MAX_VALUE_LEN] = "";

            fread(question, 1, header.valueLen, ifile);

            ChoiceDTN *result = new ChoiceDTN(question, AbstractDTN::deserialize(ifile), AbstractDTN::deserialize(ifile));

            return result;
        }
        default:
            ERR("Shouldn't be reachable");

            abort();
        }
    }


    ValueDTN *ChoiceDTN::find(AbstractDTN **newRoot) {
        bool reply = ask(question);

        return children[reply]->find(&children[reply]);
    }


    ValueDTN *ValueDTN::find(AbstractDTN **newRoot) {
        if (verify(value)) {
            printf("Yay, I guessed!\n");

            return this;
        } else {
            printf("So, what is it, and how does it differ from %s? :\n", value);

            char tmpAns[MAX_VALUE_LEN] = "";

            printf("It's ");
            fgets(tmpAns, MAX_VALUE_LEN - 1, stdin);
            int ansLen = strlen(tmpAns);
            if (ansLen > 0)  tmpAns[ansLen - 1] = '\0';

            char tmpQue[MAX_VALUE_LEN] = "";

            printf("And unlike %s, it ", value);
            fgets(tmpQue, MAX_VALUE_LEN - 1, stdin);

            while (strstr(tmpQue, "not ")) {
                printf("Definitions with not are forbidden by Ded. Try again:\n");

                printf("It ");
                fgets(tmpQue, MAX_VALUE_LEN - 1, stdin);
            }

            int queLen = strlen(tmpQue);
            if (queLen > 0)  tmpQue[queLen - 1] = '\0';

            ValueDTN *newOption = new ValueDTN(tmpAns);

            ChoiceDTN *newChoice = new ChoiceDTN(tmpQue, newOption, this);

            *newRoot = newChoice;

            return newOption;
        }
    }


    void ChoiceDTN::serialize(FILE *ofile) {
        DTFNodeHeader header{CHOICE_NODE, (uint8_t)strlen(question)};

        header.write(ofile);

        fwrite(question, 1, header.valueLen, ofile);

        children[1]->serialize(ofile);
        children[0]->serialize(ofile);
    }


    void ValueDTN::serialize(FILE *ofile) {
        DTFNodeHeader header{VALUE_NODE, (uint8_t)strlen(value)};

        header.write(ofile);

        fwrite(value, 1, header.valueLen, ofile);
    }


    ValueDTN::ValueDTN(const char *new_value) {
        value = (char *)calloc(MAX_VALUE_LEN + 1, 1);

        strncpy(value, new_value, MAX_VALUE_LEN);
    }


    ValueDTN::~ValueDTN() {
        free(value);
    }


    const char *ValueDTN::getValue() {
        return value;
    }


    ChoiceDTN::ChoiceDTN(const char *new_question, AbstractDTN *childTrue, AbstractDTN *childFalse) : children{childFalse, childTrue} {
        question = (char *)calloc(MAX_VALUE_LEN + 1, 1);

        strncpy(question, new_question, MAX_VALUE_LEN);
    }


    ChoiceDTN::~ChoiceDTN() {
        free(question);

        delete children[0];
        delete children[1];
    }


    DTFHeader::DTFHeader() : magic{MAGIC}, version{VERSION} {}


    DTFHeader::DTFHeader(FILE *ifile) {
        size_t res = fread(this, sizeof(*this), 1, ifile);

        assert(res == 1);

        assert(magic == MAGIC);
        assert(version == VERSION);
    }


    DTFNodeHeader::DTFNodeHeader(uint8_t new_type, uint8_t new_valueLen) : type{new_type}, valueLen{new_valueLen} {}


    DTFNodeHeader::DTFNodeHeader(FILE *ifile) {
        size_t res = fread(this, sizeof(*this), 1, ifile);

        assert(res == 1);

        assert(valueLen < MAX_VALUE_LEN);
    }


    void DTFHeader::write(FILE *ofile) {
        int res = fwrite(this, sizeof(*this), 1, ofile);

        assert(res == 1);
    }


    void DTFNodeHeader::write(FILE *ofile) {
        int res = fwrite(this, sizeof(*this), 1, ofile);

        assert(res == 1);

        assert(valueLen <= MAX_VALUE_LEN);
    }


    DecisionTree::DecisionTree() : root(nullptr) {}


    DecisionTree::~DecisionTree() {
        if (root) {
            delete root;
        }
    }


    void DecisionTree::serialize(FILE *ofile) {
        DTFHeader header{};

        header.write(ofile);

        root->serialize(ofile);
    }


    void DecisionTree::deserialize(FILE *ifile) {
        DTFHeader header{ifile};

        root = AbstractDTN::deserialize(ifile);
    }


    void DecisionTree::find() {
        if (!root) {
            printf("I guess I don't know anything yet, so let's just pretend this didn't happen...\n"
                   "What did you think of? :\n");

            char tmpAns[MAX_VALUE_LEN] = "";

            printf("It's ");
            fgets(tmpAns, MAX_VALUE_LEN - 1, stdin);
            int ansLen = strlen(tmpAns);
            if (ansLen > 0)  tmpAns[ansLen - 1] = '\0';

            root = new ValueDTN(tmpAns);

            return;
        }

        root->find(&root);
    }

}
