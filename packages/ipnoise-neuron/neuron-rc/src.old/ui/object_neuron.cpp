#include "object_line.hpp"
#include "object_object.hpp"
#include "viewer.hpp"

#define BUFSIZE 1*1024*1024

#define TAIL_PERIOD 0
int32_t tail = 0;

UiObjectObject::UiObjectObject(
    UiViewer *a_viewer)
    :   UiObject(
            a_viewer,
            UiObject::UI_TYPE_OBJECT
        )
{
    m_charge = 0;
}

UiObjectObject::~UiObjectObject()
{
}

float UiObjectObject::getCharge() const
{
    return m_charge;
}

void UiObjectObject::normalize()
{
    if (m_charge < -1){
        m_charge = -1;
    }
    if (m_charge > 1){
        m_charge = 1;
    }
}

void UiObjectObject::inc()
{
    setCharge(m_charge + 0.05f);
    normalize();
}

void UiObjectObject::dec()
{
    setCharge(m_charge - 0.05f);
    normalize();
}

void UiObjectObject::draw()
{
    static GLUquadric *quad = gluNewQuadric();

    glPushMatrix();
    glMultMatrixd(m_frame.matrix());

    if (m_color){
        glColor3f(
            m_color->at(0),
            m_color->at(1),
            m_color->at(2)
        );
    } else {
        if (m_charge > 0){
            glColor3f(m_charge, 0, 0);
        } else {
            glColor3f(0, 0, -m_charge);
        }
    }

    gluSphere(quad, 0.01, 10, 6);
    //gluCylinder(quad, 0.03, 0.0, 0.09, 10, 1);
    glPopMatrix();
}

UiObjectLineSptr UiObjectObject::getLink(
    UiObjectObject *a_object)
{
    LinksConstIt        it;
    UiObjectLineSptr    ret;
    for (it = m_links.begin();
        it != m_links.end();
        it++)
    {
        UiObjectLineSptr cur_link = *it;
        if (    !cur_link->m_object_to
            ||  cur_link->m_object_to != a_object)
        {
            continue;
        }
        ret = cur_link;
        break;
    }
    return ret;
}

UiObjectLineSptr UiObjectObject::getFreeLink()
{
    LinksConstIt        it;
    UiObjectLineSptr    ret;
    for (it = m_links.begin();
        it != m_links.end();
        it++)
    {
        UiObjectLineSptr cur_link = *it;
        if (cur_link->m_object_to){
            continue;
        }
        ret = cur_link;
        break;
    }
    return ret;
}

void UiObjectObject::processHits(
    GLint   hits,
    GLuint  buffer[])
{
    unsigned int    i, j;
    GLuint          names_count, *ptr;

    LinksConstIt                            it1, it2;
    vector<UiObjectObjectSptr>              neighs;
    vector<UiObjectObjectSptr>::iterator    neighs_it;

    PDEBUG (10, "----- hits %d -----\n", hits);

    ptr = (GLuint *) buffer;
    for (i = 0; i < (uint32_t)hits; i++){
        vector<int32_t> names;
        int32_t             id = 0;
        UiObjectSptr        object;
        UiObjectObjectSptr  object;

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
        object = dynamic_pointer_cast<UiObjectObject>(object);
        if (!object){
            // skip non object
            continue;
        }
        if (object.get() == this){
            // skip our self
            continue;
        }
        neighs.push_back(object);
    }

    if (!neighs.size()){
        goto out;
    }

    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        UiObjectObjectSptr  neigh = *neighs_it;
        UiObjectLineSptr    link;
        float               ax, ay, az;

        // -- direct link --
        link = getLink(neigh.get());
        if (link){
            // link already exist
            continue;
        }
        link = getFreeLink();
        if (!link){
            // We have not free links
            break;
        }

        link->m_object_to = neigh.get();
//        link->setCharge(m_charge);

        // get remote object position
        neigh->m_frame.getPosition(
            ax, ay, az
        );

        // setup our link to remote
        link->m_frame_to.setPosition(
            Vec(ax, ay, az)
        );

/*
        // -- back link --
        if (!neigh->getLink(this)){
            link = neigh->getFreeLink();
        }
        if (link){
            link->m_object_to = this;
            link->setCharge(m_charge);

            // get our object position
            m_frame.getPosition(ax, ay, az);

            // setup remote link to our
            link->m_frame_to.setPosition(
                Vec(ax, ay, az)
            );
        }
*/
    }

out:
    return;
}

void UiObjectObject::selectObjects()
{
    selectObjects(1, 0, 0);
    selectObjects(0, 1, 0);
    selectObjects(0, 0, 1);
}

void UiObjectObject::selectObjects(
    const float &a_x,
    const float &a_y,
    const float &a_z)
{
    GLuint  selectBuf[BUFSIZE];
    GLint   hits;
    float   x, y, z;

    m_frame.getPosition(
        x, y, z
    );

    glSelectBuffer (BUFSIZE, selectBuf);
    (void) glRenderMode (GL_SELECT);

    glInitNames ();
    glPushName (0);
    glPushMatrix ();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    // glOrtho(
    //     левая граница,  правая граница,
    //     нижняя граница, верхняя граница,
    //     расстояние до ближнего плана отсечения,
    //     расстояние до дальнего плана отсечения
    // );
    float max_distance = 2 * getViewer()->m_step  * fabs(m_charge)
        + 0.000001f; // never 0

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
    gluLookAt(x,y,z, 0,0,0, a_x, a_y, a_z);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    //getViewer()->drawWithNames();

    glPopMatrix ();
    glFlush ();

    hits = glRenderMode (GL_RENDER);
    processHits (hits, selectBuf);
}

void UiObjectObject::searchLinks()
{
    LinksConstIt it;

    for (it = m_links.begin();
        it != m_links.end();
        it++)
    {
        UiObjectLineSptr link = *it;
        if (!link->m_object_to){
            link->selectObjects();
        }
    }
}

void UiObjectObject::getConnectedLinks(
    vector<UiObjectLineSptr>    &a_out,
    UiObjectObject              *a_skip_object)
{
    LinksConstIt it;

    for (it = m_links.begin();
        it != m_links.end();
        it++)
    {
        UiObjectLineSptr    link        = *it;
        UiObjectObject      *object_to  = NULL;

        object_to = link->m_object_to;
        if (!object_to){
            continue;
        }
        if (    a_skip_object
            &&  a_skip_object == object_to)
        {
            continue;
        }
        a_out.push_back(link);
    }
}

void UiObjectObject::setCharge(
    const float &a_charge)
{
    float       ax, ay, az;
    float       bx, by, bz;
    float       cx, cy, cz;

    UiViewer            *viewer = getViewer();
    LinksIt             links_it;
    UiObjectObjectSptr  object = viewer->m_object_object;
    UiObjectsObjectsIt  it;

    if ((void *)object.get() == this){
        PFATAL("attempt to set charge for root object\n");
    }

    // store charge
    m_charge = a_charge;
    normalize();

    object->m_frame.getPosition(
        ax, ay, az
    );

    if (!object->m_color){
        ColorSptr color(new Color);
        color->push_back(1);
        color->push_back(1);
        color->push_back(1);
        object->m_color = color;
    }

    m_frame.getPosition(bx, by, bz);

    //float len = sqrtf(
    //    (bx-ax)*(bx-ax) + (by-ay)*(by-ay) + (bz-az)*(bz-az)
    //);
    //float step      = 0; //len / (m_charge + object->m_charge + 0.000001f);
    //float charge    = 0;
    //for (it = viewer->m_objects_objects.begin();
    //    it != viewer->m_objects_objects.end();
    //    it++)
    //{
    //    charge += it->second->m_charge;
    //}
    //step = 0.8f/(charge +0.000001f);


    float k = m_charge/(viewer->m_total_charge + 0.000001f);
    //PWARN("k = '%f'\n", k);
    if (k > 1){
        k = 1;
    }

    cx = ax + (bx-ax)*k;
    cy = ay + (by-ay)*k;
    cz = az + (bz-az)*k;

    //object->m_charge = m_charge;

/*
    PWARN("pos: '%f'\n", pos);
    PWARN("ax: '%f', ay: '%f', az: '%f'\n",
        ax, ay, az
    );
    PWARN("bx: '%f', by: '%f', bz: '%f'\n",
        bx, by, bz
    );
    PWARN("cx: '%f', cy: '%f', cz: '%f'\n",
        cx, cy, cz
    );
    PWARN("MORIK len: '%f', m_charge: '%f'\n", len, m_charge);
    if (    0 == object->m_object->morik_row
        &&  0 == object->m_object->morik_col)
    {
    //    PWARN("MORIK len: '%f', m_charge: '%f'\n", len, m_charge);
    }
    if (    0  == object->m_object->morik_row
        &&  79 == object->m_object->morik_col)
    {
    //    PWARN("MORIK len: '%f', m_charge: '%f'\n", len, m_charge);
    }
    if (    0  == object->m_object->morik_row
        &&  39 == object->m_object->morik_col)
    {
    //    PWARN("MORIK len: '%f', m_charge: '%f'\n", len, m_charge);
    }
    if (    29 == object->m_object->morik_row
        &&  39 == object->m_object->morik_col)
    {
    //    PWARN("MORIK len: '%f', m_charge: '%f'\n", len, m_charge);
    }
*/

    object->m_frame.setPosition(
        Vec(cx, cy, cz)
    );

    // add tail
    if (TAIL_PERIOD && ++tail >= TAIL_PERIOD){
        tail = 0;
        UiObjectObjectSptr tail;
        tail = UiObjectObjectSptr(new UiObjectObject(
            getViewer()
        ));
        tail->m_frame.setPosition(
            Vec(cx, cy, cz)
        );
        tail->m_charge = -0.9f;
        getViewer()->m_objects.push_back(tail);
    }

    for (links_it = object->m_links.begin();
        links_it != object->m_links.end();
        links_it++)
    {
        UiObjectLineSptr link = *links_it;
        link->m_frame_from.setPosition(
            Vec(cx, cy, cz)
        );
    }
}

void UiObjectObject::selected()
{
    PWARN("object object selected\n");

    float ax, ay, az;
    m_frame.getPosition(ax, ay, az);

/*
    if (m_object){
        PWARN("selected object row: '%d', col: '%d',"
            " ax: '%f', ay: '%f', az: '%f'\n",
            m_object->morik_row,
            m_object->morik_col,
            ax, ay, az
        );
    }
*/

    inc();

    if (0 && m_object){
        QMessageBox::information(
            0, "Selection",
            QString::fromUtf8(m_object->serialize().c_str())
        );
    }
}


