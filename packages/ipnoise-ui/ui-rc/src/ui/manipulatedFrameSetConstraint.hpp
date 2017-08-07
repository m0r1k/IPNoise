#ifndef MANIPULATED_FRAME_SET_CONSTRAINT
#define MANIPULATED_FRAME_SET_CONSTRAINT

#include <QGLViewer/constraint.h>
#include <QGLViewer/frame.h>
#include "object.hpp"

using namespace std;
using namespace qglviewer;

class ManipulatedFrameSetConstraint
    :   public Constraint
{
    public:
        void clearSet();
        void addObjectToSet(UiObjectSptr);

        virtual void constrainTranslation(
            Vec             &a_translation,
            Frame *const    a_frame
        );
        virtual void constrainRotation(
            Quaternion      &a_rotation,
            Frame           *const a_frame
        );

    private:
        QList<UiObjectSptr>   m_objects;
};

#endif

