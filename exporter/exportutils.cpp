#include "./exportutils.h"
#include "../common/xmlgeomutils.h"


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
      // CSUString texture_path;
      // SU_CALL(SUTextureGetFileName(texture, texture_path));
      // info.texture_path_ = texture_path.utf8();

      // Texture scale
      size_t width = 0;
      size_t height = 0;
      double s_scale = 0.0;
      double t_scale = 0.0;
      SU_CALL(SUTextureGetDimensions(texture, &width, &height,
                                     &s_scale, &t_scale));
      info.texture_sscale_ = s_scale;
      info.texture_tscale_ = t_scale;
      info.width_ = width;
      info.height_ = height;
    
      //Texture data
      SU_CALL(SUTextureGetImageRep(texture,&image_rep));
      //to fix jpg bug,we convert all images to 32 bit
      SUImageRepConvertTo32BitsPerPixel(image_rep);

      size_t data_size=0,bits_per_pixel=0;
      SU_CALL(SUImageRepGetDataSize(image_rep,&data_size,&bits_per_pixel));

      info.data_size_=data_size;
      info.bits_per_pixel_=bits_per_pixel;
      int image_size = width*height*bits_per_pixel / 8;
      info.pixel_data_=new SUByte[data_size];
      SU_CALL(SUImageRepGetData(image_rep, data_size,info.pixel_data_));
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