#include "viewer.hpp"
#include "object_line.hpp"

#define BUFSIZE 1*1024*1024

UiObjectLine::UiObjectLine(
    UiViewer *a_viewer)
    :   UiObject(
            a_viewer,
            UiObject::UI_TYPE_LINE
        )
{
    m_selected      = 0;
    m_object_from   = NULL;
    m_object_to     = NULL;
    m_charge        = 0;
}

UiObjectLine::~UiObjectLine()
{
}

void UiObjectLine::processHits(
    GLint   hits,
    GLuint  buffer[])
{
    unsigned int i, j;
    GLuint names_count, *ptr;

    vector<UiObjectLineSptr>            links_our;
    vector<UiObjectLineSptr>::iterator  links_our_it;
    vector<UiObjectLineSptr>            links_foreign;
    vector<UiObjectLineSptr>::iterator  links_foreign_it;

    PDEBUG (10, "----- hits %d -----\n", hits);
    ptr = (GLuint *) buffer;

    for (i = 0; i < (uint32_t)hits; i++){
        vector<int32_t> names;
        int32_t             id = 0;
        UiObjectSptr        object;
        UiObjectLineSptr    link;

        // for each hit
        names_count = *ptr;

        //printf (" number of names for hit = %d\n", names);
        ptr++;

        //printf (" z1 is %g;",  (float) *ptr/0x7fffffff);
        ptr++;

        //printf (" z2 is %g\n", (float) *ptr/0x7fffffff);
        ptr++;

        //printf ("   the name is ");
        for (j = 0; j < names_count; j++){
            //  for each name
            names.push_back(*ptr);
            ptr++;
        }
        //printf ("\n");

        id = names.at(1);
        object = getViewer()->m_objects.at(id);
        if (!object){
            // object not found
            PWARN("object not found\n");
            continue;
        }
        link = dynamic_pointer_cast<UiObjectLine>(object);
        if (!link){
            // skip non links
            PWARN("skip non link\n");
            continue;
        }
        if (link->m_object_from == m_object_from){
            // it is our object link
            links_our.push_back(link);
        } else {
            // it is foreign object link
            if (link->m_object_to){
                // already connected
                continue;
            }
            links_foreign.push_back(link);
        }
    }

    if (links_our.size() && links_foreign.size()){
        UiObjectLineSptr    our     = *links_our.begin();
        UiObjectLineSptr    foreign = *links_foreign.begin();
        float               ax, ay, az;

        UiObjectObject  *our_object     = NULL;
        UiObjectObject  *foreign_object = NULL;

        foreign_object  = foreign->m_object_from;
        our_object      = our->m_object_from;

        // get remote object position
        foreign_object->m_frame.getPosition(ax, ay, az);

        // setup our link to remote
        our->m_frame_to.setPosition(
            Vec(ax, ay, az)
        );

        // get our object position
        our_object->m_frame.getPosition(ax, ay, az);

        // setup remote link to our
        foreign->m_frame_to.setPosition(
            Vec(ax, ay, az)
        );

        // store remote object
        our->m_object_to      = foreign_object;
        foreign->m_object_to  = our_object;

        // process foreign
        // foreign->selectObjects();
    }
}

void UiObjectLine::selectObjects(void)
{
    GLuint  selectBuf[BUFSIZE];
    GLint   hits;
    float   x, y, z;

    m_frame_to.getPosition(
        x, y, z
    );

    glSelectBuffer (BUFSIZE, selectBuf);
    (void) glRenderMode (GL_SELECT);

    glInitNames();
    glPushName(0);
    glPushMatrix ();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    // glOrtho(
    //     левая граница,  правая граница,
    //     нижняя граница, верхняя граница,
    //     расстояние до ближнего плана отсечения,
    //     расстояние до дальнего плана отсечения
    // );
    float max_distance = getViewer()->m_step;
    glOrtho (
        -max_distance, max_distance,
        -max_distance, max_distance,
        -max_distance, max_distance
    );

    // Устанавливаем точку, в которой
    // находится наш глаз                       --- (x,y,z)
    // направление, куда смотрим                --- (0,0,0)
    // вектор, принимаемый за направление вверх --- (0,1,0)
    // этим вектором является ось Y
    gluLookAt(x,y,z, 0,0,0, 0,1,0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    //getViewer()->drawWithNames();

    glPopMatrix ();
    glFlush ();

    hits = glRenderMode (GL_RENDER);
    processHits (hits, selectBuf);
}

void UiObjectLine::draw()
{
    float ax, ay, az;
    float bx, by, bz;

    if (!m_object_to){
        goto out;
    }

    m_frame_from.getPosition(
        ax, ay, az
    );

    m_frame_to.getPosition(
        bx, by, bz
    );

    if (m_charge > 0){
        glColor3f(m_charge, 0, 0);
    } else {
        glColor3f(0, 0, -m_charge);
    }

    glLineWidth(0.5);
    glBegin(GL_LINES);
    glVertex3f (ax, ay, az);
    glVertex3f (bx, by, bz);
    glEnd();

out:
    return;
}

float UiObjectLine::getCharge() const
{
    return m_charge;
}

void UiObjectLine::setCharge(
    const float &a_charge)
{
    m_charge = a_charge;
}

void UiObjectLine::normalize()
{
    if (m_charge < -1){
        m_charge = -1;
    }
    if (m_charge > 1){
        m_charge = 1;
    }
}

void UiObjectLine::inc()
{
    m_charge += 0.1f;
    normalize();
}

void UiObjectLine::dec()
{
    m_charge -= 0.1f;
    normalize();
}

void UiObjectLine::selected()
{
    PWARN("object line selected\n");
    m_selected = m_selected ? 0 : 1;

    // selectObjects();
}

