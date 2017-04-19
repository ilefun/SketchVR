#ifndef SKPTOXML_COMMON_STRINGUTILS_H
#define SKPTOXML_COMMON_STRINGUTILS_H

#include <string>

#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>


struct StringConvertUtils
{
  static std::string UTF8_To_string(const std::string & str);
  static std::string string_To_UTF8(const std::string & str);
};

// A simple SUStringRef wrapper class which makes usage simpler from C++.
class CSUString {
 public:
  CSUString() {
    SUSetInvalid(su_str_);
    SUStringCreate(&su_str_);
  }

  ~CSUString() {
    SUStringRelease(&su_str_);
  }

  operator SUStringRef*() {
    return &su_str_;
  }

  std::string utf8();

private:
  // Disallow copying for simplicity
  CSUString(const CSUString& copy);
  CSUString& operator= (const CSUString& copy);

  SUStringRef su_str_;
};
#endif // SKPTOXML_COMMON_STRINGUTILS_H
