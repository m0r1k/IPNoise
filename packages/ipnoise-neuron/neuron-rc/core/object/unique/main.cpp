#include "core/object/object/main.hpp"
#include "core/object/dir/main.hpp"
#include "core/object/file/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/vector/main.hpp"

#include "main.hpp"

// ---------------- ObjectUniqueIt ----------------

ObjectUniqueIterator::ObjectUniqueIterator()
{
}

ObjectUniqueIterator::~ObjectUniqueIterator()
{
}

// !=
bool ObjectUniqueIterator::operator != (
    const ObjectUniqueIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectUniqueIterator::operator != (
    const ObjectUniqueConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// ==
bool ObjectUniqueIterator::operator == (
    const ObjectUniqueIterator &a_right) const
{
    return m_it == a_right.m_it;
}

bool ObjectUniqueIterator::operator == (
    const ObjectUniqueConstIterator &a_right) const
{
    return m_it == a_right.m_it;
}

// =
ObjectUniqueIterator & ObjectUniqueIterator::operator = (
    const ObjectUniqueIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectUniqueIterator::operator ++ (int32_t)
{
    m_it++;
}

// *
ObjectSptr ObjectUniqueIterator::operator * () const
{
    return m_it->first;
}

// ->
ObjectMapIt ObjectUniqueIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectUniqueConstIt ----------------

ObjectUniqueConstIterator::ObjectUniqueConstIterator()
{
}

ObjectUniqueConstIterator::~ObjectUniqueConstIterator()
{
}

// !=
bool ObjectUniqueConstIterator::operator != (
    const ObjectUniqueConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectUniqueConstIterator::operator != (
    const ObjectUniqueIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// =
ObjectUniqueConstIterator & ObjectUniqueConstIterator::operator = (
    const ObjectUniqueConstIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectUniqueConstIterator::operator ++ (int32_t)
{
    m_it++;
}

// *
ObjectSptr ObjectUniqueConstIterator::operator * () const
{
    return m_it->first;
}

// ->
ObjectMapConstIt ObjectUniqueConstIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectUnique ----------------

ObjectUnique::ObjectUnique(
    const CreateFlags   &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectUnique::~ObjectUnique()
{
}

string ObjectUnique::getType()
{
    return ObjectUnique::s_getType();
}

int32_t ObjectUnique::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    m_vals = OBJECT<ObjectMap>(CREATE_PROP);

    return err;
}

int32_t ObjectUnique::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    m_vals = OBJECT<ObjectMap>();

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

ObjectSptr  ObjectUnique::copy()
{
    ObjectUniqueSptr    ret;
    ObjectMapIt         it;

    ret = OBJECT<ObjectUnique>();

    for (it = m_vals->begin();
        it != m_vals->end();
        it++)
    {
        ObjectSptr key = it->first;
        ret->add(key->copy());
    }

    return ret;
}

void ObjectUnique::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

void ObjectUnique::add(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr empty;

    empty = OBJECT<ObjectString>();
    m_vals->add(a_val, empty);

    setDirty(1);
}

void ObjectUnique::add(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_val;

    prop_val = OBJECT<ObjectString>(a_val);
    add(prop_val);
}

void ObjectUnique::add(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectInt32Sptr  prop_val;

    prop_val = OBJECT<ObjectInt32>(a_val);
    add(prop_val);
}

bool ObjectUnique::has(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals->has(a_key);
}

bool ObjectUnique::has(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals->has(a_key);
}

uint32_t ObjectUnique::size()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals->size();
}

ObjectStringSptr ObjectUnique::toString()
{
    return serializeAsProp();
}

ObjectStringSptr ObjectUnique::serializeAsProp(
    const char *a_delim)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr    ret;
    ObjectMapIt         it;

    ret = OBJECT<ObjectString>();

    for (it = m_vals->begin();
        it != m_vals->end();
        it++)
    {
        ObjectSptr key_object = it->first;

        if (!ret->empty()){
            ret->add(", ");
        }

        ret->add(
            key_object->serializeAsProp(a_delim)
        );
    }


    {
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_begin");
        ret->add("[ ", flags);
    }

    {
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_begin");
        ret->add(" ]", flags);
    }

    return ret;
}

int32_t ObjectUnique::loadAsProp(
    const char  *a_dir)
{
    int32_t         err         = 0;
    DIR             *dirp       = NULL;
    struct dirent   *dir_ent    = NULL;
    string          dir         = getObjectPath();
    string          vals_path;

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot load object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    if ('/' != dir.at(dir.size() - 1)){
        // add '/' to end if not exist
        dir += "/";
    }

    // load vals
    vals_path = dir;
    vals_path += OBJECT_DIR_VALS;

    dirp = opendir(vals_path.c_str());
    if (!dirp){
        goto out;
    }

    do {
        string      val_path;

        dir_ent = readdir(dirp);
        if (!dir_ent){
            break;
        }

        if (    !strcmp(dir_ent->d_name, ".")
            ||  !strcmp(dir_ent->d_name, ".."))
        {
            continue;
        }

        val_path = vals_path;
        val_path += dir_ent->d_name;

        PDEBUG(50, "object: '%s', was found val: '%s'\n",
            dir.c_str(),
            val_path.c_str()
        );

        add(dir_ent->d_name);
    } while (dir_ent);

out:
    if (dirp){
        closedir(dirp);
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectUnique::saveAsProp(
    const char  *a_dir)
{
    int32_t         res, ret    = -1;
    string          dir         = getObjectPath();
    ObjectMapIt     vals_it;
    string          vals_path;

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot save object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    if ('/' != dir.at(dir.size() - 1)){
        // add '/'to end if not exist
        dir += "/";
    }

    PDEBUG(15, "attempt to save object to dir: '%s'\n",
        dir.c_str()
    );

    // check what object's dir exist
    res = ObjectDir::s_mkdir(dir.c_str());
    if (res){
        PERROR("cannot crate dir: '%s'\n", dir.c_str());
        goto fail;
    }

    vals_path = dir;
    vals_path += OBJECT_DIR_VALS;

    // recreate vals dir
    res = ObjectDir::s_recreate_dir(vals_path.c_str());
    if (res){
        PERROR("cannot recreate dir: '%s', res: '%d'\n",
            vals_path.c_str(),
            res
        );
    }

    for (vals_it = m_vals->begin();
        vals_it != m_vals->end();
        vals_it++)
    {
        ObjectStringSptr    key;
        ObjectFileSptr      file;

        key = dynamic_pointer_cast<ObjectString>(
            vals_it->first
        );

        file = OBJECT<ObjectFile>();
        file->setDir(vals_path.c_str());
        file->setFileName(key->c_str());

        // create empty file
        res = file->touch();
        if (res){
            PFATAL("cannot touch file: '%s' in dir: '%s'\n",
                key->c_str(),
                vals_path.c_str()
            );
        }
    }

    // all ok
    ret = 0;

out:
    return ret;
fail:
    goto out;
}

// ---------------- begin ----------------

ObjectUnique::iterator ObjectUnique::begin() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectUnique::iterator it;
    it.m_it = m_vals->begin();
    return it;
}

// ---------------- cbegin ----------------

ObjectUnique::const_iterator ObjectUnique::cbegin() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectUnique::const_iterator it;
    it.m_it = m_vals->cbegin();
    return it;
}

// ---------------- end ----------------

ObjectUnique::iterator ObjectUnique::end() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectUnique::iterator it;
    it.m_it = m_vals->end();
    return it;
}

// ---------------- cend ----------------

ObjectUnique::const_iterator ObjectUnique::cend() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectUnique::const_iterator it;
    it.m_it = m_vals->cend();
    return it;
}

// ---------------- find ----------------

ObjectUnique::iterator ObjectUnique::find(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectUnique::iterator it;
    it.m_it = m_vals->find(a_key);
    return it;
}

ObjectUnique::iterator ObjectUnique::find(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectUnique::iterator it;
    ObjectStringSptr    prop_key;

    prop_key = OBJECT<ObjectString>(a_key);
    it.m_it  = m_vals->find(prop_key);

    return it;
}

// ---------------- erase ----------------

void ObjectUnique::erase(
    const ObjectUnique::iterator &a_it)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals->erase(a_it.m_it);
    setDirty(1);
}

void ObjectUnique::erase(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals->erase(a_val);
    setDirty(1);
}

void ObjectUnique::erase(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr val;

    val = OBJECT<ObjectString>(a_val);
    erase(val);
}

void ObjectUnique::erase(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectInt32Sptr val;

    val = OBJECT<ObjectInt32>(a_val);
    erase(val);
}

void ObjectUnique::clear()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals->clear();
    setDirty(1);
}

void ObjectUnique::getVals(
    ObjectVectorSptr    a_out)
{
    ObjectMapIt it;

    for (it = m_vals->begin();
        it != m_vals->end();
        it++)
    {
        ObjectSptr key = it->first;
        a_out->add(key);
    }
}

ObjectSptr ObjectUnique::get(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr  ret;
    ObjectMapIt it;

    it = m_vals->find(a_val);
    if (m_vals->end() != it){
        ret = it->first;
    }

    return ret;
}

ObjectSptr ObjectUnique::get(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr          ret;
    ObjectStringSptr    val;

    val = OBJECT<ObjectString>(a_val);

    ret = get(val);

    return ret;
}

bool ObjectUnique::empty()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals->empty();
}

// ---------------- static ----------------

// ---------------- tests ----------------

int32_t ObjectUnique::s_test_save(
    const char *a_tmp_path)
{
    int32_t             err = -1;
    ObjectUniqueSptr    unique;
    string              path;

    path = a_tmp_path;
    path += "test_save";

    unique = OBJECT<ObjectUnique>();
    // fill key(s)
    unique->add("key1");
    // fill key(s)
    unique->add("key1");
    unique->add("key2");
    // fill key(s)
    unique->add("key1");
    unique->add("key2");
    unique->add("key3");

    // save object
    err = unique->save(path.c_str());
    PDEBUG(15, "after save unique to: '%s', err: '%d'\n",
        path.c_str(),
        err
    );

    return err;
}

int32_t ObjectUnique::s_test_load(
    const char *a_tmp_path)
{
    int32_t             err = -1;
    ObjectUniqueSptr    unique;
    string              path;

    path = a_tmp_path;
    path += "test_save";

    unique = OBJECT<ObjectUnique>();

    // load object
    err = unique->load(path.c_str());
    PDEBUG(15, "after load unique from: '%s', err: '%d'\n",
        path.c_str(),
        err
    );

    if (3 != unique->size()){
        PERROR("keys count != 3\n");
        goto fail;
    }

    if (!unique->has("key1")){
        PERROR("key1 not found\n");
        goto fail;
    }

    if (!unique->has("key2")){
        PERROR("key2 not found\n");
        goto fail;
    }

    if (!unique->has("key3")){
        PERROR("key3 not found\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

// ---------------- module ----------------

string ObjectUnique::s_getType()
{
    return "core.object.unique";
}

int32_t ObjectUnique::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectUnique::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectUnique::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectUnique(a_create_flags));

    return object;
}

void ObjectUnique::s_getTests(
    Tests &a_out)
{
    TestInfo    test;

    test = TestInfo();
    test.name         = "test save";
    test.success_code = 0;
    test.run          = ObjectUnique::s_test_save;
    a_out.push_back(test);

    test = TestInfo();
    test.name         = "test load";
    test.success_code = 0;
    test.run          = ObjectUnique::s_test_load;
    a_out.push_back(test);
}

ModuleInfo core_object_unique = {
    .getType        = ObjectUnique::s_getType,
    .init           = ObjectUnique::s_init,
    .shutdown       = ObjectUnique::s_shutdown,
    .objectCreate   = ObjectUnique::s_objectCreate,
    .getTests       = ObjectUnique::s_getTests
};

