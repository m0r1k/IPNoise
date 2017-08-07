#ifndef CLASS_HPP
#define CLASS_HPP

#include <memory>

#define DEFINE_CLASS(name)                                      \
    class name;                                                 \
    typedef std::shared_ptr<name>       name  ## Sptr;          \
    typedef std::weak_ptr<name>         name  ## Wptr;

#endif

