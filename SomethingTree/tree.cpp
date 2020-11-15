#include "tree.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>


namespace SomethingTree {

    bool TextQuestion::ask() {
        printf("It %s? ([Y]es/[n]o)\n", value);

        char reply[64] = "";
        int res = scanf("%63s", reply);
        assert(res == 1);

        return reply[0] != 'n' && reply[0] != 'N';
    }


    bool TextQuestion::serialize(FILE *ofile) {

    }


    bool TextAnswer::verify() {
        printf("Did you mean \"%s\"? ([Y]es/[n]o)\n", value);

        char reply[64] = "";
        int res = scanf("%63s", reply);
        assert(res == 1);

        return reply[0] != 'n' && reply[0] != 'N';
    }


    AbstractDTN *AbstractDTN::deserialize(FILE *ifile) {
        DTFNodeHeader header{ifile};

        switch (header.type) {
        case VALUE_NODE:
            AnswerType answer = (AnswerType)calloc(header.valueLen + 1, 1);

            fread(answer, 1, header.valueLen)

            return new ValueDTN();
        case CHOICE_NODE:
            break;
        default:
            assert(false /* Shouldn't be reachable */);
        }
    }


    ValueDTN *ChoiceDTN::find(AbstractDecisionProvider *goal) {
        return children[goal->ask(question)]->find(goal);
    }


    ValueDTN *ValueDTN::find(AbstractDecisionProvider *goal) {
        if (goal->verify(value)) {
            return this;
        }
        return NULL;
    }


    ValueDTN::ValueDTN(AnswerType new_value) : value{new_value} {}


    ChoiceDTN::ChoiceDTN(QuestionType new_question, AbstractDTN *childTrue, AbstractDTN *childFalse) :
        question{new_question}, children{childFalse, childTrue} {}


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

        root.write(ofile);
    }


    void DecisionTree::deserialize(FILE *ifile) {
        DTFHeader header{ifile};

        root = AbstractDTN::deserialize(ifile);
    }

}
