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
}

XmlGroupInfo::XmlGroupInfo(const XmlGroupInfo& info) {
  entities_ = new XmlEntitiesInfo(*info.entities_);
  transform_ = info.transform_;
  component_name_ = info.component_name_;
}

XmlGroupInfo::~XmlGroupInfo() {
  delete entities_;
}

const XmlGroupInfo& XmlGroupInfo::operator = (const XmlGroupInfo& info) {
  *entities_ = *info.entities_;
  transform_ = info.transform_;
  component_name_ = info.component_name_;
  return *this;
}


//------------------------------------------------------------------------------
DefinitionInfo::DefinitionInfo(bool GetEntities) {
  if(GetEntities)
  	entities_ = new XmlEntitiesInfo;
  else
  	entities_=NULL;
}

DefinitionInfo::DefinitionInfo(const DefinitionInfo& info) {
  if(info.entities_)
  	entities_ = new XmlEntitiesInfo(*info.entities_);
  behavior_ = info.behavior_;
  insert_point_ = info.insert_point_;
}

DefinitionInfo::~DefinitionInfo() {
  if(entities_)
  	delete entities_;
}

const DefinitionInfo& DefinitionInfo::operator = (const DefinitionInfo& info) {
  if(info.entities_)
  	*entities_ = *info.entities_;
  behavior_ = info.behavior_;
  insert_point_ = info.insert_point_;
  return *this;
}