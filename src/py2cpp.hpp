#ifndef __PY2CPP_HPP__
#define __PY2CPP_HPP__

#include <Python.h>

#include <cassert>
#include <climits>
#include <functional>
#include <memory>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace dubzzz {
namespace Py2Cpp {

/**
 * Public structures
 */

// Struct automatically decreasing PyObject counter when used with unique_ptr
//
// Syntax:
//    std::unique_ptr<PyObject, decref> python_object
struct decref
{
  void operator() (PyObject* pyo)
  {
    Py_DECREF(pyo);
  }
};

// FromTuple and FromDict are used to build cutsom and complex objects
// based on dicts or classes
// Syntax:
//    CppBuilder<FromTuple<MyClass, accessors' types...>>
//    CppBuilder<FromDict<MyClass, accessors' types...>>
template <class OBJ, class... Args> struct FromTuple {};
template <class OBJ, class... Args> struct FromDict {};

template <class T> struct CppBuilder;

/**
 * Internal structures
 */

template <class T> struct ToBuildable : T {};

/**
 * Identity builder
 */

template <>
struct CppBuilder<PyObject*>
{
  typedef PyObject* value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    return pyo;
  }
};
template <> struct ToBuildable<PyObject*> : CppBuilder<PyObject*> {};

/**
 * Primitives builders
 */

template <>
struct CppBuilder<bool>
{
  typedef bool value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyBool_Check(pyo))
    {
      return pyo == Py_True;
    }
    throw std::invalid_argument("Not a PyBool instance");
  }
};
template <> struct ToBuildable<bool> : CppBuilder<bool> {};

template <>
struct CppBuilder<int>
{
  typedef int value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyLong_Check(pyo))
    {
      long v { PyLong_AsLong(pyo) };
      if (v < INT_MIN || v > INT_MAX)
      {
        throw std::overflow_error("Out of <int> boundaries");
      }
      return static_cast<int>(v);
    }
    else if (PyInt_Check(pyo))
    {
      long v { PyInt_AS_LONG(pyo) };
      if (v < INT_MIN || v > INT_MAX)
      {
        throw std::overflow_error("Out of <int> boundaries");
      }
      return static_cast<int>(v);
    }
    throw std::invalid_argument("Not a PyLong instance");
  }
};
template <> struct ToBuildable<int> : CppBuilder<int> {};

template <>
struct CppBuilder<unsigned int>
{
  typedef unsigned int value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyLong_Check(pyo))
    {
      unsigned long v { PyLong_AsUnsignedLong(pyo) };
      if (!! PyErr_Occurred() && !! PyErr_ExceptionMatches(PyExc_OverflowError))
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <unsigned int> boundaries");
      }
      if (v > UINT_MAX)
      {
        throw std::overflow_error("Out of <unsigned int> boundaries");
      }
      return static_cast<unsigned int>(v);
    }
    else if (PyInt_Check(pyo))
    {
      long v { PyInt_AsLong(pyo) }; // PyInt is a long
      if (v < 0 || v > UINT_MAX)
      {
        throw std::overflow_error("Out of <unsigned int> boundaries");
      }
      return static_cast<unsigned int>(v);
    }
    throw std::invalid_argument("Not a PyLong instance");
  }
};
template <> struct ToBuildable<unsigned int> : CppBuilder<unsigned int> {};

template <>
struct CppBuilder<long>
{
  typedef long value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyLong_Check(pyo))
    {
      long value { PyLong_AsLong(pyo) };
      if (!! PyErr_Occurred() && !! PyErr_ExceptionMatches(PyExc_OverflowError))
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <long> boundaries");
      }
      return value;
    }
    else if (PyInt_Check(pyo))
    {
      return PyInt_AS_LONG(pyo);
    }
    throw std::invalid_argument("Not a PyLong instance");
  }
};
template <> struct ToBuildable<long> : CppBuilder<long> {};

template <>
struct CppBuilder<unsigned long>
{
  typedef unsigned long value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyLong_Check(pyo))
    {
      unsigned long value { PyLong_AsUnsignedLong(pyo) };
      if (!! PyErr_Occurred() && !! PyErr_ExceptionMatches(PyExc_OverflowError))
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <unsigned long> boundaries");
      }
      return value;
    }
    else if (PyInt_Check(pyo))
    {
      long value { PyInt_AsLong(pyo) }; // PyInt is a long, LONG_MAX < ULONG_MAX
      if (value < 0)
      {
        throw std::overflow_error("Out of <unsigned long> boundaries");
      }
      return static_cast<unsigned long>(value);
    }
    throw std::invalid_argument("Not a PyLong instance");
  }
};
template <> struct ToBuildable<unsigned long> : CppBuilder<unsigned long> {};

template <>
struct CppBuilder<long long>
{
  typedef long long value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyLong_Check(pyo))
    {
      long long value { PyLong_AsLongLong(pyo) };
      if (!! PyErr_Occurred() && !! PyErr_ExceptionMatches(PyExc_OverflowError))
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <long long> boundaries");
      }
      return value;
    }
    else if (PyInt_Check(pyo))
    {
      return static_cast<long long>(PyInt_AsLong(pyo));
    }
    throw std::invalid_argument("Not a PyLong instance");
  }
};
template <> struct ToBuildable<long long> : CppBuilder<long long> {};

template <>
struct CppBuilder<unsigned long long>
{
  typedef unsigned long long value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyLong_Check(pyo))
    {
      unsigned long long value { PyLong_AsUnsignedLongLong(pyo) };
      if (!! PyErr_Occurred()
          && (!! PyErr_ExceptionMatches(PyExc_OverflowError) || !! PyErr_ExceptionMatches(PyExc_TypeError)))
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <unsigned long long> boundaries");
      }
      return value;
    }
    else if (PyInt_Check(pyo))
    {
      long value { PyInt_AsLong(pyo) }; // PyInt is a long, LONG_MAX < ULLONG_MAX
      if (value < 0)
      {
        throw std::overflow_error("Out of <unsigned long long> boundaries");
      }
      return static_cast<unsigned long long>(value);
    }
    throw std::invalid_argument("Not a PyLong instance");
  }
};
template <> struct ToBuildable<unsigned long long> : CppBuilder<unsigned long long> {};

template <>
struct CppBuilder<double>
{
  typedef double value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyFloat_Check(pyo))
    {
      double value { PyFloat_AsDouble(pyo) }; // PyFloat is a double
      if (value ==  INFINITY || value == -INFINITY)
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <double> boundaries");
      }
      return value;
    }
    else if (PyLong_Check(pyo))
    {
      double value { PyLong_AsDouble(pyo) };
      if (!! PyErr_Occurred() && !! PyErr_ExceptionMatches(PyExc_OverflowError))
      {
        PyErr_Clear();
        throw std::overflow_error("Out of <double> boundaries");
      }
      return value;
    }
    else if (PyInt_Check(pyo))
    {
      return PyInt_AS_LONG(pyo);
    }
    throw std::invalid_argument("Neither a PyDouble nor a PyLong instance");
  }
};
template <> struct ToBuildable<double> : CppBuilder<double> {};

/**
 * Strings builders
 */

template <>
struct CppBuilder<std::string>
{
  typedef std::string value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyString_Check(pyo))
    {
      long int size { PyString_Size(pyo) };
      if (size < 0)
      {
        throw std::runtime_error("Unable to retrieve C/C++ string from PyString");
      }
      return std::string(PyString_AsString(pyo), size);
    }
    else if (PyUnicode_Check(pyo))
    {
      long int size { PyUnicode_GetSize(pyo) };
      if (size < 0)
      {
        throw std::runtime_error("Unable to retrieve C/C++ string from PyUnicode");
      }
      wchar_t wstr[size];
      PyUnicode_AsWideChar(reinterpret_cast<PyUnicodeObject*>(pyo), wstr, size);
      std::wstring wide(wstr, size);
      return std::string(wide.begin(), wide.end());
    }
    throw std::invalid_argument("Neither a PyString nor a PyUnicode instance");
  }
};
template <> struct ToBuildable<std::string> : CppBuilder<std::string> {};

template <>
struct CppBuilder<std::wstring>
{
  typedef std::wstring value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyString_Check(pyo))
    {
      const char* str { PyString_AsString(pyo) };
      long int size { PyString_Size(pyo) };
      if (size < 0)
      {
        throw std::runtime_error("Unable to retrieve C/C++ string from PyString");
      }
      wchar_t wstr[size];
      mbstowcs(wstr, str, size);
      return std::wstring(wstr, size);
    }
    else if (PyUnicode_Check(pyo))
    {
      long int size { PyUnicode_GetSize(pyo) };
      if (size < 0)
      {
        throw std::runtime_error("Unable to retrieve C/C++ string from PyUnicode");
      }
      wchar_t wstr[size];
      PyUnicode_AsWideChar(reinterpret_cast<PyUnicodeObject*>(pyo), wstr, size);
      return std::wstring(wstr, size);
    }
    throw std::invalid_argument("Neither a PyString nor a PyUnicode instance");
  }
};
template <> struct ToBuildable<std::wstring> : CppBuilder<std::wstring> {};

/**
 * Tuple builder
 */

template <class TUPLE, std::size_t pos>
void _feedCppTuple(TUPLE& tuple, PyObject* root)
{}

template <class TUPLE, std::size_t pos, class T, class... Args>
void _feedCppTuple(TUPLE& tuple, PyObject* root)
{
  std::get<pos>(tuple) = CppBuilder<T>()(PyTuple_GetItem(root, pos));
  _feedCppTuple<TUPLE, pos +1, Args...>(tuple, root);
}

template <class... Args>
struct CppBuilder<std::tuple<Args...>>
{
  typedef std::tuple<Args...> value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyTuple_Check(pyo))
    {
      if (PyTuple_Size(pyo) == sizeof...(Args))
      {
        std::tuple<Args...> tuple { std::make_tuple(Args()...) };
        _feedCppTuple<std::tuple<Args...>, 0, Args...>(tuple, pyo);
        return tuple;
      }
      else
      {
        std::ostringstream oss;
        oss << "PyTuple length differs from asked one: "
            << "PyTuple(" << PyTuple_Size(pyo) << ") "
            << "and std::tuple<...>(" << sizeof...(Args) << ")";
        throw std::length_error(oss.str());
      }
    }
    throw std::invalid_argument("Not a PyTuple instance");
  }
};
template <class... Args> struct ToBuildable<std::tuple<Args...>> : CppBuilder<std::tuple<Args...>> {};

/**
 * Vector builder
 */

template <class T>
struct CppBuilder<std::vector<T>>
{
  typedef std::vector<typename ToBuildable<T>::value_type> value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyList_Check(pyo))
    {
      unsigned int i { 0 };
      value_type v(PyList_Size(pyo));
      for (typename value_type::iterator it { v.begin() } ; it != v.end() ; ++it, ++i)
      {
        *it = ToBuildable<T>()(PyList_GetItem(pyo, i));
      }
      return v;
    }
    throw std::invalid_argument("Not a PyList instance");
  }
};
template <class T> struct ToBuildable<std::vector<T>> : CppBuilder<std::vector<T>> {};

/**
 * Set builder
 */

template <class T>
struct CppBuilder<std::set<T>>
{
  typedef std::set<typename ToBuildable<T>::value_type> value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PySet_Check(pyo))
    {
      long size { PySet_Size(pyo) };
      std::vector<PyObject*> backup(size);
      value_type s;
      for (long i { 0 } ; i != size ; ++i)
      {
        PyObject* popped { PySet_Pop(pyo) };
        backup[i] = popped;
        s.insert(ToBuildable<T>()(popped));
      }
      for (PyObject* popped : backup)
      {
        PySet_Add(pyo, popped);
        Py_DECREF(popped);
      }
      return s;
    }
    throw std::invalid_argument("Not a PySet instance");
  }
};
template <class T> struct ToBuildable<std::set<T>> : CppBuilder<std::set<T>> {};

/**
 * Map builder
 */

template <class K, class T>
struct CppBuilder<std::map<K,T>>
{
  typedef std::map<typename ToBuildable<K>::value_type, typename ToBuildable<T>::value_type> value_type;
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    if (PyDict_Check(pyo))
    {
      value_type dict;
      PyObject *key, *value;
      Py_ssize_t pos = 0;
      while (PyDict_Next(pyo, &pos, &key, &value))
      {
        dict[ToBuildable<K>()(key)] = ToBuildable<T>()(value);
      }
      return dict;
    }
    throw std::invalid_argument("Not a PyDict instance");
  }
};
template <class K, class T> struct ToBuildable<std::map<K,T>> : CppBuilder<std::map<K,T>> {};

/**
 * Objects builders
 */

template <class OBJ, class T>
inline std::pair<std::string, std::function<void(OBJ&,T)>> make_mapping(const std::string &key, std::function<void(OBJ&,T)> fun)
{
  return std::make_pair(key, fun);
}

template <class OBJ, class T>
inline std::pair<std::string, std::function<void(OBJ&,T)>> make_mapping(const std::string &key, void (OBJ::*fun)(T))
{
  return std::make_pair(key, fun);
}

template <class OBJ, class T>
inline std::pair<std::string, std::function<void(OBJ&,T)>> make_mapping(const std::string &key, T OBJ::*member)
{
  return std::make_pair(key, [member](OBJ& obj, const T &value){ obj.*member = value; });
}

template <class OBJ, std::size_t pos, class... Args>
struct CppBuilderHelper;

template <class OBJ, std::size_t pos>
struct CppBuilderHelper<OBJ, pos>
{
  CppBuilderHelper() {}
  inline void fromDict(OBJ& obj, PyObject* pyo) const {}
  inline void fromObject(OBJ& obj, PyObject* pyo) const {}
  inline void fromTuple(OBJ& obj, PyObject* pyo) const {}
};

template <class OBJ, std::size_t pos, class FUNCTOR, class... Args>
struct CppBuilderHelper<OBJ,pos,FUNCTOR,Args...>
{
  const std::pair<std::string, std::function<void(OBJ&, typename FUNCTOR::value_type)>> callback;
  const CppBuilderHelper<OBJ, pos +1, Args...> subBuilder;
  
  // tuple's constructors
  CppBuilderHelper(std::function<void(OBJ&, typename FUNCTOR::value_type)> fun, std::function<void(OBJ&, typename Args::value_type)>... args)
      : callback(make_mapping("", fun)), subBuilder(args...)
  {}
  CppBuilderHelper(typename FUNCTOR::value_type OBJ::*member, typename Args::value_type OBJ::*... args)
      : callback(make_mapping("", member)), subBuilder(args...)
  {}
  
  // full constructors  
  CppBuilderHelper(
        std::pair<std::string, std::function<void(OBJ&, typename FUNCTOR::value_type)>> callback
        , std::pair<std::string, std::function<void(OBJ&, typename Args::value_type)>>... args)
      : callback(callback), subBuilder(args...)
  {}
  
  inline void fromDict(OBJ& obj, PyObject* pyo) const
  {
    PyObject *pyo_item { PyDict_GetItemString(pyo, callback.first.c_str()) };
    if (pyo_item)
    {
      typename FUNCTOR::value_type value { FUNCTOR()(pyo_item) };
      callback.second(obj, value);
    }
    subBuilder.fromDict(obj, pyo);
  }
  
  inline void fromObject(OBJ& obj, PyObject* pyo) const
  {
    if (PyObject_HasAttrString(pyo, callback.first.c_str()))
    {
      std::unique_ptr<PyObject, decref> pyo_item { PyObject_GetAttrString(pyo, callback.first.c_str()) };
      typename FUNCTOR::value_type value { FUNCTOR()(pyo_item.get()) };
      callback.second(obj, value);
    }
    subBuilder.fromObject(obj, pyo);
  }
  
  inline void fromTuple(OBJ& obj, PyObject* pyo) const
  {
    typename FUNCTOR::value_type value { FUNCTOR()(PyTuple_GetItem(pyo, pos)) };
    callback.second(obj, value);
    subBuilder.fromTuple(obj, pyo);
  }
};

template <class OBJ, class... Args>
struct CppBuilder<FromTuple<OBJ, Args...>>
{
  typedef OBJ value_type;
  const CppBuilderHelper<OBJ, 0, ToBuildable<Args>...> subBuilder;
  
  CppBuilder(std::function<void(OBJ&, typename ToBuildable<Args>::value_type)>... args)
      : subBuilder(args...)
  {}
  
  CppBuilder(typename ToBuildable<Args>::value_type OBJ::*... args)
      : subBuilder(args...)
  {}
  
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    
    if (PyTuple_Check(pyo))
    {
      if (PyTuple_Size(pyo) == sizeof...(Args))
      {
        OBJ obj;
        subBuilder.fromTuple(obj, pyo);
        return obj;
      }
      else
      {
        std::ostringstream oss;
        oss << "PyTuple length differs from asked one: "
            << "PyTuple(" << PyTuple_Size(pyo) << ") "
            << "and FromTuple<...>(" << sizeof...(Args) << ")";
        throw std::length_error(oss.str());
      }
    }
    throw std::invalid_argument("Not a PyTuple instance");
  }
};

template <class OBJ, class... Args>
struct CppBuilder<FromDict<OBJ, Args...>>
{
  typedef OBJ value_type;
  CppBuilderHelper<OBJ, 0, ToBuildable<Args>...> subBuilder;
  
  CppBuilder(std::pair<std::string, std::function<void(OBJ&, typename ToBuildable<Args>::value_type)>>... args)
      : subBuilder(args...)
  {}
  
  value_type operator() (PyObject* pyo)
  {
    assert(pyo);
    
    OBJ obj;
    if (PyDict_Check(pyo))
    {
      subBuilder.fromDict(obj, pyo);
    }
    else
    {
      subBuilder.fromObject(obj, pyo);
    }
    return obj;
  }
};

}
}

#endif

