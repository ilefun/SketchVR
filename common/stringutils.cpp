#include "stringutils.h"

std::string StringConvertUtils::UTF8_To_string(const std::string & str)
{
  int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

  wchar_t * pwBuf = new wchar_t[nwLen + 1];//Ò»¶¨Òª¼Ó1£¬²»È»»á³öÏÖÎ²°Í 
  memset(pwBuf, 0, nwLen * 2 + 2);

  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

  int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

  char * pBuf = new char[nLen + 1];
  memset(pBuf, 0, nLen + 1);

  WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

  std::string retStr = pBuf;

  delete[]pBuf;
  delete[]pwBuf;

  pBuf = NULL;
  pwBuf = NULL;

  return retStr;
}

std::string StringConvertUtils::string_To_UTF8(const std::string & str)
{
  int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

  wchar_t * pwBuf = new wchar_t[nwLen + 1];//Ò»¶¨Òª¼Ó1£¬²»È»»á³öÏÖÎ²°Í 
  ZeroMemory(pwBuf, nwLen * 2 + 2);

  ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

  int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

  char * pBuf = new char[nLen + 1];
  ZeroMemory(pBuf, nLen + 1);

  ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

  std::string retStr(pBuf);

  delete[]pwBuf;
  delete[]pBuf;

  pwBuf = NULL;
  pBuf = NULL;

  return retStr;
}
 
std::string CSUString::utf8() {
  size_t length;
  SUStringGetUTF8Length(su_str_, &length);
  std::string string;
  string.resize(length+1);
  size_t returned_length;
  SUStringGetUTF8(su_str_, length, &string[0], &returned_length);
  return string;
}
