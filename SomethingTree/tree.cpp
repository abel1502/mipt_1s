#include "tree.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>


namespace SomethingTree {

    bool ask(const char *question) {
        printf("It %s? ([Y]es/[n]o)\n", question);

        char reply[64] = "";
        int res = scanf("%63s", reply);
        assert(res == 1);

        return reply[0] != 'n' && reply[0] != 'N';
    }

    bool verify(const char *answer) {
        printf("Did you mean \"%s\"? ([Y]es/[n]o)\n", answer);

        char reply[64] = "";
        int res = scanf("%63s", reply);
        assert(res == 1);

        return reply[0] != 'n' && reply[0] != 'N';
    }


    AbstractDTN *AbstractDTN::deserialize(FILE *ifile) {
        DTFNodeHeader header{ifile};

        switch (header.type) {
        case VALUE_NODE: {
            char *answer = (char *)calloc(header.valueLen + 1, 1);

            fread(answer, 1, header.valueLen, ifile);

            ValueDTN *result = new ValueDTN(answer);

            free(answer);

            return result;
        }
        case CHOICE_NODE: {
            char *question = (char *)calloc(header.valueLen + 1, 1);

            fread(question, 1, header.valueLen, ifile);

            ChoiceDTN *result = new ChoiceDTN(question, AbstractDTN::deserialize(ifile), AbstractDTN::deserialize(ifile));

            free(question);

            return result;
        }
        default:
            assert(false /* Shouldn't be reachable */);

            return NULL;
        }
    }


    ValueDTN *ChoiceDTN::find() {
        return children[ask(question)]->find();
    }


    ValueDTN *ValueDTN::find() {
        if (verify(value)) {
            return this;
        }
        return NULL;
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


    void DecisionTree::serialize(FILE *ofile) {
        DTFHeader header{};

        header.write(ofile);

        root->serialize(ofile);
    }


    void DecisionTree::deserialize(FILE *ifile) {
        DTFHeader header{ifile};

        root = AbstractDTN::deserialize(ifile);
    }

}
