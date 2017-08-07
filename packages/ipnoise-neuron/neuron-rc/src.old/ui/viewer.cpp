#include "thread.hpp"
#include "network.hpp"
#include "utils.hpp"
#include "object/object.hpp"
#include "object/object/id.hpp"
#include "object_object.hpp"
#include "object_line.hpp"

#include "core/object/console/main.hpp"

//MORIK #include "ui/nerves/input/video/webcam.hpp"
//MORIK #include "ui/nerves/input/video/test.hpp"

//#include <QGLFunctions>

#include "viewer.hpp"

using namespace qglviewer;

UiObjectObjectSptr g_object;

float step = 1000.025f;

UiViewer::UiViewer(
    Network *a_network)
    :   QGLViewer()
{
    m_network       = a_network;
    m_step          = 0.0f;
    m_total_charge  = 0.0f;
    m_state         = STATE_NORMAL;
    m_selected      = OBJECT_MAP();
    m_show_console  = 0;

    m_cmd_status    = "test";

//    REGISTER_NERVES_INPUT("eye",      NervesInputVideoWebCam);
//    REGISTER_NERVES_INPUT("eye_test", NervesInputVideoTest);

//    REGISTER_MODEL("sphere1", UiModelSphere);

    // - create_sphere();
    // load_sphere();

    // - create_cube();
    // load_cube();

    {
        // QTimer::singleShot(2000, this, SLOT(updateLinks()));
        m_timer = new QTimer();
        m_timer->setInterval(1000);
        connect(
            m_timer, SIGNAL(timeout()),
            this,    SLOT(timer())
        );
        m_timer->start();
    }


    // cmd commands
    registerCmdHandler(
        "resume",
        UiViewer::processCommandResume
    );
    registerCmdHandler(
        "pause",
        UiViewer::processCommandPause
    );

    //glutInitWindowSize(1024, 768);
}

UiViewer::~UiViewer()
{
}

void UiViewer::timer()
{
// MORIK    if (!m_video){
// MORIK        m_video = VideoWebCamSptr(new VideoWebCam(this));
// MORIK        //m_video = VideoTestSptr(new VideoTest(this));
// MORIK    }
    //displayMessage("TEST");
    //drawText(100, 100, "TEST");

    if (STATE_PAUSED != m_state){
// MORIK        m_video->process();
    }
//    repaint();
}

/*
void UiViewer::updateLinks()
{
    UiObjectsIt objects_it;

    for (objects_it = m_objects.begin();
        objects_it != m_objects.end();
        objects_it++)
    {
        UiObjectLineSptr link;
        link = dynamic_pointer_cast<UiObjectLine>(
            *objects_it
        );
        if (!link){
            continue;
        }
        link->selectObjects();
    }
}

void UiViewer::create_cube(
    ObjectSptr  a_object,
    int32_t     a_deep)
{
    char        buffer[1024] = { 0x00 };
    int32_t     i;
    int32_t     max         = 0;
    ObjectSptr  object      = a_object;
    ObjectSptr  new_object;

    if (!object){
        object = ThreadDb::get()->getObjectById(ObjectIdSptr(
            new ObjectId(
                "god"
            )
        ));
    }

    snprintf(buffer, sizeof(buffer), "level%d", a_deep);
    object->setDescr(PROP_STRING(
        buffer
    ));

    if (0 == a_deep){
        max = 10;
    } else if (1 == a_deep){
        max = 10;
    } else if (2 == a_deep){
        max = 2;
    } else if (3 == a_deep){
        max = 0;
    }

    for (i = 0; i < max; i++){
        new_object = db->createObject<Object>();
        LINK_AND_SAVE(object.get(), new_object.get());
        // create(new_object, a_deep + 1);
    }
}

ObjectSptr UiViewer::create_sphere(
    ObjectSptr  a_object,
    int32_t     a_deep)
{
    char        buffer[1024] = { 0x00 };
    int32_t     row     = 0;
    int32_t     col     = 0;
    int32_t     max     = YUE_HEIGHT * YUE_WIDTH;
    ObjectSptr  object  = a_object;

    if (!object){
        max++;
        object = ThreadDb::get()->createObject();
        object->setId(ObjectIdSptr(
            new ObjectId(
                "sphere"
            )
        ));
    }

    snprintf(buffer, sizeof(buffer), "level%d", a_deep);
    object->setDescr(PROP_STRING(
        buffer
    ));

    PWARN("will created: '%d' objects\n", max);

    for (row = 0; row < YUE_HEIGHT; row++){
        for (col = 0; col < YUE_WIDTH; col++){
            ObjectSptr new_object = db->createObject<Object>();
            new_object->morik_row = row;
            new_object->morik_col = col;
            LINK(object.get(), new_object.get());
        }
    }

    return object;
}

void UiViewer::load_cube(
    ObjectSptr  a_object,
    LoadInfo    a_info)
{
    const int32_t       nb = 3;
    int32_t             i, j, m;

    m_step = 1/float(2*nb + 1);

    for (i = -nb; i <= nb; ++i){
        for (j = -nb; j <= nb; ++j){
            for (m = -nb; m <= nb; ++m){
                UiObjectObjectSptr  object_object;
                UiObjectLineSptr    object_line;

                float ax, ay, az;
                float bx, by, bz;
                float ang_x, ang_y;
                float lo    = -90;
                float hi    = +90;

                // create object
                object_object = UiObjectObjectSptr(
                    new UiObjectObject(this)
                );
                ax = i/float(nb);
                ay = j/float(nb);
                az = m/float(nb);
                object_object->m_frame.setPosition(
                    Vec(ax, ay, az)
                );
                m_objects.push_back(object_object);

                // store first object to global
                if (!g_object){
                    g_object = object_object;
                }

                // create link
                for (int o = 0; o < 10; o++){
                    object_line = UiObjectLineSptr(
                        new UiObjectLine(this)
                    );
                    object_object->m_links.push_back(object_line);

                    object_line->m_object_from = object_object.get();
                    object_line->m_frame_from.setPosition(
                        Vec(ax, ay, az)
                    );

                    ang_x = lo + (float)rand()
                        /   ((float)RAND_MAX/(hi - lo));
                    ang_y = lo + (float)rand()
                        /   ((float)RAND_MAX/(hi - lo));

                    bx = ax + sin(ang_y) * cos(ang_x) * m_step;
                    by = ay + sin(0 - ang_x)          * m_step;
                    bz = az + cos(ang_y) * cos(ang_x) * m_step;

                    object_line->m_frame_to.setPosition(
                        Vec(bx, by, bz)
                    );
                    m_objects.push_back(object_line);
                }
            }
        }
    }

    return;
}

void UiViewer::load_sphere(
    ObjectSptr  a_object,
    LoadInfo    a_info)
{
    ObjectSptr  object   = a_object;

    vector<ObjectSptr>              neighs;
    vector<ObjectSptr>::iterator    neighs_it;

    float x, y, z, ang_x, ang_y;
    ObjectsIt           it;
    string              id;
    UiObjectObjectSptr  object_object;
    uint32_t            index = 0;

    if (!object){
        object = ThreadDb::get()->getObjectById(ObjectIdSptr(
            new ObjectId(
              "sphere"
            )
        ));
        if (!object){
            object = create_sphere();
        }
    }

    id = object->getId()->toString();
    it = m_objects.find(id);
    if (m_objects.end() != it){
        // already loaded
        goto out;
    }

    // create ui object
    index = (object->morik_row << 16)
        + object->morik_col;
    object_object = UiObjectObjectSptr(new UiObjectObject(
        this));
    object_object->m_object  = object;
    m_objects.push_back(object_object);

    m_objects_objects[index] = object_object;

    if (!a_object){
        // store root object to different variable too
        m_object_object = object_object;
    }

    // store us in hash by name (for uniq checking)
    m_objects[id] = object;

    ang_x = a_info.m_ang_x;
    ang_y = a_info.m_ang_y;

    // ang_x = a_info.m_ang_x + a_info.m_ang_x_lo;
    // if (a_info.m_ang_x_hi - a_info.m_ang_x_lo){
    //     ang_x += (float)rand()
    //         /   ((float)RAND_MAX/
    //             (a_info.m_ang_x_hi - a_info.m_ang_x_lo));
    // }
    // ang_y = a_info.m_ang_y + a_info.m_ang_y_lo;
    // if (a_info.m_ang_y_hi - a_info.m_ang_y_lo){
    //     ang_y += (float)rand()
    //         /   ((float)RAND_MAX/
    //             (a_info.m_ang_y_hi - a_info.m_ang_y_lo));
    // }
    // 
    // ang_x = a_info.m_ang_x + 10;
    // ang_y = 10; // MORIK a_info.m_ang_y + 10;
    // 
    // 
    // PWARN("ang_x: '%f', ang_y: '%f', a_info.m_radius: '%f'"
    //     "row: '%d', col: '%d'\n",
    //     ang_x, ang_y, a_info.m_radius,
    //     object->morik_row,
    //     object->morik_col
    // );

    x = sin(ang_y) * cos(ang_x)   * a_info.m_radius;
    y = sin(0 - ang_x)            * a_info.m_radius;
    z = cos(ang_y) * cos(ang_x)   * a_info.m_radius;

    object_object->m_frame.setPosition(
        Vec(x, y, z)
    );

    // create line to parent
    if (0 && a_info.m_object){ // MORIK
        UiObjectLineSptr object_line(new UiObjectLine(this));
        object_line->m_frame_from.setPosition(
            Vec(
                a_info.m_x,
                a_info.m_y,
                a_info.m_z
            )
        );
        object_line->m_object_from = a_info.m_object.get();
        object_line->m_object_to   = object_object.get();
        object_line->m_frame_to.setPosition(
            Vec(x, y, z)
        );
        a_info.m_object->m_links.push_back(
            object_line
        );
        m_objects.push_back(object_line);
        object_object->m_links.push_back(object_line);
    }

    // prepare for next level
    a_info.m_radius     += 1.0f;
    a_info.m_level      += 1;
    a_info.m_x          = x;
    a_info.m_y          = y;
    a_info.m_z          = z;
    a_info.m_object     = object_object;

    // get neighs
    object->getNeighs<Object>(neighs);
    sort(neighs.begin(), neighs.end());
    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        ObjectSptr  cur_object = *neighs_it;
        string      cur_id;
        ObjectsIt   cur_it;
        LoadInfo    tmp;

        cur_id = cur_object->getId()->toString();
        cur_it = m_objects.find(cur_id);

        if (m_objects.end() != it){
            // already loaded
            continue;
        }

        ang_x = -1.5f + 3.0f*cur_object->morik_row/(float(YUE_HEIGHT));
        ang_y = -2.0f + 2.0f*cur_object->morik_col/(float(YUE_WIDTH));
        //ang_y = -2.0f + 6.3f*cur_object->morik_col/(float(YUE_WIDTH));

        // prepare for next level
        a_info.m_ang_x = ang_x;
        a_info.m_ang_y = ang_y;

        load_sphere(cur_object, a_info);
    }

out:
    return;
}
*/

void UiViewer::init()
{
    // A ManipulatedFrameSetConstraint
    // will apply displacements to the selection
    setManipulatedFrame(new ManipulatedFrame());
    manipulatedFrame()->setConstraint(
        new ManipulatedFrameSetConstraint()
    );

    // Used to display semi-transparent relection rectangle
    glBlendFunc(GL_ONE, GL_ONE);

    restoreStateFromFile();
//    help();

    m_console_object = m_network
        ->createObject<ObjectConsole>();
    m_console_object->setViewer(this);
}

QString UiViewer::helpString() const
{
    QString text;
    return text;
}

void UiViewer::draw1()
{
    ObjectSptr                      object;
    vector<ObjectSptr>              objects;
    vector<ObjectSptr>::iterator    objects_it;
    double                          x, y, z, r;

    FILE        *fin    = NULL;
    int32_t     res;

    // Draws all the objects.
    // Selected ones are not repainted
    // because of GL depth test.
    glColor3f(1.0f, 1.0f, 1.0f);

    PWARN("MORIK\n");
    fin = fopen ("write.dat","r");
    if (fin){
        do {
            res = fscanf(fin, "%lf %lf %lf %lf",
                &x, &y, &z, &r
            );
            if (res <= 0){
                break;
            }
            PWARN("%f %f %f %f\n",
                x, y, z, r
            );
            ObjectSptr object = m_network->createObject();
            object->setPos(x, y, z);
            object->setHumanVal(r);
            objects.push_back(object);
        } while (1);
        fclose (fin);
    }

    for (objects_it = objects.begin();
        objects_it != objects.end();
        objects_it++)
    {
        ObjectSptr      cur_object = *objects_it;
        ObjectStringSptr  object_nerve_path;
        string          nerve_path;
        ObjectDoubleSptr  object_ax;
        ObjectDoubleSptr  object_ay;
        ObjectDoubleSptr  object_az;
        ObjectDoubleSptr  object_cur_val;

        object_ax         = cur_object->getPosX();
        object_ay         = cur_object->getPosY();
        object_az         = cur_object->getPosZ();
        object_cur_val    = cur_object->getHumanVal();
        object_nerve_path = cur_object->getHumanNervePath();
        nerve_path      = object_nerve_path->toString();

        double ax = (*object_ax.get());
        double ay = (*object_ay.get());
        double az = (*object_az.get());
        double val = (*object_cur_val.get()) * 5;

        {
            Frame             m_frame;
            static GLUquadric *quad = gluNewQuadric();
            m_frame.setPosition(
                Vec(ax, ay, az)
            );

            glPushMatrix();
            glMultMatrixd(m_frame.matrix());
            glColor3f(val, 0, 0);
            gluSphere(quad, *object_cur_val.get(), 10, 6);
            glPopMatrix();
        }
    }

    return;
}

void UiViewer::_draw(
    const int32_t   &a_with_names)
{
    ObjectVectorIt    objects_it;
    int32_t         object_id = 1;

    // Draws all the objects.
    // Selected ones are not repainted
    // because of GL depth test.
    glColor3f(1.0f, 1.0f, 1.0f);

    if (    !m_objects
        ||  !m_objects->size())
    {
        goto out;
    }

    for (objects_it = m_objects->begin();
        objects_it != m_objects->end();
        objects_it++)
    {
        PropObjectSptr      object_object;
        ObjectIdSptr    object_object_id;
        ObjectSptr          object;
        string              object_id;
        int32_t             selected = 0;

        object_object = dynamic_pointer_cast<PropObject>(
            *objects_it
        );
        object          = object_object->getVal();
        object_object_id  = object->getId();
        object_id       = object_object_id->toString();

#ifdef UI
        if (a_with_names){
            object->renderOpenGl(
                object_id++,        // object name
                selected            // is object selected
            );
        } else {
            object->renderOpenGl(
                0,                  // object name
                selected            // is object selected
            );

        }
#endif

    }

out:
    return;
}

void UiViewer::draw()
{
    _draw();

    // Draws rectangular selection area.
    // Could be done in postDraw() instead.
    if (SELECT_MODE_NONE != m_select_mode){
        drawSelectionRectangle();
    }
}

ObjectSptr UiViewer::getObjectBySelectId(
    const int32_t &a_id)
{
    PropObjectSptr  object_object;
    ObjectSptr      object;

    if (0 >= a_id){
        goto out;
    }

    object_object = dynamic_pointer_cast<PropObject>(
        m_objects->get(a_id - 1)
    );
    object = object_object->getVal();

out:
    return object;
}

/*
void UiViewer::postSelection(
    const QPoint &a_point)
{
    PWARN("UiViewer::postSelection\n");

    ObjectSptr  object;
    int32_t     select_id   = -1;
    bool        found       = false;

    // Compute orig and dir, used to draw
    // a representation of the intersecting line
    camera()->convertClickToLine(a_point, m_orig, m_dir);

    // Find the selectedPoint coordinates,
    // using camera()->pointUnderPixel().
    m_selected_point = camera()->pointUnderPixel(
        a_point,
        found
    );

    // Small offset to make point clearly visible.
    m_selected_point -= 0.01f * m_dir;

    // Note that "found" is different from
    // (selectedObjectId()>=0) because of the size
    // of the select region.

    select_id = selectedName();
    if (0 <= select_id){
        object = getObjectBySelectId(select_id);
    }

    if (object){
        PWARN("post selected object ID: '%s'\n",
            object->getId()->toString().c_str()
        );
    }
}
*/

void UiViewer::drawWithNames()
{
    PWARN("UiViewer::drawWithNames\n");
    if (!m_objects){
        goto out;
    }

    _draw(1);

out:
    return;
}

void UiViewer::endSelection(
    const QPoint    &)
{
    // Flush GL buffers
    glFlush();

    // Get the number of objects that were seen
    // through the pick matrix frustum. Reset GL_RENDER mode.
    GLint nbHits = glRenderMode(GL_RENDER);

    if (0 < nbHits){
        // Interpret results : each object created 4 values in the selectBuffer().
        // (selectBuffer())[4*i+3] is the id pushed on the stack.
        for (int i=0; i < nbHits; ++i){
            PWARN("UiViewer::endSelection"
                " m_select_mode: '%d',"
                " hit No: '%d'\n",
                m_select_mode,
                i
            );
            switch (m_select_mode){
                case SELECT_MODE_ADD:
                    addIdToSelection((selectBuffer())[4*i+3]);
                    break;

                case SELECT_MODE_REMOVE:
                    removeIdFromSelection((selectBuffer())[4*i+3]);
                    break;

                default:
                    break;
            }
        }
    }

    m_select_mode = SELECT_MODE_NONE;
}

void UiViewer::registerCmdHandler(
    const string    &a_name,
    void (*a_handler)(UiViewer *, string &))
{
    m_cmd_handlers[a_name] = a_handler;
}

void UiViewer::processCommandPause(
    UiViewer        *a_viewer,
    string          &a_out)
{
    a_viewer->m_cmd_status  = "paused";
    a_viewer->m_state       = STATE_PAUSED;
}

void UiViewer::processCommandResume(
    UiViewer        *a_viewer,
    string          &a_out)
{
    a_viewer->m_cmd_status  = "";
    a_viewer->m_state       = STATE_NORMAL;
}

void UiViewer::processCommand(
    const string &a_cmd)
{
    HandlersIt m_cmd_handlers_it;

    PWARN("MORIK executing: '%s'\n",
        a_cmd.c_str()
    );

    m_cmd_handlers_it = m_cmd_handlers.find(a_cmd);
    if (m_cmd_handlers.end() != m_cmd_handlers_it){
        (*m_cmd_handlers_it->second)(
            this,
            m_cmd_status
        );
    } else {
        m_cmd_status = "no such command";
    }
}

void UiViewer::setCenterObject(
    ObjectSptr a_object)
{
    m_center_object = a_object;
}

void UiViewer::setObjects(
    ObjectVectorSptr a_objects)
{
    m_objects = a_objects;
}

void UiViewer::keyReleaseEvent(
    QKeyEvent *a_ev)
{
    PWARN("UiViewer::keyReleaseEvent\n");
    QGLViewer::keyReleaseEvent(a_ev);
}


int32_t UiViewer::consoleIsHidden()
{
    int32_t         ret = 1;
    ObjectVectorIt    it;
    string          console_object_id;
    PropObjectSptr  object_object;
    ObjectSptr      object;
    string          object_id;

    if (!m_console_object){
        goto out;
    }

    if (    !m_objects
        ||  !m_objects->size())
    {
        goto out;
    }

    console_object_id = m_console_object->getId()->toString();

    object_object = dynamic_pointer_cast<PropObject>(
        m_objects->at(m_objects->size() - 1)
    );
    object      = object_object->getVal();
    object_id   = object->getId()->toString();

    if (console_object_id == object_id){
        ret = 0;
    }

out:
    return ret;
}

void UiViewer::consoleShow()
{
    int32_t res;

    res = consoleIsHidden();
    if (res){
        m_objects->push_back(OBJECT(
            m_console_object
        ));
    }
}

void UiViewer::consoleHide()
{
    int32_t res;

    // TODO XXX remove from here
    // reset selection
    m_selected = OBJECT_MAP();

    res = consoleIsHidden();
    if (!res){
        m_objects->pop_back();
    }
}

int32_t UiViewer::isSelected(
    ObjectSptr  a_object)
{
    int32_t             ret = 0;
    ObjectIdSptr    object_object_id;
    string              object_id;

    if (!a_object){
        goto out;
    }

    object_object_id  = a_object->getId();
    object_id       = object_object_id->toString();

    if (m_selected->end() != m_selected->find(object_id)){
        ret = 1;
    };

out:
    return ret;
}

void UiViewer::keyPressEvent(
    QKeyEvent *a_ev)
{
    int32_t code = a_ev->key();
    string  text = a_ev->text().toStdString();

    PDEBUG(20, "QKeyEvent: code: 0x'%x', text: '%s'\n",
        code,
        text.c_str()
    );

    PWARN("QKeyEvent: code: 0x'%x', text: '%s'\n",
        code,
        text.c_str()
    );

    // ` - 0x60
    if (0x60 == code){
        if (consoleIsHidden()){
            consoleShow();
        } else {
            consoleHide();
        }
        goto out;
    }

    if (    !consoleIsHidden()
        &&  isSelected(m_console_object))
    {

        m_console_object->keyPressEvent(a_ev);
    } else {
        QGLViewer::keyPressEvent(a_ev);
    }

out:
    repaint();
    return;
}

void UiViewer::To3D()
{
    glMatrixMode(GL_PROJECTION);  //Switch to projection matrix
    glPopMatrix();                // Restore the old projection matrix
    glMatrixMode(GL_MODELVIEW);   // Return to modelview matrix
    glPopMatrix();                // Restore old modelview matrix
    glEnable(GL_TEXTURE_2D);      // Turn on textures, don't want our text textured
    glEnable(GL_LIGHTING);
    glPopAttrib();                // Restore depth testing
}

void UiViewer::To2D()
{
    glPushAttrib(GL_DEPTH_TEST);  // Save the current Depth test settings (Used for blending )
    glDisable(GL_DEPTH_TEST);     // Turn off depth testing (otherwise we get no FPS)
    glDisable(GL_TEXTURE_2D);     // Turn off textures, don't want our text textured
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);  // Switch to the projection matrix
    glPushMatrix();               // Save current projection matrix
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);  // Change the projection matrix using an orthgraphic projection
    glMatrixMode(GL_MODELVIEW);  // Return to the modelview matrix
    glPushMatrix();              // Save the current modelview matrix
    glLoadIdentity();
}
#define BUFSIZE 1*1024*1024

void UiViewer::processHits(
    GLint   hits,
    GLuint  buffer[])
{
//    unsigned int i, j;
//    GLuint names_count, *ptr;

//    vector<UiObjectLineSptr>            links_our;
//    vector<UiObjectLineSptr>::iterator  links_our_it;
//    vector<UiObjectLineSptr>            links_foreign;
//    vector<UiObjectLineSptr>::iterator  links_foreign_it;

    PWARN("----- hits %d -----\n", hits);
/*
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
*/
}

void UiViewer::getNeighObjects(
    ObjectSptr          a_object,
    vector<ObjectSptr>  &a_out,
    double              a_radius)
{
    GLuint  selectBuf[BUFSIZE];
    GLint   hits;

    double  x = *a_object->getPosX();
    double  y = *a_object->getPosY();
    double  z = *a_object->getPosZ();

    // задаём массив для возвращения записей о попаданиях
    glSelectBuffer(BUFSIZE, selectBuf);

    // переходим в режим выбора
    (void)glRenderMode(GL_SELECT);

    // инициализируем стек имен
    glInitNames();
    glPushName(0);

    // сохраняем текущее состояние преобразований
    glPushMatrix();

    // рисуем все по новой
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // отсекаем все, кроме указанной области
    // glOrtho(
    //     левая граница,  правая граница,
    //     нижняя граница, верхняя граница,
    //     расстояние до ближнего плана отсечения,
    //     расстояние до дальнего плана отсечения
    // );
    float max_distance = a_radius * 2;
    glOrtho(
        -max_distance, +max_distance,
        -max_distance, +max_distance,
        -max_distance, +max_distance
    );

    gluLookAt(0,0,0, 0,0,0, x,y,z);

    if (0){
        // для того, чтобы захватить только объекты,
        // видимые с определенного места, устанавливаем точку,
        // в которой находится наш глаз             --- (x,y,z)
        // направление, откуда                      --- (0,0,0)
        // и куда смотрим                           --- (0,1,0)
        // в данном случае - вектор принимаемый за направление вверх (ось Y)
        gluLookAt(x,y,z, 0,0,0, 0,1,0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawWithNames();

    // восстанавливаем предыдущее состояние преобразований
    glPopMatrix();
    glFlush();

    // собираем выделенные объекты
    hits = glRenderMode(GL_RENDER);

    // обрабатываем выделенные объекты
    processHits(hits, selectBuf);
}

void UiViewer::mousePressEvent(
    QMouseEvent *a_ev)
{
    PWARN("UiViewer::mousePressEvent\n");

    Qt::KeyboardModifiers modifiers = a_ev->modifiers();

    m_select_rectangle = QRect(a_ev->pos(), a_ev->pos());

    if (    (Qt::LeftButton     == a_ev->button()
        &&  (Qt::ShiftModifier  == modifiers)))
    {
        PWARN("SELECT_MODE_ADD\n");
        m_select_mode = SELECT_MODE_ADD;
        goto out;
    }

    if (    (Qt::LeftButton      == a_ev->button())
        &&  (Qt::ShiftModifier   & modifiers)
        &&  (Qt::ControlModifier & modifiers))
    {
        PWARN("SELECT_MODE_REMOVE\n");
        m_select_mode = SELECT_MODE_REMOVE;
        goto out;
    }

    QGLViewer::mousePressEvent(a_ev);

out:
    return;
}

void UiViewer::mouseMoveEvent(
    QMouseEvent *a_e)
{
    // PWARN("UiViewer::mouseMoveEvent\n");
    if (SELECT_MODE_NONE != m_select_mode){
        // Updates rectangle_ coordinates and redraws rectangle
        m_select_rectangle.setBottomRight(a_e->pos());
        updateGL();
    } else {
        QGLViewer::mouseMoveEvent(a_e);
    }
}

void UiViewer::mouseReleaseEvent(QMouseEvent *a_e)
{
    PWARN("UiViewer::mouseReleaseEvent\n");
    if (SELECT_MODE_NONE != m_select_mode){
        // Actual selection on the rectangular area.
        // Possibly swap left/right and top/bottom
        // to make rectangle_ valid.
        m_select_rectangle = m_select_rectangle.normalized();
        // Define selection window dimensions
        setSelectRegionWidth(m_select_rectangle.width());
        setSelectRegionHeight(m_select_rectangle.height());
        // Compute rectangle center and perform selection
        PWARN("call select\n");
        select(m_select_rectangle.center());
        // Update display to show new selected objects
        updateGL();
    } else {
        QGLViewer::mouseReleaseEvent(a_e);
    }
}

void UiViewer::addIdToSelection(
    int32_t a_id)
{
    PWARN("addIdToSelection: '%d'\n", a_id);

    ObjectSptr object;

    object = getObjectBySelectId(a_id);
    if (object){
        ObjectIdSptr id = object->getId();
        PWARN("add to selection object with ID: '%s'\n",
            id->toString().c_str()
        );
        m_selected->add(id, OBJECT(object));
    }
}

void UiViewer::removeIdFromSelection(
    int32_t a_id)
{
    PWARN("removeIdFromSelection: '%d'\n", a_id);

    ObjectSptr object;

    object = getObjectBySelectId(a_id);
    if (object){
        ObjectIdSptr id = object->getId();
        PWARN("remove from selection object with ID: '%s'\n",
            object->getId()->toString().c_str()
        );
        m_selected->erase(id);
    }
}

void UiViewer::drawSelectionRectangle() const
{
    startScreenCoordinatesSystem();
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    glColor4f(0.0, 0.0, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2i(
        m_select_rectangle.left(),
        m_select_rectangle.top()
    );
    glVertex2i(
        m_select_rectangle.right(),
        m_select_rectangle.top()
    );
    glVertex2i(
        m_select_rectangle.right(),
        m_select_rectangle.bottom()
    );
    glVertex2i(
        m_select_rectangle.left(),
        m_select_rectangle.bottom()
    );
    glEnd();

    glLineWidth(2.0);
    glColor4f(0.4f, 0.4f, 0.5f, 0.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(
        m_select_rectangle.left(),
        m_select_rectangle.top()
    );
    glVertex2i(
        m_select_rectangle.right(),
        m_select_rectangle.top()
    );
    glVertex2i(
        m_select_rectangle.right(),
        m_select_rectangle.bottom()
    );
    glVertex2i(
        m_select_rectangle.left(),
        m_select_rectangle.bottom()
    );
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    stopScreenCoordinatesSystem();
}

