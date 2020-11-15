#ifndef TREE_H
#define TREE_H

#include <cstdio>
#include <cstdint>


namespace SomethingTree {

    const uint8_t MAX_VALUE_LEN = 128;

    enum NodeType {
        VALUE_NODE,
        CHOICE_NODE
    };


    class AbstractDTN;

    #pragma pack(1)

    struct DTFHeader {
        static const uint32_t MAGIC = '\nFTD';

        static const uint32_t VERSION = 0;

        uint32_t magic;
        uint32_t version;

        DTFHeader();

        DTFHeader(FILE *ifile);

        void write(FILE *ofile);
    };

    struct DTFNodeHeader {
        uint8_t type : 1;
        uint8_t valueLen : 7;

        DTFNodeHeader(uint8_t new_type, uint8_t new_valueLen);

        DTFNodeHeader(FILE *ifile);

        void write(FILE *ofile);
    };

    static_assert(sizeof(DTFNodeHeader) == 1);

    #pragma pack()


    class ValueDTN;
    class ChoiceDTN;

    class AbstractDTN {
    public:
        virtual ~AbstractDTN() = default;

        virtual ValueDTN *lookup(AbstractDTN **newRoot) = 0;

        static AbstractDTN *deserialize(FILE *ifile);

        virtual void serialize(FILE *ofile) = 0;

        virtual void dump(FILE *ofile) = 0;

        virtual ValueDTN *findByName(const char *name) = 0;

        ChoiceDTN *parent;

        bool parentDir;
    };


    class ChoiceDTN : public AbstractDTN {
    public:
        ChoiceDTN(const char *new_question, AbstractDTN *childTrue, AbstractDTN *childFalse);

        ~ChoiceDTN();

        virtual ValueDTN *lookup(AbstractDTN **newRoot) override;

        virtual void serialize(FILE *ofile) override;

        const char *getQuestion();

        virtual void dump(FILE *ofile) override;

        virtual ValueDTN *findByName(const char *name) override;

    protected:
        char *question;

        AbstractDTN *children[2];

        static_assert(sizeof(children) == sizeof(void *) * 2, "I'm not sure if the declaration syntax was correct");
    };


    class ValueDTN : public AbstractDTN {
    public:
        ValueDTN(const char *new_value);

        ~ValueDTN();

        virtual ValueDTN *lookup(AbstractDTN **newRoot) override;

        virtual void serialize(FILE *ofile) override;

        const char *getValue();

        virtual void dump(FILE *ofile) override;

        virtual ValueDTN *findByName(const char *name) override;

    protected:
        char *value;
    };


    class DecisionTree {
    public:
        DecisionTree();

        ~DecisionTree();

        void serialize(FILE *ofile);

        void deserialize(FILE *ifile);

        void lookup();

        void dump();

        void define(const char *term);

        void compare(const char *term1, const char *term2);

    protected:
        AbstractDTN *root = nullptr;
    };

}


#endif // TREE_H
