#ifndef SKPTOXML_COMMON_STRINGUTILS_H
#define SKPTOXML_COMMON_STRINGUTILS_H

#include <string>
#include <SketchUpAPI/unicodestring.h>

using namespace std;
struct StringConvertUtils
{
  static std::string UTF8_To_string(const std::string & str);
  static std::string string_To_UTF8(const std::string & str);
  static string&   StringConvertUtils::replace_all(string&   str,const   string&   old_value,const   string&   new_value)  ;
  static string&   StringConvertUtils::replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value);

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
