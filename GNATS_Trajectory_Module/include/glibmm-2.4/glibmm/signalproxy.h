/* This is a generated file, do not edit.  Generated from signalproxy.h.m4 */

#ifndef _GLIBMM_SIGNALPROXY_H
#define _GLIBMM_SIGNALPROXY_H

extern "C"
{
  typedef void (*GCallback) (void);
  typedef struct _GObject GObject;
}

#include <sigc++/sigc++.h>
#include <glibmm/signalproxy_connectionnode.h>
#include <glibmm/ustring.h>

namespace Glib
{

// Forward declarations
class ObjectBase;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

struct SignalProxyInfo
{
  const char* signal_name;
  GCallback   callback;
  GCallback   notify_callback;
};

#endif //DOXYGEN_SHOULD_SKIP_THIS

// This base class is used by SignalProxyNormal, SignalProxyDetailed and SignalProxyProperty.
class SignalProxyBase
{
public:
  SignalProxyBase(Glib::ObjectBase* obj);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static inline sigc::slot_base* data_to_slot(void* data)
  {
    const auto pConnectionNode = static_cast<SignalProxyConnectionNode*>(data);

    // Return 0 if the connection is blocked.
    return (!pConnectionNode->slot_.blocked()) ? &pConnectionNode->slot_ : 0;
  }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

protected:
  ObjectBase* obj_;

private:
  SignalProxyBase& operator=(const SignalProxyBase&); // not implemented
};


// Shared portion of a Signal without detail
/** The SignalProxy provides an API similar to sigc::signal that can be used to
 * connect sigc::slots to glib signals.
 *
 * This holds the name of the glib signal and the object
 * which might emit it. Actually, proxies are controlled by
 * the template derivatives, which serve as gatekeepers for the
 * types allowed on a particular signal.
 *
 * For signals with a detailed name (signal_name::detail_name) see SignalProxyDetailed.
 */
class SignalProxyNormal : public SignalProxyBase
{
public:
  ~SignalProxyNormal() noexcept;

  /// Stops the current signal emission (not in libsigc++)
  void emission_stop();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // This callback for SignalProxy0<void>
  // is defined here to avoid code duplication.
  static void slot0_void_callback(GObject*, void* data);
#endif

protected:

  /** Creates a proxy for a signal that can be emitted by @a obj.
   * @param obj The object that can emit the signal.
   * @param info Information about the signal, including its name, and the C callbacks that should be called by glib.
   */
  SignalProxyNormal(Glib::ObjectBase* obj, const SignalProxyInfo* info);

  /** Connects a generic signal handler to a signal.
   * This is called by connect() in derived SignalProxy classes.
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::slot_base& connect_(const sigc::slot_base& slot, bool after);

  /** Connects a signal handler without a return value to a signal.
   * This is called by connect_notify() in derived SignalProxy classes.
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::slot_base& connect_notify_(const sigc::slot_base& slot, bool after);

private:
  const SignalProxyInfo* info_;

  //TODO: We could maybe replace both connect_() and connect_notify_() with this in future, because they don't do anything extra.
  /** This is called by connect_() and connect_notify_().
   */
  sigc::slot_base& connect_impl_(GCallback callback, const sigc::slot_base& slot, bool after);

  // no copy assignment
  SignalProxyNormal& operator=(const SignalProxyNormal&);
};

// Shared portion of a Signal with detail
/** The SignalProxy provides an API similar to sigc::signal that can be used to
 * connect sigc::slots to glib signals.
 *
 * This holds the name of the glib signal, including the detail name if any,
 * and the object which might emit it. Actually, proxies are controlled by
 * the template derivatives, which serve as gatekeepers for the
 * types allowed on a particular signal.
 */
class SignalProxyDetailed : public SignalProxyBase
{
public:
  ~SignalProxyDetailed() noexcept;

  /// Stops the current signal emission (not in libsigc++)
  void emission_stop();

protected:

  /** Creates a proxy for a signal that can be emitted by @a obj.
   * @param obj The object that can emit the signal.
   * @param info Information about the signal, including its name
   *             and the C callbacks that should be called by glib.
   * @param detail_name The detail name, if any.
   */
  SignalProxyDetailed(Glib::ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name);

  /** Connects a signal handler to a signal.
   * This is called by connect() and connect_notify() in derived SignalProxy classes.
   *
   * @param notify Whether this method is called by connect_notify() or by connect().
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::slot_base& connect_impl_(bool notify, const sigc::slot_base& slot, bool after);

private:
  const SignalProxyInfo* info_; // Pointer to statically allocated structure.
  const Glib::ustring detailed_name_; // signal_name[::detail_name]

  // no copy assignment
  SignalProxyDetailed& operator=(const SignalProxyDetailed&);
};


/**** Glib::SignalProxy0 ***************************************************/

/** Proxy for signals with 0 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R>
class SignalProxy0 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R>    SlotType;
  typedef sigc::slot<void> VoidSlotType;

  SignalProxy0(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxy1 ***************************************************/

/** Proxy for signals with 1 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1>
class SignalProxy1 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R,P1>    SlotType;
  typedef sigc::slot<void,P1> VoidSlotType;

  SignalProxy1(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxy2 ***************************************************/

/** Proxy for signals with 2 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2>
class SignalProxy2 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R,P1,P2>    SlotType;
  typedef sigc::slot<void,P1,P2> VoidSlotType;

  SignalProxy2(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxy3 ***************************************************/

/** Proxy for signals with 3 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3>
class SignalProxy3 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R,P1,P2,P3>    SlotType;
  typedef sigc::slot<void,P1,P2,P3> VoidSlotType;

  SignalProxy3(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxy4 ***************************************************/

/** Proxy for signals with 4 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3,class P4>
class SignalProxy4 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R,P1,P2,P3,P4>    SlotType;
  typedef sigc::slot<void,P1,P2,P3,P4> VoidSlotType;

  SignalProxy4(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxy5 ***************************************************/

/** Proxy for signals with 5 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3,class P4,class P5>
class SignalProxy5 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R,P1,P2,P3,P4,P5>    SlotType;
  typedef sigc::slot<void,P1,P2,P3,P4,P5> VoidSlotType;

  SignalProxy5(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxy6 ***************************************************/

/** Proxy for signals with 6 arguments.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
class SignalProxy6 : public SignalProxyNormal
{
public:
  typedef sigc::slot<R,P1,P2,P3,P4,P5,P6>    SlotType;
  typedef sigc::slot<void,P1,P2,P3,P4,P5,P6> VoidSlotType;

  SignalProxy6(ObjectBase* obj, const SignalProxyInfo* info)
    : SignalProxyNormal(obj, info) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_(slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_notify_(slot, after)); }
};


/**** Glib::SignalProxyDetailed0 ***************************************************/

/** Proxy for signals with 0 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R>
class SignalProxyDetailed0 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R>    SlotType;
  typedef sigc::slot<void> VoidSlotType;

  SignalProxyDetailed0(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};


/**** Glib::SignalProxyDetailed1 ***************************************************/

/** Proxy for signals with 1 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1>
class SignalProxyDetailed1 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R,P1>    SlotType;
  typedef sigc::slot<void,P1> VoidSlotType;

  SignalProxyDetailed1(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};


/**** Glib::SignalProxyDetailed2 ***************************************************/

/** Proxy for signals with 2 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2>
class SignalProxyDetailed2 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R,P1,P2>    SlotType;
  typedef sigc::slot<void,P1,P2> VoidSlotType;

  SignalProxyDetailed2(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};


/**** Glib::SignalProxyDetailed3 ***************************************************/

/** Proxy for signals with 3 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3>
class SignalProxyDetailed3 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R,P1,P2,P3>    SlotType;
  typedef sigc::slot<void,P1,P2,P3> VoidSlotType;

  SignalProxyDetailed3(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};


/**** Glib::SignalProxyDetailed4 ***************************************************/

/** Proxy for signals with 4 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3,class P4>
class SignalProxyDetailed4 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R,P1,P2,P3,P4>    SlotType;
  typedef sigc::slot<void,P1,P2,P3,P4> VoidSlotType;

  SignalProxyDetailed4(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};


/**** Glib::SignalProxyDetailed5 ***************************************************/

/** Proxy for signals with 5 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3,class P4,class P5>
class SignalProxyDetailed5 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R,P1,P2,P3,P4,P5>    SlotType;
  typedef sigc::slot<void,P1,P2,P3,P4,P5> VoidSlotType;

  SignalProxyDetailed5(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};


/**** Glib::SignalProxyDetailed6 ***************************************************/

/** Proxy for signals with 6 arguments and possibly a detailed name.
 * Use the connect() or connect_notify() method, with sigc::mem_fun() or sigc::ptr_fun()
 * to connect signal handlers to signals.
 */
template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
class SignalProxyDetailed6 : public SignalProxyDetailed
{
public:
  typedef sigc::slot<R,P1,P2,P3,P4,P5,P6>    SlotType;
  typedef sigc::slot<void,P1,P2,P3,P4,P5,P6> VoidSlotType;

  SignalProxyDetailed6(ObjectBase* obj, const SignalProxyInfo* info, const Glib::ustring& detail_name)
    : SignalProxyDetailed(obj, info, detail_name) {}

  /** Connects a signal handler to a signal.
   *
   * For instance, connect( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect(const SlotType& slot, bool after = true)
    { return sigc::connection(connect_impl_(false, slot, after)); }

  /** Connects a signal handler without a return value to a signal.
   * By default, the signal handler will be called before the default signal handler.
   *
   * For instance, connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   *
   * If the signal requires signal handlers with a @c void return type,
   * the only difference between connect() and connect_notify() is the default
   * value of @a after.
   *
   * If the signal requires signal handlers with a return value of type T,
   * connect_notify() binds <tt>return T()</tt> to the connected signal handler.
   * For instance, if the return type is @c bool, the following two calls are equivalent.
   * @code
   * connect_notify( sigc::mem_fun(*this, &TheClass::on_something) );
   * connect( sigc::bind_return<bool>(sigc::mem_fun(*this, &TheClass::on_something), false), false );
   * @endcode
   *
   * @param slot The signal handler, which should have a @c void return type,
   *        usually created with sigc::mem_fun() or sigc::ptr_fun().
   * @param after Whether this signal handler should be called before or after the default signal handler.
   */
  sigc::connection connect_notify(const VoidSlotType& slot, bool after = false)
    { return sigc::connection(connect_impl_(true, slot, after)); }
};

} // namespace Glib

#endif /* _GLIBMM_SIGNALPROXY_H */

