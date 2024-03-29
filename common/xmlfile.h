// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLFILE_H
#define SKPTOXML_COMMON_XMLFILE_H

#include <string>
#include <vector>
//#include <map>
#include <unordered_map>

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/component_definition.h>

#include "./xmlgeomutils.h"


// Helper data transfer types storing model information.
struct TextureInfo
{
    TextureInfo()
    : texture_sscale_(0.0), texture_tscale_(0.0),pixel_data_(NULL) {}

  std::string texture_path_;
  double texture_sscale_;
  double texture_tscale_;
  size_t width_;
  size_t height_;

  size_t origin_bits_per_pixel_;
  size_t bits_per_pixel_;
  size_t data_size_;
  SUByte *pixel_data_;  
};

struct SceneInfo
{
  std::string name_;
  SUPoint3D position_;
  SUPoint3D target_;
  SUVector3D up_vector_;
};

struct XmlMaterialInfo {
  XmlMaterialInfo()
    : has_color_(false), has_alpha_(false), alpha_(0.0),
      has_texture_(false){}
  
  ~XmlMaterialInfo(){}
  SUMaterialRef origin_ref_;
  std::string name_;
  bool has_color_;
  SUColor color_;
  bool has_alpha_;
  double alpha_;

  bool has_texture_;
  // TextureInfo tex_info_;
  std::string texture_key_;
};

struct XmlLayerInfo {
  XmlLayerInfo() : has_material_info_(false), is_visible_(false) {}

  std::string name_;
  bool has_material_info_;
  XmlMaterialInfo material_info_;
  bool is_visible_;
};

struct XmlEdgeInfo {
  XmlEdgeInfo() : has_layer_(false), has_color_(false) {}

  bool has_layer_;
  std::string layer_name_;
  bool has_color_;
  SUColor color_;
  XmlGeomUtils::CPoint3d start_;
  XmlGeomUtils::CPoint3d end_;
};

struct XmlCurveInfo {
  std::vector<XmlEdgeInfo> edges_;
};

struct XmlFaceVertex {
  XmlGeomUtils::CPoint3d vertex_;
  XmlGeomUtils::CPoint3d front_texture_coord_;
  XmlGeomUtils::CPoint3d back_texture_coord_;
};

struct XmlFaceInfo {
  XmlFaceInfo()
    : has_front_texture_(false),
      has_back_texture_(false),
      has_single_loop_(false),
	  face_num_(0){}

  std::string layer_name_;
  std::string front_mat_name_;
  std::string back_mat_name_;
  bool has_front_texture_;
  bool has_back_texture_;
  bool has_single_loop_;
  int face_num_;
  XmlGeomUtils::CVector3d face_normal_;
  // if single loop, vertices_ are the points in the loop
  // if triangles, vertices_ are 3 per triangle
  std::vector<XmlFaceVertex> vertices_;
};

struct XmlEntitiesInfo;
struct XmlComponentDefinitionInfo;

struct DefinitionInfo
{
	SUComponentBehavior behavior_;
	SUPoint3D insert_point_;
};

struct XmlGroupInfo {
  XmlGroupInfo();
  XmlGroupInfo(const XmlGroupInfo&);
  ~XmlGroupInfo();
  const XmlGroupInfo& operator = (const XmlGroupInfo&);
  
  XmlEntitiesInfo* entities_;
  SUTransformation transform_;
  std::string component_name_;
};

struct XmlComponentInstanceInfo {
  std::string definition_name_;
  std::string layer_name_;
  std::string material_name_;
  SUTransformation transform_;
};

struct XmlEntitiesInfo {
  XmlEntitiesInfo(){vertex_num_=0;face_num_=0;}
  ~XmlEntitiesInfo(){};

  std::vector<XmlComponentInstanceInfo> component_instances_;
  std::vector<XmlGroupInfo> groups_;
  std::vector<XmlFaceInfo>  faces_;
  int vertex_num_;
  int face_num_;

  std::vector<XmlEdgeInfo>  edges_;
  std::vector<XmlCurveInfo> curves_;
};

struct XmlComponentDefinitionInfo {
  std::string name_;
  XmlEntitiesInfo entities_;
};

struct XmlModelInfo {
  std::vector<XmlLayerInfo> layers_;
  
  std::vector<XmlMaterialInfo> materials_;
  std::unordered_map<std::string, int> matname_id_map_;

  std::unordered_map<std::string, TextureInfo> texture_map_;

  std::vector<SceneInfo> scenes_;
  std::unordered_map<std::string, DefinitionInfo> defitions_;
  
  XmlEntitiesInfo entities_;
  
};

#endif // SKPTOXML_COMMON_XMLFILE_H
