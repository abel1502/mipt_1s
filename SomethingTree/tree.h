#ifndef TREE_H
#define TREE_H


namespace SomethingTree {

    // TODO: Traits?

    template <typename QT, typename AT>
    class AbstractDecisionProvider {
    public:
        virtual ~AbstractDecisionProvider() = default;

        virtual bool ask(QT question) = 0;

        virtual bool verify(AT answer) = 0;
    };

    template <typename QT, typename AT>
    class ConsoleDecisionProvider : public AbstractDecisionProvider<QT, AT> {
    public:
        virtual bool ask(QT question) override;

        virtual bool verify(AT answer) override;
    };

    template <typename QT, typename AT>
    class ValueDTN;

    template <typename QT, typename AT>
    class AbstractDTN {
    public:
        virtual ~AbstractDTN() = default;  // TODO: Maybe I need to set =0 to ensure explicit redefinition?

        virtual ValueDTN<QT, AT> *find(AbstractDecisionProvider<QT, AT> *goal) = 0;

        //virtual void serialize(FILE *ofile);
        //static virtual AbstractDTN *deserialize(FILE *ifile);
    };

    template <typename QT, typename AT>
    class ChoiceDTN : public AbstractDTN<QT, AT> {
    public:
        ChoiceDTN(QT new_question, AbstractDTN<QT, AT> *childTrue, AbstractDTN<QT, AT> *childFalse);

        virtual ValueDTN<QT, AT> *find(AbstractDecisionProvider<QT, AT> *goal) override;

    protected:
        QT question;

        AbstractDTN<QT, AT> *children[2];

        static_assert(sizeof(children) == sizeof(void *) * 2, "I'm not sure if the declaration syntax was correct");

        // TODO
    };

    template <typename QT, typename AT>
    class ValueDTN : public AbstractDTN<QT, AT> {
    public:
        ValueDTN(AT new_value);

        virtual ValueDTN<QT, AT> *find(AbstractDecisionProvider<QT, AT> *goal) override;

    protected:
        AT value;
        // TODO
    };


    template <typename QT, typename AT>
    class DecisionTree {
    public:
        //

    protected:
        AbstractDTN<QT, AT> *root = nullptr;
    };

}


#endif // TREE_H
