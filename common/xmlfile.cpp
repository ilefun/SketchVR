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
}

XmlGroupInfo::~XmlGroupInfo() {
  delete entities_;
}

const XmlGroupInfo& XmlGroupInfo::operator = (const XmlGroupInfo& info) {
  *entities_ = *info.entities_;
  transform_ = info.transform_;
  return *this;
}

void XmlModelInfo::debug_print()
{
	print_entities(&entities_);
}

void print_entities(XmlEntitiesInfo *entities)
{
    cout<<"{"<<endl;
    std::cout<<"Group Data :"<<endl;
    for (size_t i = 0; i < entities->groups_.size(); ++i)
    {
		cout << endl << "Group transform ---- "<<i<<endl;
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				std::cout << entities->groups_[i].transform_.values[j * 4 + k] << " ";
			}
			std::cout << std::endl;
		}
        print_entities(entities->groups_[i].entities_);
    }

    std::cout <<endl<<"Face num: " << entities->faces_.size() << std::endl;
    for (size_t i = 0; i < entities->faces_.size(); i++)
    {
        if (entities->faces_[i].has_single_loop_)
            std::cout << std::endl << "Face index: " << i << std::endl;
        else
            std::cout << std::endl << "Face index : " << i <<std::endl<< "Contains " << entities->faces_[i].vertices_.size() / 3 << " triangles" << std::endl;
        for (size_t j = 0; j < entities->faces_[i].vertices_.size(); j++)
            std::cout << entities->faces_[i].vertices_[j].vertex_.x() << " " \
            << entities->faces_[i].vertices_[j].vertex_.y() << " " \
            << entities->faces_[i].vertices_[j].vertex_.z() << std::endl;
    }

    cout<<"}"<<endl;
}
