#include "manipulatedFrameSetConstraint.hpp"

void ManipulatedFrameSetConstraint::clearSet()
{
    m_objects.clear();
}

void ManipulatedFrameSetConstraint::addObjectToSet(
    UiObjectSptr  a_neuron)
{
    m_objects.append(a_neuron);
}

void ManipulatedFrameSetConstraint::constrainTranslation(
    Vec     &a_translation,
    Frame   *const)
{
    QList<UiObjectSptr>::iterator it;
    for (it = m_objects.begin();
        it != m_objects.end();
        it++)
    {
        it->get()->m_frame.translate(a_translation);
    }
}

void ManipulatedFrameSetConstraint::constrainRotation(
    Quaternion      &a_rotation,
    Frame *const    a_frame)
{
    // A little bit of math. Easy to understand, hard to guess (tm).
    // rotation is expressed in the frame local coordinates system.
    // Convert it back to world coordinates.

    QList<UiObjectSptr>::iterator it;

    const Vec   worldAxis   = a_frame->inverseTransformOf(a_rotation.axis());
    const Vec   pos         = a_frame->position();
    const float angle       = a_rotation.angle();

    for (it = m_objects.begin();
        it != m_objects.end();
        it++)
    {
        // Rotation has to be expressed in the object local coordinates system.
        Quaternion qObject(
            it->get()->m_frame.transformOf(worldAxis),
            angle
        );
        it->get()->m_frame.rotate(qObject);

        // Comment these lines only rotate the objects
        Quaternion qWorld(worldAxis, angle);

        // Rotation around frame world position (pos)
        it->get()->m_frame.setPosition(
            pos + qWorld.rotate(
                it->get()->m_frame.position() - pos
            )
        );
    }
}

