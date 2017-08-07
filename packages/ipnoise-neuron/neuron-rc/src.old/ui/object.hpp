#include "class.hpp"
DEFINE_CLASS(UiObject);

#ifndef UI_OBJECT_HPP
#define UI_OBJECT_HPP

#include <stdint.h>

#include <qgl.h>
#include <GL/glu.h>
#include <QGLViewer/frame.h>

#include "log.hpp"

DEFINE_CLASS(UiViewer);
DEFINE_CLASS(UiObjectLine);
DEFINE_CLASS(UiObjectObject);

using namespace std;
using namespace qglviewer;

typedef vector<float>           Color;
typedef Color::iterator         ColorIt;
typedef Color::const_iterator   ColorConstIt;
typedef shared_ptr<Color>       ColorSptr;

class UiObject
{
    public:
        enum UiType {
            UI_TYPE_ERROR  = 0,
            UI_TYPE_OBJECT,
            UI_TYPE_LINE
        };

        UiObject(
            UiViewer                *,
            const UiObject::UiType  &
        );
        virtual ~UiObject();

        virtual void        draw() = 0;
        virtual void        selected();
        UiViewer *          getViewer() const;
        UiObject::UiType    getType()   const;
        Frame               m_frame;

    private:
        UiViewer            *m_viewer;
        UiObject::UiType    m_type;

};

#endif

