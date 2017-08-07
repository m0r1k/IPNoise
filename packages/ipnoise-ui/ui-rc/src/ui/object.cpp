#include "viewer.hpp"

#include "object.hpp"

UiObject::UiObject(
    UiViewer                *a_viewer,
    const UiObject::UiType  &a_type)
{
    m_type      = a_type;
    m_viewer    = a_viewer;
}

UiObject::~UiObject()
{
}

UiObject::UiType UiObject::getType() const
{
    return m_type;
}

UiViewer * UiObject::getViewer() const
{
    return m_viewer;
}

void UiObject::selected()
{
    PWARN("object selected\n");
}

