// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <vector>
#include <sstream>
#include <iostream>

#include "./xmlfile.h"

using namespace XmlGeomUtils;
using namespace std;

//------------------------------------------------------------------------------

XmlGroupInfo::XmlGroupInfo() {
  entities_ = new XmlEntitiesInfo;
  definition_info_=NULL;
}

XmlGroupInfo::XmlGroupInfo(const XmlGroupInfo& info) {
  entities_ = new XmlEntitiesInfo(*info.entities_);
  transform_ = info.transform_;
}

XmlGroupInfo::~XmlGroupInfo() {
  delete entities_;
}

const XmlGroupInfo& XmlGroupInfo::operator = (const XmlGroupInfo& info) {
  *entities_ = *info.entities_;
  transform_ = info.transform_;
  return *this;
}