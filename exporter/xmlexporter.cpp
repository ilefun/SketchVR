// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <ctime>

#include "./xmlexporter.h"
#include "./xmltexturehelper.h"
#include "../common/xmlgeomutils.h"
#include "../common/utils.h"

#include <SketchUpAPI/import_export/pluginprogresscallback.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/drawing_element.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/entity.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/loop.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/texture_writer.h>
#include <SketchUpAPI/model/uv_helper.h>
#include <SketchUpAPI/model/vertex.h>
#include <SketchUpAPI/model/image_rep.h>

using namespace XmlGeomUtils;
using namespace std;

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

  std::string utf8() {
    size_t length;
    SUStringGetUTF8Length(su_str_, &length);
    std::string string;
    string.resize(length+1);
    size_t returned_length;
    SUStringGetUTF8(su_str_, length, &string[0], &returned_length);
    return string;
  }

private:
  // Disallow copying for simplicity
  CSUString(const CSUString& copy);
  CSUString& operator= (const CSUString& copy);

  SUStringRef su_str_;
};

// Utility function to get a material's name
static std::string GetMaterialName(SUMaterialRef material) {
  CSUString name;
  SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
  return name.utf8();
}

// Utility function to get a layer's name
static std::string GetLayerName(SULayerRef layer) {
  CSUString name;
  SU_CALL(SULayerGetName(layer, name));
  return name.utf8();
}

// Utility function to get a component definition's name
static std::string GetComponentDefinitionName(
    SUComponentDefinitionRef comp_def) {
  CSUString name;
  SU_CALL(SUComponentDefinitionGetName(comp_def, name));
  return name.utf8();
}

CXmlExporter::CXmlExporter() {
  SUSetInvalid(model_);
  SUSetInvalid(texture_writer_);
  SUSetInvalid(image_rep_);
  max_vertex_num_pergroup_=50000;
}

CXmlExporter::~CXmlExporter() {
}

void CXmlExporter::ReleaseModelObjects() {
  if (!SUIsInvalid(texture_writer_)) {
    SUTextureWriterRelease(&texture_writer_);
    SUSetInvalid(texture_writer_);
  }

  if (!SUIsInvalid(image_rep_)) {
    SUImageRepRelease(&image_rep_);
    SUSetInvalid(image_rep_);
  }

  if (!SUIsInvalid(model_)) {
    SUModelRelease(&model_);
    SUSetInvalid(model_);
  }

  // Terminate the SDK
  SUTerminate();
}

bool CXmlExporter::Convert(const std::string& from_file,
    SketchUpPluginProgressCallback* progress_callback) {
  bool exported = false;
  skp_file_ = from_file;

  try {

    // Initialize the SDK
    SUInitialize();

    // Create the model from the src_file
    SUSetInvalid(model_);

  	// Create the model from the src_file
  #ifdef TIME_LOGGER
	clock_t start, end;
    start = clock();
  #endif

  	SUSetInvalid(model_);
  	SU_CALL(SUModelCreateFromFile(&model_, skp_file_.c_str()));
  
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Open file in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl<<endl;
  #endif
  
    std::cout << "Initialize skp file " << skp_file_ << std::endl;

    // Create a texture writer
    SUSetInvalid(texture_writer_);
    SU_CALL(SUTextureWriterCreate(&texture_writer_));

    // Create a texture writer
    SUSetInvalid(image_rep_);
    SU_CALL(SUImageRepCreate(&image_rep_));

    // Write file header
    int major_ver = 0, minor_ver = 0, build_no = 0;
    SU_CALL(SUModelGetVersion(model_, &major_ver, &minor_ver, &build_no));

     // Layers
  	std::cout << "Exporting layers..." << std::endl;
    WriteLayers();



    // // Materials
  #ifdef TIME_LOGGER
    start = clock();
  #endif
  	std::cout << "Exporting materials..."<<std::endl;
    WriteMaterials();
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Export materials in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl<<endl;
  #endif





  #ifdef TIME_LOGGER
    start = clock();
  #endif
    // Component definitions
    std::cout<<"Exporting component data..."<<std::endl;
    WriteComponentDefinitions();
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Export component in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl<<endl;
  #endif




    // Geometry
  #ifdef TIME_LOGGER
    start = clock();
  #endif

    std::cout<<"Exporting geometry data..."<<std::endl;
    WriteGeometry();
  #ifdef TIME_LOGGER
    
    end = clock();
    cout<<"Time Logger : Export geometry in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl<<endl;
  #endif


#ifdef TIME_LOGGER
	start = clock();
#endif
	std::cout << "Combine final faces..." << std::endl;
     std::vector<SUTransformation> transform;
     final_faces_.resize(2);
     CombineEntities(&skpdata_.entities_, final_faces_,transform);
#ifdef TIME_LOGGER

	 end = clock();
	 cout << "Time Logger : Combine final faces in " << (double((end - start)) / CLOCKS_PER_SEC) << "s" << endl << endl;
#endif

#ifdef TIME_LOGGER
  start = clock();
#endif
     FixNormal();
#ifdef TIME_LOGGER

   end = clock();
   cout << "Time Logger : Fix normal in " << (double((end - start)) / CLOCKS_PER_SEC) << "s" << endl << endl;
#endif

	 #ifdef PRINT_SKP_DATA
	 std::cout << "Final face group size : " << final_faces_.size() << std::endl;
	 for (size_t i = 0; i < final_faces_.size(); i++)
	 {
		 std::cout << "\tGroup index "<<i<<" Face num is : "<<final_faces_[i].face_num_ << std::endl;
	 }

	  #endif // PRINT_SKP_DATA

  	std::cout << "Export complete." << std::endl;
    exported = true;

	//SU_CALL(SUModelSaveToFile(model_,"D:\\sketchup\\test_skp_file\\out.skp"));
  } catch(...) {
    exported = false;
  }
  ReleaseModelObjects();

  return exported;
}

void CXmlExporter::CombineEntities(XmlEntitiesInfo *entities,
									EntitiyList &faces_group,
									std::vector<SUTransformation> &transforms,
									size_t index,
									bool combine_component)
{
  if(combine_component)
  {
    //get instance
    for (int i = 0; i < entities->component_instances_.size(); ++i)
    {
      auto entities_list=definition_faces_[entities->component_instances_[i].definition_name_];
      
      transforms.push_back(entities->component_instances_[i].transform_);
      CombineEntities(&entities_list[0],faces_group,transforms,0,combine_component);
	  CombineEntities(&entities_list[1], faces_group, transforms, 1, combine_component);
      transforms.pop_back();
    }
  }
  else{
    for (int i = 0; i < entities->component_instances_.size(); ++i)
    {
      auto entities_list=definition_faces_[entities->component_instances_[i].definition_name_];

      transforms.push_back(entities->component_instances_[i].transform_);
      GetTransformedFace(&faces_group[0],&entities_list[0],transforms);
      GetTransformedFace(&faces_group[1],&entities_list[1],transforms);
      transforms.pop_back();
    }    
  }

  entities->component_instances_.clear();
  vector <XmlComponentInstanceInfo>().swap(entities->component_instances_);



  // get group
  for (int i = 0; i < entities->groups_.size(); ++i)
  {
    transforms.push_back(entities->groups_[i].transform_);
    CombineEntities(entities->groups_[i].entities_,faces_group,transforms,index,combine_component);
    transforms.pop_back();
  }
  entities->groups_.clear();
  vector <XmlGroupInfo>().swap(entities->groups_);



  //get face
  GetTransformedFace(&faces_group[index],entities,transforms);
  entities->faces_.clear();
  vector <XmlFaceInfo>().swap(entities->faces_);

}

void CXmlExporter::GetTransformedFace(XmlEntitiesInfo *to_entities,
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

void CXmlExporter::FixNormal(){
    //we only check and fix the normal of first group,faces in second group are always facing camera
    for (int j = 0; j < final_faces_[0].faces_.size(); ++j)
    {
      for (int k = 0; k < final_faces_[0].faces_[j].face_num_; ++k)
      {
		  if (!XmlGeomUtils::NormalEqual(final_faces_[0].faces_[j].vertices_[k * 3].vertex_,
			  final_faces_[0].faces_[j].vertices_[k * 3 + 1].vertex_,
			  final_faces_[0].faces_[j].vertices_[k * 3 + 2].vertex_,
			  final_faces_[0].faces_[j].face_normal_))
			  swap(final_faces_[0].faces_[j].vertices_[k * 3], final_faces_[0].faces_[j].vertices_[k * 3 + 2]);
      }
    }
}

void CXmlExporter::WriteLayers() {
  if (options_.export_layers()) {

    // Get the number of layers
    size_t num_layers = 0;
    SU_CALL(SUModelGetNumLayers(model_, &num_layers));
    if (num_layers > 0) {
      // Get the layers
      std::vector<SULayerRef> layers(num_layers);
      SU_CALL(SUModelGetLayers(model_, num_layers, &layers[0], &num_layers));
      // Write out each layer
      for (size_t i = 0; i < num_layers; i++) {
        SULayerRef layer = layers[i];
        WriteLayer(layer);
      }
    }

  }
}

XmlMaterialInfo CXmlExporter::GetMaterialInfo(SUMaterialRef material) {
  assert(!SUIsInvalid(material));

  XmlMaterialInfo info;
  // Name
  info.name_ = GetMaterialName(material);

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
      SU_CALL(SUTextureGetImageRep(texture,&image_rep_));
	  //to fix jpg bug,we convert all images to 32 bit
	  SUImageRepConvertTo32BitsPerPixel(image_rep_);

      size_t data_size=0,bits_per_pixel=0;
      SU_CALL(SUImageRepGetDataSize(image_rep_,&data_size,&bits_per_pixel));

      info.data_size_=data_size;
      info.bits_per_pixel_=bits_per_pixel;
	  int image_size = width*height*bits_per_pixel / 8;
	  //if (image_size != data_size)
		 // cout << "Error : image size is not equal to the size from su." << endl;
      info.pixel_data_=new SUByte[data_size];
	    //std::cout << width << " " << height << " " << data_size << " " << bits_per_pixel << std::endl;
	    // std::cout <<std::endl<<"===" <<SUImageRepGetData(image_rep_, info.data_size_, info.pixel_data_);
      SU_CALL(SUImageRepGetData(image_rep_, data_size,info.pixel_data_));
	  //SUImageRepSaveToFile(image_rep_,"D:\\sketchup\\test_skp_file\\a.jpg");
	  //for (size_t i = 0; i < 10; i++)
	  //{
		 // std::cout << int(info.pixel_data_[i]) << " ";
	  //}
	  //std::cout << std::endl;
    }
  }
  return info;
}

void CXmlExporter::WriteLayer(SULayerRef layer) {
  if (SUIsInvalid(layer))
    return;

  XmlLayerInfo info;

  // Name
  info.name_ = GetLayerName(layer);

  // Color
  SUMaterialRef material = SU_INVALID;
  info.has_material_info_ = false;
  if (SULayerGetMaterial(layer, &material) == SU_ERROR_NONE) {
    info.has_material_info_ = true;
	info.material_info_=GetMaterialInfo(material);
  }

  // Visibility
  bool is_visible = true;
  SU_CALL(SULayerGetVisibility(layer, &is_visible));
  info.is_visible_ = is_visible;

  skpdata_.layers_.push_back(info);
  stats_.AddLayer();

}

void CXmlExporter::WriteMaterials() {
  if (options_.export_materials()) {
    if (options_.export_materials_by_layer()) {
      size_t num_layers;
      SU_CALL(SUModelGetNumLayers(model_, &num_layers));
      if (num_layers > 0) {
        std::vector<SULayerRef> layers(num_layers);
        SU_CALL(SUModelGetLayers(model_, num_layers, &layers[0], &num_layers));

        for (size_t i = 0; i < num_layers; i++)  {
          SULayerRef layer = layers[i];
          SUMaterialRef material = SU_INVALID;
          if (SULayerGetMaterial(layer, &material) == SU_ERROR_NONE) {
            WriteMaterial(material);
          }
        }

      }
    } else {
      size_t count = 0;
      SU_CALL(SUModelGetNumMaterials(model_, &count));
      if (count > 0) {

        std::vector<SUMaterialRef> materials(count);
        SU_CALL(SUModelGetMaterials(model_, count, &materials[0], &count));
        for (size_t i=0; i<count; i++) {
			//std::cout <<std::endl<<"mat index : "<< i << std::endl;
          WriteMaterial(materials[i]);
        }

      }
    }
    
	for (size_t i = 0; i < skpdata_.materials_.size(); i++)
	{
		matname_id_map_[skpdata_.materials_[i].name_] = int(i);
		//std::cout << i<<" " << skpdata_.materials_[i].name_ << std::endl;
	}
  }
}

void CXmlExporter::WriteMaterial(SUMaterialRef material) {
  if (SUIsInvalid(material))
    return;  
  skpdata_.materials_.push_back(GetMaterialInfo(material));

}

void CXmlExporter::WriteGeometry() {
  if (options_.export_faces() || options_.export_edges()) {
    // Write entities
    SUEntitiesRef model_entities;
    SU_CALL(SUModelGetEntities(model_, &model_entities));

    WriteEntities(model_entities,&skpdata_.entities_);

  }
}

void CXmlExporter::WriteComponentDefinitions() {
  size_t num_comp_defs = 0;
  SU_CALL(SUModelGetNumComponentDefinitions(model_, &num_comp_defs));
  if (num_comp_defs > 0) {


    std::vector<SUComponentDefinitionRef> comp_defs(num_comp_defs);
    SU_CALL(SUModelGetComponentDefinitions(model_, num_comp_defs, &comp_defs[0],
                                           &num_comp_defs));
#ifdef PRINT_SKP_DATA
	std::cout << endl << "Component Num : " << num_comp_defs << std::endl;

#endif // PRINT_SKP_DATA

	std::vector<string> definition_name;
    for (size_t def = 0; def < num_comp_defs; ++def) {
      SUComponentDefinitionRef comp_def = comp_defs[def];
      definition_name.push_back(WriteComponentDefinition(comp_def));
    }

	CombineComponentDefinitions(definition_name);
  }
}

void CXmlExporter::CombineComponentDefinitions(std::vector<std::string> definition_name_list) {
	//we need the right order of the component name list to get the face data
	for (size_t i = 0; i < definition_name_list.size(); i++)
	{
		std::vector<SUTransformation> transform;
		EntitiyList faces_data(2);

		int index = 0;
		if (skpdata_.behavior_[definition_name_list[i]].component_always_face_camera)
			index = 1;

		CombineEntities(&skpdata_.definitions_[definition_name_list[i]], faces_data, transform, index, true);

		definition_faces_[definition_name_list[i]] = faces_data;

#ifdef PRINT_SKP_DATA

		cout << "Combined component " << definition_name_list[i] << " Face size " << faces_data[0].face_num_ << " , " << faces_data[1].face_num_ << endl;
#endif // PRINT_SKP_DATA
	}
	for (auto it = skpdata_.definitions_.begin(); it != skpdata_.definitions_.end(); ++it) {


	}

}

std::string CXmlExporter::WriteComponentDefinition(SUComponentDefinitionRef comp_def) {
  auto def_name = GetComponentDefinitionName(comp_def);

#ifdef PRINT_SKP_DATA
  std::cout << endl<<"Component Name : " << def_name << std::endl;

#endif // PRINT_SKP_DATA

  SUEntitiesRef entities = SU_INVALID;
  SUComponentDefinitionGetEntities(comp_def, &entities);
  XmlEntitiesInfo entity_info;
  WriteEntities(entities,&entity_info);
  skpdata_.definitions_[def_name]= entity_info;

  SUComponentBehavior behavior;
  SUComponentDefinitionGetBehavior(comp_def, &behavior);
  skpdata_.behavior_[def_name]=behavior;

  return def_name;
}

void CXmlExporter::WriteEntities(SUEntitiesRef entities,XmlEntitiesInfo *entity_info) {
  // Component instances
  size_t num_instances = 0;
  SU_CALL(SUEntitiesGetNumInstances(entities, &num_instances));
  if (num_instances > 0) {
#ifdef PRINT_SKP_DATA
	  std::cout << "Instance Num : " << num_instances << std::endl;

#endif // PRINT_SKP_DATA
    std::vector<SUComponentInstanceRef> instances(num_instances);
    SU_CALL(SUEntitiesGetInstances(entities, num_instances,
                                   &instances[0], &num_instances));
    for (size_t c = 0; c < num_instances; c++) {
      SUComponentInstanceRef instance = instances[c];
      SUComponentDefinitionRef definition = SU_INVALID;
      SU_CALL(SUComponentInstanceGetDefinition(instance, &definition));


      // convert component to entities---------------
      // XmlGroupInfo info;
      // SU_CALL(SUComponentInstanceGetTransform(instance, &info.transform_));

      // // Write entities
      // SUEntitiesRef  comp_entity_ref = SU_INVALID;
      // SU_CALL(SUComponentDefinitionGetEntities(definition,&comp_entity_ref));
      // WriteEntities(comp_entity_ref,info.entities_);

      // entity_info->groups_.push_back(info);

      XmlComponentInstanceInfo instance_info;
      
      // Layer
      SULayerRef layer = SU_INVALID;
      
      
      SUDrawingElementGetLayer(SUComponentInstanceToDrawingElement(instance),
                               &layer);
      if (!SUIsInvalid(layer))
        instance_info.layer_name_ = GetLayerName(layer);

      // Material
      SUMaterialRef material = SU_INVALID;
      SUDrawingElementGetMaterial(SUComponentInstanceToDrawingElement(instance),
                                  &material);
      if (!SUIsInvalid(material))
        instance_info.material_name_ = GetMaterialName(material);

      instance_info.definition_name_ = GetComponentDefinitionName(definition);

      SU_CALL(SUComponentInstanceGetTransform(instance,
                                              &instance_info.transform_));

#ifdef PRINT_SKP_DATA
	  std::cout << "\tInstance Index : " << c << " Name : " << instance_info.definition_name_ << std::endl;
	  std::cout << "\tXform : "<<endl;
	  for (size_t i = 0; i < 4; i++) {
		  cout << "\t\t";
		  for (size_t j = 0; j < 4; j++)
		  {
			  std::cout << instance_info.transform_.values[i*4+j] << " ";
		  }
		  cout << endl;
	  }
	  std::cout << endl<<endl;
#endif // PRINT_SKP_DATA

      entity_info->component_instances_.push_back(instance_info);
    }
  }

  // Groups
  size_t num_groups = 0;
  SU_CALL(SUEntitiesGetNumGroups(entities, &num_groups));
  if (num_groups > 0) {
    std::vector<SUGroupRef> groups(num_groups);
    SU_CALL(SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups));
    for (size_t g = 0; g < num_groups; g++) {
      SUGroupRef group = groups[g];
      SUEntitiesRef group_entities = SU_INVALID;
      SU_CALL(SUGroupGetEntities(group, &group_entities));
      inheritance_manager_.PushElement(group);

      // Write transformation
      XmlGroupInfo info;
      SU_CALL(SUGroupGetTransform(group, &info.transform_));

      // Write entities
      WriteEntities(group_entities,info.entities_);

      entity_info->groups_.push_back(info);

      inheritance_manager_.PopElement();
    }
  }

  // Faces
  if (options_.export_faces()) {
    size_t num_faces = 0;
    SU_CALL(SUEntitiesGetNumFaces(entities, &num_faces));
    if (num_faces > 0) {
      std::vector<SUFaceRef> faces(num_faces);
      SU_CALL(SUEntitiesGetFaces(entities, num_faces, &faces[0], &num_faces));
      for (size_t i = 0; i < num_faces; i++) {
        inheritance_manager_.PushElement(faces[i]);
        WriteFace(faces[i],entity_info);
        inheritance_manager_.PopElement();
      }
    }
  }

  // Edges
  if (options_.export_edges()) {
    size_t num_edges = 0;
    bool standAloneOnly = true; // Write only edges not connected to faces.
    SU_CALL(SUEntitiesGetNumEdges(entities, standAloneOnly, &num_edges));
    if (num_edges > 0) {
      std::vector<SUEdgeRef> edges(num_edges);
      SU_CALL(SUEntitiesGetEdges(entities, standAloneOnly, num_edges,
                                 &edges[0], &num_edges));
      for (size_t i = 0; i < num_edges; i++) {
        inheritance_manager_.PushElement(edges[i]);
        WriteEdge(edges[i],entity_info);
        inheritance_manager_.PopElement();
      }
    }
  }

  // Curves
  if (options_.export_edges()) {
    size_t num_curves = 0;
    SU_CALL(SUEntitiesGetNumCurves(entities, &num_curves));
    if (num_curves > 0) {
      std::vector<SUCurveRef> curves(num_curves);
      SU_CALL(SUEntitiesGetCurves(entities, num_curves,
                                  &curves[0], &num_curves));
      for (size_t i = 0; i < num_curves; i++) {
        WriteCurve(curves[i],entity_info);
      }
    }
  }
}

void CXmlExporter::WriteFace(SUFaceRef face,XmlEntitiesInfo *entity_info) {
  if (SUIsInvalid(face))
    return;

  XmlFaceInfo info;

  // Get Current layer off of our stack and then get the id from it
  SULayerRef layer = inheritance_manager_.GetCurrentLayer();
  if (!SUIsInvalid(layer)) {
    info.layer_name_ = GetLayerName(layer);
  }

  // Get the current front and back materials off of our stack
  if (options_.export_materials()) {
    SUMaterialRef front_material =
        inheritance_manager_.GetCurrentFrontMaterial();
    if (!SUIsInvalid(front_material)) {
      // Material name
      info.front_mat_name_ = GetMaterialName(front_material);

      // Has texture ?
      SUTextureRef texture_ref = SU_INVALID;
      info.has_front_texture_ =
          SUMaterialGetTexture(front_material, &texture_ref) == SU_ERROR_NONE;
    }
    SUMaterialRef back_material =
        inheritance_manager_.GetCurrentBackMaterial();
    if (!SUIsInvalid(back_material)) {
      // Material name
      info.back_mat_name_ = GetMaterialName(back_material);

      // Has texture ?
      SUTextureRef texture_ref = SU_INVALID;
      info.has_back_texture_ =
          SUMaterialGetTexture(back_material, &texture_ref) == SU_ERROR_NONE;
    }
  }
  bool has_texture = info.has_front_texture_ || info.has_back_texture_;

  // Get face normal
  SUVector3D face_normal;
  SUFaceGetNormal (face, &face_normal);
  info.face_normal_=CVector3d(face_normal.x,face_normal.y,face_normal.z);

  // Get a uv helper
  SUUVHelperRef uv_helper = SU_INVALID;
  SUFaceGetUVHelper(face, info.has_front_texture_, info.has_back_texture_,
                    texture_writer_, &uv_helper);

  // Find out how many loops the face has
  size_t num_loops = 0;
  SU_CALL(SUFaceGetNumInnerLoops(face, &num_loops));
  num_loops++;  // add the outer loop

  if (num_loops == 1 && options_.get_triangle()==false) {
    // Simple Face
    info.has_single_loop_ = true;
    
    SULoopRef outer_loop = SU_INVALID;
    SU_CALL(SUFaceGetOuterLoop(face, &outer_loop));
    size_t num_vertices;
    SU_CALL(SULoopGetNumVertices(outer_loop, &num_vertices));
    if (num_vertices > 0) {
	  info.face_num_ = 1;
      std::vector<SUVertexRef> vertices(num_vertices);
      SU_CALL(SULoopGetVertices(outer_loop, num_vertices, &vertices[0],
                                &num_vertices));
      for (size_t i = 0; i < num_vertices; i++) {
        XmlFaceVertex vertex_info;
        // vertex position
        SUPoint3D su_point;
        SUVertexRef vertex_ref = vertices[i];
        SU_CALL(SUVertexGetPosition(vertex_ref, &su_point));
        vertex_info.vertex_ = CPoint3d(su_point);

        // texture coordinates
        if (info.has_front_texture_) {
          SUUVQ uvq;
          if (SUUVHelperGetFrontUVQ(uv_helper, &su_point, &uvq) ==
              SU_ERROR_NONE) {
            vertex_info.front_texture_coord_ = CPoint3d(uvq.u, uvq.v, 0);
          }
        }

        if (info.has_back_texture_) {
          SUUVQ uvq;
          if (SUUVHelperGetBackUVQ(uv_helper, &su_point, &uvq) ==
              SU_ERROR_NONE) { 
            vertex_info.back_texture_coord_ = CPoint3d(uvq.u, uvq.v, 0);
          }
        }
        info.vertices_.push_back(vertex_info);
      }
    }
  } else {
    // If this is a complex face with one or more holes in it
    // we tessellate it into triangles using the polygon mesh class, then
    // export each triangle as a face.
    info.has_single_loop_ = false;

    // Create a mesh from face.
    SUMeshHelperRef mesh_ref = SU_INVALID;
    SU_CALL(SUMeshHelperCreateWithTextureWriter(&mesh_ref, face,
                                                texture_writer_));

    // Get the vertices
    size_t num_vertices = 0;
    SU_CALL(SUMeshHelperGetNumVertices(mesh_ref, &num_vertices));
    if (num_vertices == 0)
      return;

    std::vector<SUPoint3D> vertices(num_vertices);
    SU_CALL(SUMeshHelperGetVertices(mesh_ref, num_vertices,
                                    &vertices[0], &num_vertices));

    // Get triangle indices.
    size_t num_triangles = 0;
    SU_CALL(SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles));
  	info.face_num_ = int(num_triangles);

    const size_t num_indices = 3 * num_triangles;
    size_t num_retrieved = 0;
    std::vector<size_t> indices(num_indices);
    SU_CALL(SUMeshHelperGetVertexIndices(mesh_ref, num_indices,
                                         &indices[0], &num_retrieved));

    // Get UV coords.
    std::vector<SUPoint3D> front_stq(num_vertices);
    std::vector<SUPoint3D> back_stq(num_vertices);
    size_t count;
    if (info.has_front_texture_) {
      SU_CALL(SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices,
                                            &front_stq[0], &count));
    }

    if (info.has_back_texture_) {
      SU_CALL(SUMeshHelperGetBackSTQCoords(mesh_ref, num_vertices,
                                           &back_stq[0], &count));
    }

    for (size_t i_triangle = 0; i_triangle < num_triangles; i_triangle++) {

      // Three points in each triangle
      for (size_t i = 0; i < 3; i++) {
        XmlFaceVertex vertex_info;
        // Get vertex
        size_t index = indices[i_triangle * 3 + i];
        vertex_info.vertex_.SetLocation(vertices[index].x,
                                        vertices[index].y,
                                        vertices[index].z);

        if (info.has_front_texture_) {
          SUPoint3D stq = front_stq[index];
          vertex_info.front_texture_coord_ = CPoint3d(stq.x, stq.y, 0);
        }

        if (info.has_back_texture_) {
          SUPoint3D stq = back_stq[index];
          vertex_info.back_texture_coord_ = CPoint3d(stq.x, stq.y, 0);
        }
        info.vertices_.push_back(vertex_info);
      }
    }
  }
  // skpdata_.entities_.faces_.push_back(info);
  entity_info->faces_.push_back(info);

  stats_.AddFace();


  SU_CALL(SUUVHelperRelease(&uv_helper));
}

XmlEdgeInfo CXmlExporter::GetEdgeInfo(SUEdgeRef edge) const {
  XmlEdgeInfo info;
  info.has_layer_ = false;
  if (options_.export_layers()) {
    info.has_layer_ = true;
    SULayerRef layer = inheritance_manager_.GetCurrentLayer();
    if (!SUIsInvalid(layer)) {
      SU_CALL(SUDrawingElementGetLayer(SUEdgeToDrawingElement(edge), &layer));
      info.layer_name_ = GetLayerName(layer);
    }
  }

  // Edge color
  info.has_color_ = false;
  if (options_.export_materials()) {
    info.color_ = inheritance_manager_.GetCurrentEdgeColor();
    info.has_color_ = true;
  }

  info.start_ = CPoint3d(-1, -1, -1);
  SUVertexRef start_vertex = SU_INVALID;
  SU_CALL(SUEdgeGetStartVertex(edge, &start_vertex));
  SUPoint3D p;
  SU_CALL(SUVertexGetPosition(start_vertex, &p));
  info.start_ = CPoint3d(p);

  info.end_ = CPoint3d(-1, -1, -1);
  SUVertexRef end_vertex = SU_INVALID;
  SU_CALL(SUEdgeGetEndVertex(edge, &end_vertex));
  SU_CALL(SUVertexGetPosition(end_vertex, &p));
  info.end_ = CPoint3d(p);

  return info;
}

void CXmlExporter::WriteEdge(SUEdgeRef edge,XmlEntitiesInfo *entity_info) {
  if (SUIsInvalid(edge))
    return;

  XmlEdgeInfo info = GetEdgeInfo(edge);
  // skpdata_.entities_.edges_.push_back(info);

  entity_info->edges_.push_back(info);

  stats_.AddEdge();
}

void CXmlExporter::WriteCurve(SUCurveRef curve,XmlEntitiesInfo *entity_info) {
  if (SUIsInvalid(curve))
    return;

  XmlCurveInfo info;
  size_t num_edges = 0;
  SU_CALL(SUCurveGetNumEdges(curve, &num_edges));
  std::vector<SUEdgeRef> edges(num_edges);
  SU_CALL(SUCurveGetEdges(curve, num_edges, &edges[0], &num_edges));
  for (size_t i = 0; i < num_edges; ++i) {
    XmlEdgeInfo edge_info = GetEdgeInfo(edges[i]);
    info.edges_.push_back(edge_info);
  }
  
  entity_info->curves_.push_back(info);
  // skpdata_.entities_.curves_.push_back(info);


}

void CXmlExporter::debug_print()
{
	std::cout << std::endl << "Exporter debug print starts==========" << std::endl;

	std::cout << "Face num: " << skpdata_.entities_.faces_.size() << std::endl;
	for (size_t i = 0; i < skpdata_.entities_.faces_.size(); i++)
	{
		if (skpdata_.entities_.faces_[i].has_single_loop_)
			std::cout << std::endl << "Face index: " << i << std::endl;
		else
			std::cout << std::endl << "Face index : " << i <<std::endl<< "Contains " << skpdata_.entities_.faces_[i].vertices_.size() / 3 << "triangles" << std::endl;
		for (size_t j = 0; j < skpdata_.entities_.faces_[i].vertices_.size(); j++)
			std::cout << skpdata_.entities_.faces_[i].vertices_[j].vertex_.x() << " " \
			<< skpdata_.entities_.faces_[i].vertices_[j].vertex_.y() << " " \
			<< skpdata_.entities_.faces_[i].vertices_[j].vertex_.z() << std::endl;
	}

	std::cout <<std::endl<< "Exporter debug print ends==========" << std::endl;
}
