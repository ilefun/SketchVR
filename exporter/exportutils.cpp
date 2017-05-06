#include "./exportutils.h"
#include "../common/utils.h"

#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/image_rep.h>
#include <SketchUpAPI/model/face.h>

#include <cassert>
#include <vector>
#include <iostream>

inline float clamp(float n, float lower, float upper) {
  return max(lower, min(n, upper));
}

std::string ExportUtils::GetMaterialName(SUMaterialRef material) {
  CSUString name;
  SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
  return name.utf8();
}

std::string ExportUtils::GetLayerName(SULayerRef layer) {
  CSUString name;
  SU_CALL(SULayerGetName(layer, name));
  return name.utf8();
}

std::string ExportUtils::GetComponentDefinitionName(
    SUComponentDefinitionRef comp_def) {
  CSUString name;
  SU_CALL(SUComponentDefinitionGetName(comp_def, name));
  return name.utf8();
}

void ExportUtils::GetTexturePixel(const XmlMaterialInfo &current_mat, float pixel_data[])
{
  if (current_mat.has_color_) {
    float avg_color[3] = { 0,0,0 };
    int size_per_pixel = 3;
    if (current_mat.tex_info_.bits_per_pixel_ == 32)
      size_per_pixel = 4;

    int pixel_num = current_mat.tex_info_.data_size_ / size_per_pixel;
    for (int j = 0; j < pixel_num; j++)
    {
      avg_color[0] += current_mat.tex_info_.pixel_data_[j * size_per_pixel + 2];
      avg_color[1] += current_mat.tex_info_.pixel_data_[j * size_per_pixel + 1];
      avg_color[2] += current_mat.tex_info_.pixel_data_[j * size_per_pixel];
    }
    avg_color[0] /= float(pixel_num);
    avg_color[1] /= float(pixel_num);
    avg_color[2] /= float(pixel_num);

    for (size_t i = 0; i < pixel_num; ++i) {
      pixel_data[i * size_per_pixel + 2] = clamp((current_mat.tex_info_.pixel_data_[i * size_per_pixel + 2] + current_mat.color_.red - avg_color[0]) / 255.0f, 0, 1);
      pixel_data[i * size_per_pixel + 1] = clamp((current_mat.tex_info_.pixel_data_[i * size_per_pixel + 1] + current_mat.color_.green - avg_color[1]) / 255.0f, 0, 1);
      pixel_data[i * size_per_pixel] = clamp((current_mat.tex_info_.pixel_data_[i * size_per_pixel] + current_mat.color_.blue - avg_color[2]) / 255.0f, 0, 1);
    }

    if(size_per_pixel==4)
      for (size_t i = 0; i < pixel_num; ++i) {
        pixel_data[i * size_per_pixel + 3] = float(current_mat.tex_info_.pixel_data_[i * size_per_pixel + 3]) / 255.f;
      }

  }
  else {
    for (size_t i = 0; i < current_mat.tex_info_.data_size_; ++i) {
      pixel_data[i] = float(current_mat.tex_info_.pixel_data_[i]) / 255.0f;
    }
  }
}

XmlMaterialInfo ExportUtils::GetMaterialInfo(SUMaterialRef material,SUImageRepRef image_rep) {
  assert(!SUIsInvalid(material));

  XmlMaterialInfo info;
  info.origin_ref_ = material;

  // Name
  info.name_ = ExportUtils::GetMaterialName(material);

  // Color
  info.has_color_ = false;
  info.has_alpha_ = false;
  SUMaterialType type;
  SU_CALL(SUMaterialGetType(material, &type));
  // Color
  if ((type == SUMaterialType_Colored) ||
      (type == SUMaterialType_ColorizedTexture)) {
    SUColor color;
    if (SUMaterialGetColor(material, &color) == SU_ERROR_NONE) {
      info.has_color_ = true;
      info.color_ = color;
    }
  }

  // Alpha
  bool has_alpha = false;
  SU_CALL(SUMaterialGetUseOpacity(material, &has_alpha));
  if (has_alpha) {
    double alpha = 0;
    SU_CALL(SUMaterialGetOpacity(material, &alpha));
    info.has_alpha_ = true;
    info.alpha_ = alpha;
  }

  // Texture
  info.has_texture_ = false;
  if ((type == SUMaterialType_Textured) ||
      (type == SUMaterialType_ColorizedTexture)) {
    SUTextureRef texture = SU_INVALID;
    if (SUMaterialGetTexture(material, &texture) == SU_ERROR_NONE) {
      info.has_texture_ = true;
      // Texture path
      CSUString texture_path;
      SU_CALL(SUTextureGetFileName(texture, texture_path));
      std::string tex_path = texture_path.utf8();
	  //cout <<endl<<"Tex path : " <<tex_path << endl;
      // Texture scale
      size_t width = 0;
      size_t height = 0;
      double s_scale = 0.0;
      double t_scale = 0.0;
      SU_CALL(SUTextureGetDimensions(texture, &width, &height,
                                     &s_scale, &t_scale));
      info.tex_info_.texture_sscale_ = s_scale;
      info.tex_info_.texture_tscale_ = t_scale;
      info.tex_info_.width_ = width;
      info.tex_info_.height_ = height;
      info.tex_info_.texture_path_=tex_path;

      //Texture data
      SU_CALL(SUTextureGetImageRep(texture,&image_rep));

      size_t data_size=0, bits_per_pixel=0;
      SU_CALL(SUImageRepGetDataSize(image_rep,&data_size,&bits_per_pixel));
	    info.tex_info_.origin_bits_per_pixel_ = bits_per_pixel;

	  if (bits_per_pixel == 24)
	  {
		  //to fix jpg bug,we convert all images to 32 bit
		  SUImageRepConvertTo32BitsPerPixel(image_rep);
		  SU_CALL(SUImageRepGetDataSize(image_rep, &data_size, &bits_per_pixel));
	  }

      info.tex_info_.data_size_=data_size;
      info.tex_info_.bits_per_pixel_=bits_per_pixel;
      int image_size = width*height*bits_per_pixel / 8;
      info.tex_info_.pixel_data_=new SUByte[data_size];
      SU_CALL(SUImageRepGetData(image_rep, data_size,info.tex_info_.pixel_data_));
    }
  }
  return info;
}

void ExportUtils::GetTransformedFace(XmlEntitiesInfo *to_entities,
                                      XmlEntitiesInfo *from_entities,
                                      std::vector<SUTransformation> &transforms)
{
  for (int i = 0; i < from_entities->faces_.size(); ++i)
  {
    auto single_face=from_entities->faces_[i];

    for(int j=transforms.size()-1; j>=0; j--){
      single_face.face_normal_.Transform(transforms[j].values);
    
      for(int k=0;k<single_face.vertices_.size();k++)
        single_face.vertices_[k].vertex_.Transform(transforms[j].values);
    }

    to_entities->vertex_num_ += single_face.vertices_.size();
    to_entities->face_num_ += single_face.face_num_;
    to_entities->faces_.push_back(single_face);
  }
}

void ExportUtils::FixNormal(XmlEntitiesInfo &entity_info){
    //we only check and fix the normal of first group,faces in second group are always facing camera
    for (int j = 0; j < entity_info.faces_.size(); ++j)
    {
      for (int k = 0; k < entity_info.faces_[j].face_num_; ++k)
      {
          if (!XmlGeomUtils::NormalEqual(entity_info.faces_[j].vertices_[k * 3].vertex_,
              entity_info.faces_[j].vertices_[k * 3 + 1].vertex_,
              entity_info.faces_[j].vertices_[k * 3 + 2].vertex_,
              entity_info.faces_[j].face_normal_))
              swap(entity_info.faces_[j].vertices_[k * 3], entity_info.faces_[j].vertices_[k * 3 + 2]);
      }
    }
}

void ExportUtils::CheckFaceMaterial(std::vector<SUFaceRef> &faces, 
									SUMaterialRef mat_ref,
									std::vector<std::pair<size_t, bool>> &face_no_material)
{
	for (size_t i = 0; i < faces.size(); i++) {

		SUMaterialRef front_material = SU_INVALID;
		SUFaceGetFrontMaterial(faces[i], &front_material);
		if (SUIsInvalid(front_material))
		{
			SUFaceSetFrontMaterial(faces[i], mat_ref);
			face_no_material.push_back(std::make_pair(i, true));
		}

		SUMaterialRef back_material = SU_INVALID;
		SUFaceGetBackMaterial(faces[i], &back_material);
		if (SUIsInvalid(back_material))
		{
			SUFaceSetBackMaterial(faces[i], mat_ref);
			face_no_material.push_back(std::make_pair(i, false));
		}
	}
}

void ExportUtils::ClearFaceMaterial(std::vector<SUFaceRef> &faces, const std::vector<std::pair<size_t, bool>> &face_no_material) 
{
	SUMaterialRef no_material = SU_INVALID;

	for (size_t i = 0; i < face_no_material.size(); i++) {
		auto face = faces[face_no_material[i].first];
		if(face_no_material[i].second)
			SUFaceSetFrontMaterial(face, no_material);
		else
			SUFaceSetBackMaterial(face, no_material);

	}
}