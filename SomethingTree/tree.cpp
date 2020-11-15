#include "tree.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include "general.h"


// ==== [ List inclusion ] ====
// As much as I'd like to avoid using such extremely-C-level library, I kind of need it for definition comparison
typedef struct {
    SomethingTree::ChoiceDTN *node;
    bool childDir;
} list_elem_t;
#define LIST_VALIDATION_LEVEL  3
#define LIST_DUMPNAME          list_dump
#include "../SpeedyList/list.h"
// =============================


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


    ValueDTN *ChoiceDTN::lookup(AbstractDTN **newRoot __attribute__((unused))) {
        bool reply = ask(question);

        return children[reply]->lookup(&children[reply]);
    }


    ValueDTN *ValueDTN::lookup(AbstractDTN **newRoot) {
        if (verify(value)) {
            printf("Yay, I guessed!\n");

            return this;
        } else {
            printf("So, what is it, and how does it differ from %s? :\n", value);

            char tmpAns[MAX_VALUE_LEN] = "";

            printf("It's ");
            fgets(tmpAns, MAX_VALUE_LEN - 1, stdin);
            while (strchr(tmpAns, '"')) {
                printf("Items with '\"' are forbidden. Try again:\n");

                printf("It's ");
                fgets(tmpAns, MAX_VALUE_LEN - 1, stdin);
            }

            int ansLen = strlen(tmpAns);
            if (ansLen > 0)  tmpAns[ansLen - 1] = '\0';

            char tmpQue[MAX_VALUE_LEN] = "";

            printf("And unlike %s, it ", value);
            fgets(tmpQue, MAX_VALUE_LEN - 1, stdin);

            while (strstr(tmpQue, "not ") || strchr(tmpQue, '"')) {
                printf("Definitions with \"not\" or '\"' are forbidden. Try again:\n");

                printf("It ");
                fgets(tmpQue, MAX_VALUE_LEN - 1, stdin);
            }

            int queLen = strlen(tmpQue);
            if (queLen > 0)  tmpQue[queLen - 1] = '\0';

            ValueDTN *newOption = new ValueDTN(tmpAns);

            ChoiceDTN *tmpParent = parent;
            bool tmpParentDir = parentDir;

            ChoiceDTN *newChoice = new ChoiceDTN(tmpQue, newOption, this);

            newChoice->parent = tmpParent;
            newChoice->parentDir = tmpParentDir;

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

        parent = nullptr;
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

        childFalse->parent = childTrue->parent = this;
        childFalse->parentDir = false;
        childTrue->parentDir = true;

        parent = nullptr;
        parentDir = false;
    }


    ChoiceDTN::~ChoiceDTN() {
        free(question);

        delete children[0];
        delete children[1];
    }


    const char *ChoiceDTN::getQuestion() {
        return question;
    }


    ValueDTN *ChoiceDTN::findByName(const char *name) {
        ValueDTN * leftRes = children[0]->findByName(name);
        ValueDTN *rightRes = children[1]->findByName(name);

        assert(!leftRes || !rightRes);

        return (ValueDTN *)((size_t)leftRes | (size_t)rightRes);  // One of them is guaranteed to be zero
    }


    ValueDTN *ValueDTN::findByName(const char *name) {
        if (strcmp(value, name) == 0) {
            return this;
        }

        return nullptr;
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


    void DecisionTree::lookup() {
        if (!root) {
            printf("I guess I don't know anything yet, so let's just pretend this didn't happen...\n"
                   "What did you think of? :\n");

            char tmpAns[MAX_VALUE_LEN] = "";

            printf("It's ");
            fgets(tmpAns, MAX_VALUE_LEN - 1, stdin);
            while (strchr(tmpAns, '"')) {
                printf("Items with '\"' are forbidden. Try again:\n");

                printf("It's ");
                fgets(tmpAns, MAX_VALUE_LEN - 1, stdin);
            }

            int ansLen = strlen(tmpAns);
            if (ansLen > 0)  tmpAns[ansLen - 1] = '\0';

            root = new ValueDTN(tmpAns);

            return;
        }

        root->lookup(&root);
    }


    static void printDefinition_(ChoiceDTN *node, bool childDir) {
        if (!node)  return;

        if (!node->parent) {  // The root needs special care
            printf("%s%s", childDir ? "" : "not ", node->getQuestion());

            return;
        }

        printDefinition_(node->parent, node->parentDir);

        printf(", %s %s", childDir ? "and" : "but not", node->getQuestion());
    }


    void DecisionTree::define(const char *term) {
        ValueDTN *target = root->findByName(term);

        if (!target) {
            printf("Item '%s' not found.\n", term);
            return;
        }

        printf("%s ", term);

        printDefinition_(target->parent, target->parentDir);

        printf(".\nAmazing).\n");
    }


    void DecisionTree::compare(const char *term1, const char *term2) {
        ValueDTN *target1 = root->findByName(term1);
        ValueDTN *target2 = root->findByName(term2);

        if (!target1) {
            printf("Item '%s' not found.\n", term1);
            return;
        }

        if (!target2) {
            printf("Item '%s' not found.\n", term2);
            return;
        }

        if (target1 == target2) {
            printf("%s and %s are the same thing.\n", term1, term2);
            return;
        }

        list_t path1{}, path2{};
        list_init(&path1, 32);
        list_init(&path2, 32);

        ChoiceDTN *curDTN1 = target1->parent;
        bool childDir1 = target1->parentDir;
        ChoiceDTN *curDTN2 = target2->parent;
        bool childDir2 = target2->parentDir;

        while (curDTN1) {
            //printf("> %s\n", curDTN1->getQuestion());

            bool res = list_pushBack(&path1, {curDTN1, childDir1});
            assert(!res);

            childDir1 = curDTN1->parentDir;
            curDTN1 = curDTN1->parent;
        }

        while (curDTN2) {
            //printf("> %s\n", curDTN2->getQuestion());

            bool res = list_pushBack(&path2, {curDTN2, childDir2});
            assert(!res);

            childDir2 = curDTN2->parentDir;
            curDTN2 = curDTN2->parent;
        }

        list_node_t *cur1 = list_getNode(&path1, 0);
        list_node_t *cur2 = list_getNode(&path2, 0);
        assert(cur1 && cur2);

        #define PREV_() \
            cur1 = list_getNode(&path1, cur1->prev); \
            cur2 = list_getNode(&path2, cur2->prev); \
            assert(cur1 && cur2);

        #define NEXT_() \
            cur1 = list_getNode(&path1, cur1->next); \
            cur2 = list_getNode(&path2, cur2->next); \
            assert(cur1 && cur2);

        while (cur1->prev != 0 && cur2->prev != 0) {
            PREV_();

            assert(cur1->value.node == cur2->value.node);

            if (cur1->value.childDir != cur2->value.childDir) {
                break;
            }
        }

        NEXT_();

        if (cur1 != list_getNode(&path1, 0)) {
            if (cur1->value.childDir) {
                printf("%s, just like %s, ", term1, term2);
            } else {
                printf("Neither %s nor %s ", term1, term2);
            }
            printf("%s, but ", cur1->value.node->getQuestion());
        }

        PREV_();

        if (cur1->value.childDir) {
            printf("%s, unlike %s, %s.\n", term1, term2, cur1->value.node->getQuestion());
        } else {
            printf("%s, unlike %s, %s.\n", term2, term1, cur1->value.node->getQuestion());
        }

        #undef PREV_
        #undef NEXT_

        list_free(&path1);
        list_free(&path2);
    }


    #define DUMP_(...)  fprintf(ofile, ##__VA_ARGS__)

    void ChoiceDTN::dump(FILE *ofile) {
        DUMP_("%zu -> %zu [color=blue label=\"no\"]\n", (size_t)this, (size_t)children[0]);
        DUMP_("%zu -> %zu [color=red label=\"yes\"]\n", (size_t)this, (size_t)children[1]);

        DUMP_("%zu [fontname=Consolas shape=box style=filled color=black fillcolor=\"#0096FF\" label=\"%s\"]\n", (size_t)this, question);

        children[0]->dump(ofile);
        children[1]->dump(ofile);
    }


    void ValueDTN::dump(FILE *ofile) {
        DUMP_("%zu [fontname=Consolas shape=box style=filled color=black fillcolor=\"#FF6900\" label=\"%s\"]\n", (size_t)this, value);
    }


    void DecisionTree::dump() {
        #define F_DOT_  "dump/tree.dump"
        #define F_SVG_  "dump/tree.svg"

        FILE *ofile = fopen(F_DOT_, "w");

        DUMP_("digraph DecisionTree {\n");

        DUMP_("graph [rankdir=TD splines=spline]\n");

        if (root) {
            root->dump(ofile);
        }

        DUMP_("}\n");

        fclose(ofile);

        system("dot -Tsvg " F_DOT_ " -o " F_SVG_);
        system("start " F_SVG_);
    }

    #undef DUMP_

}
