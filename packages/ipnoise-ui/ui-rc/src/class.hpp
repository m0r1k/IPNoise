#ifndef CLASS_HPP
#define CLASS_HPP

#include <memory>
#include <QObject>

#define DEFINE_CLASS(name)                      \
    class name;                                 \
    typedef std::shared_ptr<name>  name##Sptr;  \
    typedef std::weak_ptr<name>    name##Wptr;

using namespace std;

/*
class Class
    :   public QObject
{
    Q_OBJECT

    public:
};
*/

#endif

