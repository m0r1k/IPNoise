#include "class.hpp"
DEFINE_CLASS(UiObjectLine);

#ifndef UI_OBJECT_LINE_HPP
#define UI_OBJECT_LINE_HPP

#include <stdint.h>

#include <qgl.h>
#include <GL/glu.h>
#include <QGLViewer/frame.h>

#include <vector>

#include "log.hpp"
#include "object_neuron.hpp"
#include "object.hpp"

using namespace std;
using namespace qglviewer;

class UiObjectLine
    :   public UiObject
{
    public:
        UiObjectLine(UiViewer *);
        virtual ~UiObjectLine();

        virtual void        draw();
        virtual void        selected();

        void processHits (GLint hits, GLuint buffer[]);
        void                selectObjects(void);

        UiObjectNeuron  *   m_neuron_from;
        UiObjectNeuron  *   m_neuron_to;
        Frame               m_frame_from;
        Frame               m_frame_to;

        int32_t             m_selected;

        float   getCharge()     const;
        void    setCharge(const float &);
        void    inc();
        void    dec();
        void    normalize();

    private:
        float   m_charge;

};

#endif

