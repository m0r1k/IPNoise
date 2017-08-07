#include "class.hpp"
DEFINE_CLASS(ObjectMicrocode);

#ifndef OBJECT_MICROCODE_HPP
#define OBJECT_MICROCODE_HPP

#include <string>
#include <vector>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectMicrocodeLocation);

using namespace std;

typedef int32_t (*Operator)(
    ObjectMicrocodeLocationSptr a_location,
    ObjectMapSptr               a_vars
);

typedef map<string, Operator>       Operators;
typedef Operators::iterator         OperatorsIt;
typedef Operators::const_iterator   OperatorsConstIt;

typedef enum
{
    STATE_OPERNAME_WAIT     = 0,
    STATE_OPERNAME_COLLECT,
    STATE_VAR_NAME_WAIT     = 10,
    STATE_VAR_NAME_COLLECT,
    STATE_ASSIGN_WAIT       = 20,
    STATE_ASSIGN_DIGIT,
    STATE_ASSIGN_FLOAT,
    STATE_ASSIGN_STRING,
    STATE_ASSIGN_VAR
} State;

class ObjectMicrocode
    :   public Object
{
    public:
        ObjectMicrocode(const CreateFlags &a_create_flags);
        virtual ~ObjectMicrocode();

        virtual string      getType();
        virtual int32_t     do_init_as_prop(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual int32_t     do_init_as_object(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual ObjectSptr  copy();
        virtual void        getAllProps(ObjectMapSptr);

        // generic
        int32_t parse(
            ObjectMicrocodeLocationSptr,
            ObjectMapSptr
        );
        int32_t parseFile(const char *a_fname);

        // static
        static int32_t          s_operatorVar(
            ObjectMicrocodeLocationSptr a_location,
            ObjectMapSptr               a_vars
        );
        static int32_t          s_operatorAssign(
            ObjectMicrocodeLocationSptr a_location,
            ObjectMapSptr               a_vars
        );
        static int32_t          s_operatorRun(
            ObjectMicrocodeLocationSptr a_location,
            ObjectMapSptr               a_vars
        );

        static int32_t s_test_parse(const char *a_tmp_path);

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    protected:
        void    registerOperators();

    private:
        Operators   m_operators;
};

#endif

