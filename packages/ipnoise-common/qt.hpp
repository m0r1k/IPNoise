#ifndef QT_HPP
#define QT_HPP

#include <QtCore/QString>
#include <ipnoise-common/path.hpp>

using namespace std;

static QString absolutePathFromUnixPath(
    const QString &a_path)
{
    QString ret;
    Path    path;

    path.setPathUnix(a_path.toStdString());
#if defined(WIN32) || defined(WIN64)
    {
        wstring wstr;
        wstr = path.absolutePath16();
        ret.setUtf16(
            (const ushort *)wstr.c_str(),
            wstr.size()
        );
    }
#else
    ret = path.absolutePath().c_str();
#endif

    return ret;
}

#endif

