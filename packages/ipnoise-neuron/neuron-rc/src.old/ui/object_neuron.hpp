#include "class.hpp"
DEFINE_CLASS(UiObjectObject);

#ifndef UI_OBJECT_HPP
#define UI_OBJECT_HPP

#include <stdint.h>

#include <qgl.h>
#include <GL/glu.h>
#include <QGLViewer/frame.h>
#include <qmessagebox.h>

#include <vector>
#include <memory>

#include "log.hpp"
#include "ui/object.hpp"
#include "core/object/object/main.hpp"

using namespace std;
using namespace qglviewer;

typedef vector<UiObjectLineSptr>    Links;
typedef Links::iterator             LinksIt;
typedef Links::const_iterator       LinksConstIt;

class UiObjectObject
    :   public UiObject
{
    public:
        UiObjectObject(UiViewer *);
        virtual ~UiObjectObject();

        virtual void    draw();
        virtual void    selected();

        void    processHits (GLint hits, GLuint buffer[]);
        void    selectObjects();
        void    selectObjects(
            const float &a_x,
            const float &a_y,
            const float &a_z
        );

        void    searchLinks();
        void    getConnectedLinks(
            vector<UiObjectLineSptr>    &a_out,
            UiObjectObject              *a_skip_object = NULL
        );

        UiObjectLineSptr getLink(UiObjectObject *);
        UiObjectLineSptr getFreeLink();

        ObjectSptr  m_object;
        Links       m_links;

        float   getCharge()     const;
        void    setCharge(const float &);
        void    inc();
        void    dec();
        void    normalize();

        ColorSptr   m_color;

        float   m_charge;

    private:
};

#endif

