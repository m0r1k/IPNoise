#include "class.hpp"
DEFINE_CLASS(Template)

#ifndef TEMPLATE_HPP
#define TEMPLATE_HPP

extern "C"
{
    #include <EXTERN.h>
    #include <perl.h>
    #include <XSUB.h>
};

#include <stdint.h>

#include <string>

#include "prop/map.hpp"
#include "core/neuron/neuron/main.hpp"
#include "context.hpp"

DEFINE_CLASS(Neuron);
DEFINE_CLASS(NeuronSession);

using namespace std;

//XS(XS_main_getNeuronById);

class Template
{
    public:
        Template(Neuron *a_neuron = NULL);
        virtual ~Template();

        string  processPerlCode(const string &);
        void    setupContext();
        void    setPrefix(const string &);
        string  getPrefix() const;

        string  render(
            PropNeuronIdSptr    &a_id,
            const string        &a_view = "index"
        );

        string  renderEmbedded(
            PropNeuronIdSptr    &a_id,
            const string        &a_view = "widget"
        );

        int32_t parseFile(
            const string    &a_fname,
            string          &a_out
        );

        static NeuronSptr   getNeuron();
        static DbThread *   getDbThread();
        static SV *         parseNeuronProps(NeuronSptr);
        static SV *         parseNeuronProps(PropSptr);

    protected:
        static void xs_init(pTHX);

    private:
        PerlInterpreter     *m_perl;
        Neuron              *m_neuron;
        string              m_prefix;
};

#endif

