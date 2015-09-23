#include <Python.h>
#include "gtest/gtest.h"

#include <climits>
#include <memory>
#include "py2cpp.hpp"

using namespace dubzzz::Py2Cpp;
PyObject *py_main;
PyObject *py_dict;

struct decref
{
void operator() (PyObject* pyo)
{
  Py_DECREF(pyo);
}
};

TEST(CppBuilder_bool, True)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("True", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ(true, CppBuilder<bool>()(pyo.get()));
}
TEST(CppBuilder_bool, False)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("False", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ(true, !CppBuilder<bool>()(pyo.get()));
}

TEST(CppBuilder_int, AnyValue)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("5", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ(5, CppBuilder<int>()(pyo.get()));
}

TEST(CppBuilder_string, String)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("'hello'", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ("hello", CppBuilder<std::string>()(pyo.get()));
}

TEST(CppBuilder_string, Unicode)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("u'hello'", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ("hello", CppBuilder<std::string>()(pyo.get()));
}

TEST(CppBuilder_string, UnicodeExotic)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String(
      "u'\u15c7\u25d8\u0034\u2b15'", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ("\xe1\x97\x87\xe2\x97\x98\x34\xe2\xac\x95", CppBuilder<std::string>()(pyo.get()));
}

TEST(CppBuilder_wstring, String)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("'hello'", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ(L"hello", CppBuilder<std::wstring>()(pyo.get()));
}

TEST(CppBuilder_wstring, Unicode)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("u'hello'", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ(L"hello", CppBuilder<std::wstring>()(pyo.get()));
}

TEST(CppBuilder_wstring, UnicodeExotic)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String(
      "u'\u15c7\u25d8\u0034\u2b15'", Py_eval_input, py_dict, NULL) }; //utf-16
  ASSERT_NE(nullptr, pyo.get());
  EXPECT_EQ(L"\xe1\x97\x87\xe2\x97\x98\x34\xe2\xac\x95", CppBuilder<std::wstring>()(pyo.get()));
}

TEST(CppBuilder_mix, AnyValue)
{
  std::unique_ptr<PyObject, decref> pyo { PyRun_String("{ \
        'positions':[ \
        { \
          'x': 5, \
          'y': 10, \
        }, { \
          'x': -1, \
          'y': 2, \
        }], \
      }", Py_eval_input, py_dict, NULL) };
  ASSERT_NE(nullptr, pyo.get());
  
  std::map<std::string, std::vector<std::map<std::string, int>>> elt;
  std::vector<std::map<std::string, int>> positions;
  positions.push_back({ {"x", 5}, {"y", 10} });
  positions.push_back({ {"x", -1}, {"y", 2} });
  elt["positions"] = positions;
  
  auto Functor = CppBuilder<std::map<std::string, std::vector<std::map<std::string, int>>>>();
  EXPECT_EQ(elt, Functor(pyo.get()));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  Py_Initialize();
  py_main = PyImport_AddModule("__main__");
  py_dict = PyModule_GetDict(py_main);
  int ret { RUN_ALL_TESTS() };
  Py_Finalize();
  return ret;
}
