#ifndef TREE_H
#define TREE_H

#include <cstdio>
#include <cstdint>


namespace SomethingTree {

    class AbstractGoal {
    public:
        virtual ~AbstractGoal() = default;
    };


    class ConsoleGoal : public AbstractGoal {};


    class AbstractQuestion {
    public:
        virtual ~AbstractQuestion() = default;

        virtual bool ask(AbstractGoal *goal) = 0;

        virtual void serialize(FILE *ofile) = 0;

        virtual void deserialize(FILE *ifile) = 0;
    };


    class AbstractAnswer {
    public:
        virtual ~AbstractAnswer() = default;

        virtual bool verify(AbstractGoal *goal) = 0;

        virtual void serialize(FILE *ofile) = 0;

        virtual void deserialize(FILE *ifile) = 0;
    };


    class TextQuestion : public AbstractQuestion {
    public:
        TextQuestion(char *value, bool copy);

        virtual ~TextQuestion() override;

        virtual bool ask(AbstractGoal *goal) override;

        virtual void serialize(FILE *ofile) override;

        virtual void deserialize(FILE *ifile) override;

    protected:
        char *value;

        bool doFreeValue;
    };


    class TextAnswer : public AbstractAnswer {
    public:
        TextAnswer(char *value, bool copy);

        virtual ~TextAnswer() override;

        virtual bool verify(AbstractGoal *goal) override;

        virtual void serialize(FILE *ofile) override;

        virtual void deserialize(FILE *ifile) override;

    protected:
        char *value;

        bool doFreeValue;
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
        static const uint8_t MAX_VALUE_LEN = 64;

        enum NodeType {
            VALUE_NODE,
            CHOICE_NODE
        };

        uint8_t type : 2;  // Some excess for scalability
        uint8_t valueLen : 6;

        DTFNodeHeader(uint8_t new_type, AbstractDTN *node);

        DTFNodeHeader(FILE *ifile);

        void write(FILE *ofile);
    };

    static_assert(sizeof(DTFNodeHeader) == 1);

    #pragma pack()


    class ValueDTN;

    class AbstractDTN {
    public:
        virtual ~AbstractDTN() = default;

        virtual ValueDTN *find(AbstractGoal *goal) = 0;

    protected:
        static AbstractDTN *deserialize(FILE *ifile);

        virtual AbstractDTN *read(FILE *ifile) = 0;
    };


    class ChoiceDTN : public AbstractDTN {
    public:
        ChoiceDTN(AbstractQuestion *new_question, AbstractDTN *childTrue, AbstractDTN *childFalse);

        virtual ValueDTN *find(AbstractGoal *goal) override;

    protected:
        AbstractQuestion *question;

        AbstractDTN *children[2];

        static_assert(sizeof(children) == sizeof(void *) * 2, "I'm not sure if the declaration syntax was correct");

        virtual AbstractDTN *read(FILE *ifile) override;
    };


    class ValueDTN : public AbstractDTN {
    public:
        ValueDTN(AbstractAnswer *new_value);

        virtual ValueDTN *find(AbstractGoal *goal) override;

    protected:
        AbstractAnswer *value;

        virtual AbstractDTN *read(FILE *ifile) override;
    };


    class DecisionTree {
    public:
        void serialize(FILE *ofile);

        void deserialize(FILE *ifile);

    protected:
        AbstractDTN *root = nullptr;
    };

}


#endif // TREE_H
