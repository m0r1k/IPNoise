#ifndef UI_VIWER_HPP
#define UI_VIWER_HPP

#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <memory>

#include <QMouseEvent>
#include <qmessagebox.h>
#include <QGLViewer/qglviewer.h>
#include <QTimer>
#include <QGLShader>

#include "manipulatedFrameSetConstraint.hpp"

DEFINE_CLASS(Object);

#include "network.hpp"
#include "object/vector.hpp"
#include "object/map.hpp"
#include "object.hpp"

#define UI_VIWER_OBJECT_SPHERE_SIZE 0.01f

using namespace std;
using namespace qglviewer;

typedef vector<UiObjectSptr>                UiObjects;
typedef UiObjects::iterator                 UiObjectsIt;

typedef map<uint32_t, UiObjectObjectSptr>   UiObjectsObjects;
typedef UiObjectsObjects::iterator          UiObjectsObjectsIt;

class LoadInfo
{
    public:
        LoadInfo(){
            m_radius        = 0;
            m_level         = 0;
            m_x             = 0;
            m_ang_x         = 0;
            m_y             = 0;
            m_ang_y         = 0;
            m_z             = 0;
        }
        virtual ~LoadInfo(){
        };

        float               m_radius;
        int32_t             m_level;
        float               m_x;
        float               m_ang_x;
        float               m_y;
        float               m_ang_y;
        float               m_z;
        UiObjectObjectSptr  m_object;
};

typedef map<string, void (*)(UiViewer *, string &)> Handlers;
typedef Handlers::iterator                          HandlersIt;

class UiViewer
    :   public QGLViewer
{
    Q_OBJECT

    public:
        UiViewer(Network *);
        virtual ~UiViewer();

        enum State {
            STATE_ERROR = -1,
            STATE_NORMAL,
            STATE_PAUSED
        };

        enum SelectMode {
            SELECT_MODE_NONE    = 0,
            SELECT_MODE_ADD,
            SELECT_MODE_REMOVE
        };

        virtual void    draw();
        virtual void    init();
        virtual QString helpString() const;

        void _draw(
            const int32_t   &a_with_names = 0
        );
        virtual void    draw1();

        ObjectSptr  getObjectBySelectId(
            const int32_t &a_id
        );

        void registerCmdHandler(
            const string    &a_name,
            void (*a_handler)(UiViewer *, string &)
        );
        static void processCommandPause(
            UiViewer        *a_viewer,
            string          &a_out
        );
        static void processCommandResume(
            UiViewer        *a_viewer,
            string          &a_out
        );
        void processCommand(const string &);

        void setCenterObject(ObjectSptr);
        void setObjects(ObjectVectorSptr);

        void processHits(
            GLint   hits,
            GLuint  buffer[]
        );
        void getNeighObjects(
            ObjectSptr          a_object,
            vector<ObjectSptr>  &a_out,
            double              a_radius = UI_VIWER_OBJECT_SPHERE_SIZE
        );

        int32_t isSelected(ObjectSptr a_object);

        int32_t consoleIsHidden();
        void    consoleShow();
        void    consoleHide();

/*
        ObjectSptr create_sphere(
            ObjectSptr  a_object    = ObjectSptr(),
            int32_t     a_deep      = 0
        );
        void create_cube(
            ObjectSptr  a_object    = ObjectSptr(),
            int32_t     a_deep      = 0
        );
        void load_sphere(
            ObjectSptr  a_object    = ObjectSptr(),
            LoadInfo    a_info      = LoadInfo()
        );
        void load_cube(
            ObjectSptr  a_object    = ObjectSptr(),
            LoadInfo    a_info      = LoadInfo()
        );
*/

        void                To3D();
        void                To2D();

        UiObjects           m_objects;
        UiObjectsObjects    m_objects_objects;
        UiObjectObjectSptr  m_object_object;

//MORIK        VideoSptr           m_video;
        float               m_step;
        float               m_total_charge;

    protected slots:
//        void    updateLinks();
        void    timer();

    protected:
        // Selection functions
        virtual void    drawWithNames();
        virtual void    endSelection(const QPoint &);
//        virtual void    postSelection(const QPoint &);
        virtual void    keyPressEvent(QKeyEvent *);
        virtual void    keyReleaseEvent(QKeyEvent *);

        // Mouse events functions
        virtual void    mousePressEvent(QMouseEvent *e);
        virtual void    mouseMoveEvent(QMouseEvent *e);
        virtual void    mouseReleaseEvent(QMouseEvent *e);

    private:
        void    drawSelectionRectangle() const;
        void    addIdToSelection(int id);
        void    removeIdFromSelection(int id);

        Vec         m_orig;
        Vec         m_dir;
        Vec         m_selected_point;

        Network     *m_network;
        QTimer      *m_timer;

        string      m_cmd_line;
        string      m_cmd_status;

        State       m_state;
        Handlers    m_cmd_handlers;

        ObjectVectorSptr  m_objects;
        ObjectSptr      m_center_object;

        // selection
        QRect           m_select_rectangle;
        SelectMode      m_select_mode;
        ObjectMapSptr     m_selected;

        // console
        int32_t         m_show_console;
        ObjectSptr      m_console_object;
};

#endif

