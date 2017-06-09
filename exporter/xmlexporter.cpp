// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <ctime>
#include <omp.h>

#include "./xmlexporter.h"
#include "./xmltexturehelper.h"
#include "../common/xmlgeomutils.h"
#include "./exportutils.h"
#include "../common/utils.h"
#include "../common/stringutils.h"

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
#include <SketchUpAPI/model/scene.h>
#include <SketchUpAPI/model/camera.h>


using namespace XmlGeomUtils;
using namespace std;


CXmlExporter::CXmlExporter() {
  SUSetInvalid(model_);
  SUSetInvalid(texture_writer_);
  SUSetInvalid(image_rep_);
}

CXmlExporter::~CXmlExporter() {
}

void CXmlExporter::ReleaseModelObjects() {
  if (!SUIsInvalid(texture_writer_)) {
    SUTextureWriterRelease(&texture_writer_);
    SUSetInvalid(texture_writer_);
    cout << "Released texture write." << endl;
  }
  
  if (!SUIsInvalid(image_rep_)) {
    SUImageRepRelease(&image_rep_);
    SUSetInvalid(image_rep_);
    cout << "Release image rep." << endl;
  }
  
  if (!SUIsInvalid(model_)) {
    SUModelRelease(&model_);
    SUSetInvalid(model_);
    cout << "Released su model." << endl;
  }

  // Terminate the SDK
  SUTerminate();
  cout << "Terminated sdk." << endl;
}

bool CXmlExporter::Convert(const std::string& from_file,
    SketchUpPluginProgressCallback* progress_callback) {
  bool exported = false;
  skp_file_ = from_file;

  face_camera_id_.clear();
  face_camera_normal_.clear();

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

	SU_CALL(SUModelCreateFromFile(&model_, skp_file_.c_str()));


  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Open file in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl<<endl;
  #endif
  
    std::cout << "Initialize skp file " << StringConvertUtils::UTF8_To_string(from_file) << std::endl;

    // Create a texture writer
    SUSetInvalid(texture_writer_);
    SU_CALL(SUTextureWriterCreate(&texture_writer_));

    // Create a texture writer
    SUSetInvalid(image_rep_);
    SU_CALL(SUImageRepCreate(&image_rep_));

    // Write file header
    int major_ver = 0, minor_ver = 0, build_no = 0;
    SU_CALL(SUModelGetVersion(model_, &major_ver, &minor_ver, &build_no));
    
    //Scenes
    std::cout << "Exporting scenes..." << std::endl;
    WriteScenes();

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
     ExportUtils::FixNormal(final_faces_[0]);
     ExportUtils::FixNormal(final_faces_[1]);
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
	 std::cout <<endl<< "Face camera group size : " << face_camera_id_.size() << " , Face camera normal size : " << face_camera_normal_.size() << std::endl;
	  #endif // PRINT_SKP_DATA

  	std::cout << endl<<endl<<"Export completed,data is ready." << std::endl;
    exported = true;

	//SU_CALL(SUModelSaveToFile(model_,"D:\\sketchup\\test_skp_file\\out.skp"));
  } catch(...) {
    exported = false;
  }
  ReleaseModelObjects();

  return exported;
}

void CXmlExporter::CombineEntities(XmlEntitiesInfo *entities,
									CXmlExporter::EntityList &faces_group,
									std::vector<SUTransformation> &transforms,
									size_t index)
{
  // get group
  for (int i = 0; i < entities->groups_.size(); ++i)
  {
    size_t current_index=index;
    auto comp_name=entities->groups_[i].component_name_;
  	if (!comp_name.empty() && skpdata_.defitions_[comp_name].behavior_.component_always_face_camera)
  	{
  		current_index = 1;
  		face_camera_id_.push_back(faces_group[current_index].face_num_);
  	}
    transforms.push_back(entities->groups_[i].transform_);
    CombineEntities(entities->groups_[i].entities_,
                    faces_group,
                    transforms,
                    current_index );
    transforms.pop_back();
  }
  entities->groups_.clear();
  vector <XmlGroupInfo>().swap(entities->groups_);

  //get face
  if(index==0)
    ExportUtils::GetTransformedFace(&faces_group[index],
                      entities,
                      transforms);
  else{
	  XmlGeomUtils::CVector3d temp_vec;
    ExportUtils::GetTransformedFace(&faces_group[index],
                      entities,
                      transforms,
                      &temp_vec);

    face_camera_normal_.push_back(temp_vec);
  }

  entities->faces_.clear();
  vector <XmlFaceInfo>().swap(entities->faces_);

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

void CXmlExporter::WriteLayer(SULayerRef layer) {
  if (SUIsInvalid(layer))
    return;

  XmlLayerInfo info;

  // Name
  info.name_ = ExportUtils::GetLayerName(layer);

  // Color
  SUMaterialRef material = SU_INVALID;
  info.has_material_info_ = false;
  if (SULayerGetMaterial(layer, &material) == SU_ERROR_NONE) {
    info.has_material_info_ = true;
	info.material_info_=ExportUtils::GetMaterialInfo(material,skpdata_.texture_map_);
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

		skpdata_.materials_.resize(num_layers);
        for (size_t i = 0; i < num_layers; i++)  {
          SULayerRef layer = layers[i];
          SUMaterialRef material = SU_INVALID;
          if (SULayerGetMaterial(layer, &material) == SU_ERROR_NONE) {
            WriteMaterial(material,i);
          }
        }

      }
    } else {
      size_t count = 0;
      SU_CALL(SUModelGetNumMaterials(model_, &count));
      if (count > 0) {

        std::vector<SUMaterialRef> materials(count);
        SU_CALL(SUModelGetMaterials(model_, count, &materials[0], &count));

		skpdata_.materials_.resize(count);

		#pragma omp parallel for  
		for (int i = 0; i < int(count); ++i) {
			//std::cout <<std::endl<<"mat index : "<< i << std::endl;
			WriteMaterial(materials[i], i);
		}

      }
    }
    
	for (size_t i = 0; i < skpdata_.materials_.size(); i++)
	{
		skpdata_.matname_id_map_[skpdata_.materials_[i].name_] = int(i);
		//std::cout << i<<" " << skpdata_.materials_[i].name_ << std::endl;
	}
  }
}

void CXmlExporter::WriteMaterial(SUMaterialRef material,int i) {
  if (SUIsInvalid(material))    return;
  skpdata_.materials_[i]=ExportUtils::GetMaterialInfo(material,skpdata_.texture_map_);
}

void CXmlExporter::WriteScenes(){
    cout << "Get active camera data and save it as first scene." << endl;
    SceneInfo scene_info;
    scene_info.name_ = string("Start");
    SUCameraRef camera;
    SU_CALL(SUModelGetCamera(model_, &camera));
    SU_CALL(SUCameraGetOrientation(camera, &scene_info.position_, &scene_info.target_, &scene_info.up_vector_));
    skpdata_.scenes_.push_back(scene_info);

  size_t count = 0;
  SU_CALL(SUModelGetNumScenes(model_, &count));
  if (count > 0) {
        std::vector<SUSceneRef> scenes(count);
        SU_CALL(SUModelGetScenes(model_, count, &scenes[0], &count));
        for (size_t i=0; i<count; i++) {
          WriteScene(scenes[i]);
        }
  }
}

void CXmlExporter::WriteScene(SUSceneRef scene)
{
    if (SUIsInvalid(scene))
        return;  

    SceneInfo scene_info;
    
    CSUString name;
    SU_CALL(SUSceneGetName(scene, name));
    scene_info.name_=name.utf8();

    SUCameraRef camera;
    SU_CALL(SUSceneGetCamera(scene,&camera));
    SU_CALL(SUCameraGetOrientation (camera, &scene_info.position_,&scene_info.target_,&scene_info.up_vector_));

    skpdata_.scenes_.push_back(scene_info);
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
#ifdef PRINT_SKP_DATA
	  std::cout << endl << "Component Num : " << num_comp_defs << std::endl;
#endif // PRINT_SKP_DATA

    std::vector<SUComponentDefinitionRef> comp_defs(num_comp_defs);
    SU_CALL(SUModelGetComponentDefinitions(model_, num_comp_defs, &comp_defs[0],
                                           &num_comp_defs));
    for (size_t def = 0; def < num_comp_defs; ++def)
      WriteComponentDefinition(comp_defs[def]);
  }
}

std::string CXmlExporter::WriteComponentDefinition(SUComponentDefinitionRef comp_def) {
  auto def_name = ExportUtils::GetComponentDefinitionName(comp_def);

#ifdef PRINT_SKP_DATA
  std::cout << endl<<"Component Name : " << StringConvertUtils::UTF8_To_string(def_name) << std::endl;

#endif // PRINT_SKP_DATA
  DefinitionInfo def_info;
  SUComponentDefinitionGetBehavior(comp_def, &def_info.behavior_);
  SUComponentDefinitionGetInsertPoint(comp_def, &def_info.insert_point_);

  skpdata_.defitions_[def_name]= def_info;

#ifdef PRINT_SKP_DATA
  if (def_info.behavior_.component_always_face_camera)
	  std::cout << "\tBehavior : Face camera" << ", Insert point : " 
				<< def_info.insert_point_.x << " " 
				<< def_info.insert_point_.y << " " 
				<< def_info.insert_point_.z << endl;
#endif // PRINT_SKP_DATA

  return def_name;
}

int CXmlExporter::GetMaterialIdByName(std::string mat_name)
{
  return skpdata_.matname_id_map_[mat_name];
}

bool CXmlExporter::GetFacingCameraFaceId(int id, int *start_face, int *end_face)
{
	auto total_size = face_camera_id_.size();
	assert(id <= total_size-1);
	if (id < total_size - 1) {
		*start_face = face_camera_id_[id];
		*end_face = face_camera_id_[id + 1]-1;
	}
	else if(id==total_size-1)
	{
		*start_face = face_camera_id_[id];
		*end_face = final_faces_[1].face_num_-1;
	}
  
  if(*end_face<*start_face) return false;
  return true;
}

int CXmlExporter::GetSceneSize()
{
  return skpdata_.scenes_.size();
}

void CXmlExporter::GetSceneData(int id,char * scene_name,float position[3],float target[3],float up_vector[3])
{
  assert(id <= skpdata_.scenes_.size() - 1);
  memcpy(scene_name, skpdata_.scenes_[id].name_.c_str(),skpdata_.scenes_[id].name_.size());

  position[0] = skpdata_.scenes_[id].position_.x;
  position[1] = skpdata_.scenes_[id].position_.y;
  position[2] = skpdata_.scenes_[id].position_.z;

  target[0] = skpdata_.scenes_[id].target_.x;
  target[1] = skpdata_.scenes_[id].target_.y;
  target[2] = skpdata_.scenes_[id].target_.z;

  up_vector[0] = skpdata_.scenes_[id].up_vector_.x;
  up_vector[1] = skpdata_.scenes_[id].up_vector_.y;
  up_vector[2] = skpdata_.scenes_[id].up_vector_.z;

}

int CXmlExporter::GetFacingCameraIdSize()
{
	return int(face_camera_id_.size());
}

void CXmlExporter::GetFacingCameraDirection(int id, float direction[3])
{
	assert(id <= face_camera_normal_.size() - 1);
	direction[0] = face_camera_normal_[id].x();
	direction[1] = face_camera_normal_[id].y();
	direction[2] = face_camera_normal_[id].z();
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
    SU_CALL(SUEntitiesGetInstances(entities, num_instances, &instances[0], &num_instances));

	//#pragma omp parallel for num_threads(2)
    for (int c = 0; c < int(num_instances); c++) {
      SUComponentInstanceRef instance = instances[c];
  	  if (ExportUtils::IsGeoHidden(instance)) continue;
      auto draw_element=SUComponentInstanceToDrawingElement(instance);
      if (!IsDrawingElementVisible(draw_element)) continue;

          SUComponentDefinitionRef definition = SU_INVALID;
          SU_CALL(SUComponentInstanceGetDefinition(instance, &definition));

	      auto definition_name = ExportUtils::GetComponentDefinitionName(definition);

	      // Write transformation
	      XmlGroupInfo info;
        SU_CALL(SUComponentInstanceGetTransform(instance,&info.transform_));
        info.component_name_=definition_name;
	      SUEntitiesRef  instance_entities = SU_INVALID;
	      SU_CALL(SUComponentDefinitionGetEntities(definition, &instance_entities));

        //push element------------------
        inheritance_manager_.PushElement(draw_element);
        // Write entities
        WriteEntities(instance_entities, info.entities_);
        entity_info->groups_.push_back(info);
      	//pop element-------------------
      	inheritance_manager_.PopElement();

#ifdef PRINT_SKP_DATA
        if (c < 5)
        {
            std::cout << "\tInstance Index : " << c << " Name : " << StringConvertUtils::UTF8_To_string(definition_name) << std::endl;
            std::cout << "\tFace num : " << info.entities_->faces_.size() << std::endl;
            std::cout << "\tXform : " << endl;
            for (size_t i = 0; i < 4; i++) {
                cout << "\t\t";
                for (size_t j = 0; j < 4; j++)
                {
                    std::cout << info.transform_.values[i * 4 + j] << " ";
                }
                cout << endl;
            }
            std::cout << endl << endl;
        }
#endif // PRINT_SKP_DATA
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
	  if (ExportUtils::IsGeoHidden(group)) continue;
    auto draw_element=SUGroupToDrawingElement(group);
    if (!IsDrawingElementVisible(draw_element)) continue;


          SUEntitiesRef group_entities = SU_INVALID;
          SU_CALL(SUGroupGetEntities(group, &group_entities));

          // Write transformation
          XmlGroupInfo info;
          SU_CALL(SUGroupGetTransform(group, &info.transform_));

          // Write entities
          inheritance_manager_.PushElement(draw_element);
          WriteEntities(group_entities, info.entities_);
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

	  auto current_mat = inheritance_manager_.GetCurrentFrontMaterial();
	  std::vector<std::pair<size_t, bool>> face_no_material;
	  if(SUIsValid(current_mat))
		ExportUtils::CheckFaceMaterial(faces, current_mat,face_no_material);

      for (size_t i = 0; i < num_faces; i++) {
		 if (ExportUtils::IsGeoHidden(faces[i])) continue;
     if (!IsDrawingElementVisible(SUFaceToDrawingElement(faces[i]))) continue;

        inheritance_manager_.PushElement(faces[i]);
        WriteFace(faces[i],entity_info);
        inheritance_manager_.PopElement();
      }

	  if (SUIsValid(current_mat))
		  ExportUtils::ClearFaceMaterial(faces, face_no_material);
    }
  }

  //Images
  size_t num_images = 0;
  SU_CALL(SUEntitiesGetNumImages(entities, &num_images));
  if (num_images > 0)
  {
	  std::vector<SUImageRef> images(num_images);
	  SU_CALL(SUEntitiesGetImages(entities, num_images, &images[0], &num_images));
	  for (size_t i = 0; i < num_images; i++) {
		  WriteImageObject(images[i], entity_info);
	  }
  }

  //// Edges
  //if (options_.export_edges()) {
  //  size_t num_edges = 0;
  //  bool standAloneOnly = true; // Write only edges not connected to faces.
  //  SU_CALL(SUEntitiesGetNumEdges(entities, standAloneOnly, &num_edges));
  //  if (num_edges > 0) {
  //    std::vector<SUEdgeRef> edges(num_edges);
  //    SU_CALL(SUEntitiesGetEdges(entities, standAloneOnly, num_edges,
  //                               &edges[0], &num_edges));
  //    for (size_t i = 0; i < num_edges; i++) {
  //      inheritance_manager_.PushElement(edges[i]);
  //      WriteEdge(edges[i],entity_info);
  //      inheritance_manager_.PopElement();
  //    }
  //  }
  //}

  //// Curves
  //if (options_.export_edges()) {
  //  size_t num_curves = 0;
  //  SU_CALL(SUEntitiesGetNumCurves(entities, &num_curves));
  //  if (num_curves > 0) {
  //    std::vector<SUCurveRef> curves(num_curves);
  //    SU_CALL(SUEntitiesGetCurves(entities, num_curves,
  //                                &curves[0], &num_curves));
  //    for (size_t i = 0; i < num_curves; i++) {
  //      WriteCurve(curves[i],entity_info);
  //    }
  //  }
  //}
}

void CXmlExporter::WriteImageObject(SUImageRef image, XmlEntitiesInfo *entity_info)
{
	int mat_id=ExportUtils::GetImageTextureInfo(image, skpdata_.texture_map_, skpdata_.materials_, skpdata_.matname_id_map_);

	XmlFaceInfo info;
	info.front_mat_name_ = skpdata_.materials_[mat_id].name_;
	info.back_mat_name_ = skpdata_.materials_[mat_id].name_;

	auto tex = skpdata_.texture_map_[skpdata_.materials_[mat_id].texture_key_];
	SUTransformation transform;
	SUImageGetTransform(image, &transform);
    info.has_back_texture_ = true;
    info.has_front_texture_ = true;

    info.face_num_=2;
    info.face_normal_.SetDirection(0,0,1);
    info.face_normal_.Transform(transform.values);
    info.has_single_loop_=false;

	ExportUtils::GetVerticesFromRectangle(tex.width_,
											tex.height_, 
											transform,
											info.vertices_);
	
  entity_info->faces_.push_back(info);

  stats_.AddFace();
}

void CXmlExporter::WriteFace(SUFaceRef face,XmlEntitiesInfo *entity_info) {
  if (SUIsInvalid(face))
    return;
  
  XmlFaceInfo info;

  // Get Current layer off of our stack and then get the id from it
  SULayerRef layer = inheritance_manager_.GetCurrentLayer();
  if (!SUIsInvalid(layer)) {
    info.layer_name_ = ExportUtils::GetLayerName(layer);
  }

  // Get the current front and back materials off of our stack
  if (options_.export_materials()) {
    SUMaterialRef front_material =
        inheritance_manager_.GetCurrentFrontMaterial();
    if (!SUIsInvalid(front_material)) {
      // Material name
      info.front_mat_name_ = ExportUtils::GetMaterialName(front_material);

      // Has texture ?
      SUTextureRef texture_ref = SU_INVALID;
      info.has_front_texture_ =
          SUMaterialGetTexture(front_material, &texture_ref) == SU_ERROR_NONE;
    }

    if(options_.get_both_sides())
    {
      SUMaterialRef back_material =
          inheritance_manager_.GetCurrentBackMaterial();
      if (!SUIsInvalid(back_material)) {
        // Material name
        info.back_mat_name_ = ExportUtils::GetMaterialName(back_material);

        // Has texture ?
        SUTextureRef texture_ref = SU_INVALID;
        info.has_back_texture_ =
            SUMaterialGetTexture(back_material, &texture_ref) == SU_ERROR_NONE;
      }
    }
  }
  // bool has_texture = info.has_front_texture_ || (info.has_back_texture_ && options_.export_both_sides_);

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
    std::vector<SUPoint3D> back_stq;
    size_t count;
    if (info.has_front_texture_) {
      SU_CALL(SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices,
                                            &front_stq[0], &count));
    }

    if (info.has_back_texture_) {
      back_stq.resize(num_vertices);
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
      info.layer_name_ = ExportUtils::GetLayerName(layer);
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

bool CXmlExporter::IsDrawingElementVisible(SUDrawingElementRef element)
{
    if (SUIsInvalid(element)) return true;

    bool is_visible = true;

    SULayerRef layer = SU_INVALID;
    SUDrawingElementGetLayer(element, &layer);
    if (!SUIsInvalid(layer)) SULayerGetVisibility(layer, &is_visible);
    
    return is_visible;
}