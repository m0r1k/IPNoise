#include "textEditWidget.hpp"

TextEditWidget::TextEditWidget(QWidget *a_parent)
    :   QTextEdit(a_parent)
{
    m_ctrl_pressed = 0;
}

TextEditWidget::~TextEditWidget()
{
}

void TextEditWidget::keyPressEvent(QKeyEvent *e)
{
    QTextEdit::keyPressEvent(e);
    if (e->modifiers() & Qt::ControlModifier){
        m_ctrl_pressed = 1;
    }
    if (e->modifiers() & Qt::ShiftModifier){
        m_shift_pressed = 1;
    }

    if (e->key() == Qt::Key_Return){
        if (    m_ctrl_pressed
            ||  m_shift_pressed)
        {
            append("<br/>");
        } else {
            emit submit();
        }
    }

    m_ctrl_pressed  = 0;
    m_shift_pressed = 0;
}

void TextEditWidget::keyReleaseEvent(QKeyEvent *e)
{
    QTextEdit::keyReleaseEvent(e);
    if (e->modifiers() & Qt::ControlModifier){
        m_ctrl_pressed = 0;
    }
    if (e->modifiers() & Qt::ShiftModifier){
        m_shift_pressed = 0;
    }
}

