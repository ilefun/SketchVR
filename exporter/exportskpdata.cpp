#include "./exportskpdata.h"
#include "./exportutils.h"
#include <assert.h>
#include <SketchUpAPI/transformation.h>
#include <algorithm> 
using namespace std;

void GetFaceData(int v_per_face_list[],
					float vertices_list[],
                    float normal_list[],
					const XmlEntitiesInfo *entities)
{
	int vertex_index = 0;
	int normal_index=0;
	int perface_index = 0;

	for (size_t i = 0; i<entities->faces_.size(); i++)
	{
		if (entities->faces_[i].has_single_loop_)
			v_per_face_list[perface_index++]=entities->faces_[i].vertices_.size();
		else
			for (size_t j = 0; j<entities->faces_[i].vertices_.size() / 3; j++)
				v_per_face_list[perface_index++]=3;
		
		for (size_t j = 0; j < entities->faces_[i].face_num_;j++)
		{
			normal_list[normal_index++]=entities->faces_[i].face_normal_.x();
			normal_list[normal_index++]=entities->faces_[i].face_normal_.y();
			normal_list[normal_index++]=entities->faces_[i].face_normal_.z();
		}

		for (size_t j = 0; j<entities->faces_[i].vertices_.size();j++)
		{
			vertices_list[vertex_index++]=entities->faces_[i].vertices_[j].vertex_.x();
			vertices_list[vertex_index++]=entities->faces_[i].vertices_[j].vertex_.y();
			vertices_list[vertex_index++]=entities->faces_[i].vertices_[j].vertex_.z();
		}
	}
}

const XmlEntitiesInfo* GetEntitiesInfo(CXmlExporter *exporter, int group_id)
{
	return exporter->GroupById(group_id);
}

void GetUVData(bool front_or_back,
				float u_list[],
				float v_list[],
				const XmlEntitiesInfo *entities)
{
	int uv_index = 0;
	if (front_or_back)
		for (size_t i = 0; i < entities->faces_.size(); i++) {
			size_t vertices_size = entities->faces_[i].vertices_.size();
			if (entities->faces_[i].has_front_texture_) {
				for (size_t j = 0; j < vertices_size; j++,uv_index++) {
					u_list[uv_index]=entities->faces_[i].vertices_[j].front_texture_coord_.x();
					v_list[uv_index]=entities->faces_[i].vertices_[j].front_texture_coord_.y();
				}
			}
			else
				for (size_t j = 0; j < vertices_size; j++,uv_index++) {
					u_list[uv_index]=-100;
					v_list[uv_index]=-100;
				}
		}
	else
		for (size_t i = 0; i < entities->faces_.size(); i++) {
			size_t vertices_size = entities->faces_[i].vertices_.size();

			if (entities->faces_[i].has_back_texture_) {
				for (size_t j = 0; j < vertices_size; j++,uv_index++) {
					u_list[uv_index]=entities->faces_[i].vertices_[j].back_texture_coord_.x();
					v_list[uv_index]=entities->faces_[i].vertices_[j].back_texture_coord_.y();
				}
			}
			else
				for (size_t j = 0; j < vertices_size; j++,uv_index++) {
					u_list[uv_index]=-100;
					v_list[uv_index]=-100;
				}
		}
}

void GetFaceMaterialData(CXmlExporter *exporter,
						 int front_id[],
                         int back_id[],
                         const XmlEntitiesInfo *entities)
{
	int front_index=0,back_index=0;
	for (size_t i = 0; i<entities->faces_.size(); i++)
	{
		int current_face_num=1;
		if (!entities->faces_[i].has_single_loop_)
			current_face_num=entities->faces_[i].vertices_.size() / 3;

		if(exporter->skpdata_.matname_id_map_.count(entities->faces_[i].front_mat_name_))
		{	
			auto mat_id=exporter->GetMaterialIdByName(entities->faces_[i].front_mat_name_);
			for(int j=0;j<current_face_num;j++)
				front_id[front_index++]=mat_id;
		}
		else
			for(int j=0;j<current_face_num;j++)
				front_id[front_index++]=-1;

		if(exporter->skpdata_.matname_id_map_.count(entities->faces_[i].back_mat_name_))
		{
			auto mat_id=exporter->GetMaterialIdByName(entities->faces_[i].back_mat_name_);
			for(int j=0;j<current_face_num;j++)
				back_id[back_index++]=mat_id;
		}
		else
			for(int j=0;j<current_face_num;j++)
				back_id[back_index++]=-1;

	}
}

EXPORT CXmlExporter* GetExporter(const char *from_file,bool both_sides)
{
	bool m_bExportMaterials = true;
	bool m_bExportFaces = true;
	bool m_bExportEdges = true;
	bool m_bExportMaterialsByLayer = false;
	bool m_bExportLayers = true;
	bool m_bExportOptions = true;
	bool m_bExportSelectionSet = false;
	bool m_bTriangleFace = true;
	bool m_both_sides = both_sides;
	// int m_bExportMaxVertexNumPerGroup = 50000;

	bool converted = false;
	CXmlExporter* exporter =new CXmlExporter();
	try {
		// Set the user preferences on the exporter
		CXmlOptions options;
		options.set_export_materials(m_bExportMaterials);
		options.set_export_faces(m_bExportFaces);
		options.set_export_edges(m_bExportEdges);
		options.set_export_materials_by_layer(m_bExportMaterialsByLayer);
		options.set_export_layers(m_bExportLayers);
		options.set_export_options(m_bExportOptions);
		options.set_triangle(m_bTriangleFace);
		options.set_both_sides(m_both_sides);
		exporter->SetOptions(options);
		// exporter->SetMaxVertexNumPerGroup(m_bExportMaxVertexNumPerGroup);
		
		// Convert
		converted = exporter->Convert(from_file, NULL);
	}
	catch (...) {
		converted = false;
	}
	if (converted)
		return exporter;
	else
		return NULL;
}

EXPORT void ReleaseExporter(CXmlExporter *exporter)
{
	if (exporter)
	{
        cout <<endl<< "Start to release exporter." << endl;
		//release pixel data memory
		for (auto iter = exporter->skpdata_.texture_map_.begin(); iter != exporter->skpdata_.texture_map_.end(); ++iter)
		{
			if (iter->second.pixel_data_) {
				delete iter->second.pixel_data_;
				iter->second.pixel_data_ = NULL;
			}
		}
        cout << endl << "Released skp pixel data." << endl;

		delete exporter;
		exporter = NULL;
		cout << endl<<"Released exporter." << endl;
	}
}

EXPORT  int GetGroupNum(CXmlExporter *exporter)
{
	return exporter->GroupNum();
}

EXPORT void GetFaceDSize(CXmlExporter *exporter,
                        int group_id,
                        int *vertex_num,
                        int *face_num )
{
	const XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return ;

	*vertex_num = current_entities->vertex_num_;
	*face_num = current_entities->face_num_;
}

EXPORT bool GetFace(CXmlExporter *exporter,
                        int group_id,
						float vertices[],
						int vertex_num_per_face[],
						float face_normal[])
{
	const XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;
	GetFaceData(vertex_num_per_face,
					vertices,
					face_normal,
					current_entities);
	return true;
}

EXPORT bool GetFacingCameraId(CXmlExporter *exporter, int id,int *start_face, int *end_face)
{
	return exporter->GetFacingCameraFaceId(id, start_face, end_face);
}

EXPORT int GetFacingCameraIdSize(CXmlExporter *exporter)
{
	return exporter->GetFacingCameraIdSize();
}

EXPORT void GetFacingCameraDirection(CXmlExporter *exporter,int id, float direction[3])
{
	exporter->GetFacingCameraDirection(id, direction);
}

EXPORT int GetSceneSize(CXmlExporter *exporter)
{
	return exporter->GetSceneSize();
}

EXPORT void GetSceneData(CXmlExporter *exporter,int id,char * scene_name, float position[3], float target[3], float up_vector[3])
{
	exporter->GetSceneData(id, scene_name, position,target,up_vector);
}


EXPORT int GetFaceUVDSize(CXmlExporter *exporter,
                        int group_id,
                        bool front_or_back )
{
	const XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return 0;
	int uv_size=0;
	for (size_t i = 0; i < current_entities->faces_.size(); i++) 
		uv_size+=int(current_entities->faces_[i].vertices_.size());
	return uv_size;
}

EXPORT bool GetFaceUV(CXmlExporter *exporter,
						int group_id,
						bool front_or_back,
						float u[],
						float v[])
{
	const XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;
	GetUVData(front_or_back,u,v,current_entities);
	return true;
}

EXPORT int GetMaterialNum(CXmlExporter *exporter)
{
	return int(exporter->skpdata_.materials_.size());
}

EXPORT const bool GetMaterialNameByID(CXmlExporter *exporter,int id,char *mat_name){
    memcpy(mat_name, exporter->skpdata_.materials_[id].name_.c_str(), exporter->skpdata_.materials_[id].name_.size());
	return true;
}

EXPORT int GetTexPixelDSize(CXmlExporter *exporter, 
                            int material_id )
{
	if(exporter->skpdata_.materials_.at(material_id).has_texture_)
	{
		auto tex_key=exporter->skpdata_.materials_[material_id].texture_key_;
		return exporter->skpdata_.texture_map_[tex_key].data_size_;
	}
	return 0;
}

EXPORT bool GetMaterialData(CXmlExporter *exporter, 
							int material_id,

                            bool *has_color,
                            float color[3],

                            bool *has_alpha,
                            float *alpha,

                            bool *has_texture,
                            float *tex_sscale,
                            float *tex_tscale,
							
							int *origin_bits_per_pixel,
                            int *bits_per_pixel,
                            int *width,
                            int *height,
                            float pixel_data[] )
{
	XmlMaterialInfo current_mat=exporter->skpdata_.materials_.at(material_id);
	
	*has_color=current_mat.has_color_;
	if(*has_color){
		color[0]=float(current_mat.color_.red)/255.0f;
		color[1]=float(current_mat.color_.green)/255.0f;
		color[2]=float(current_mat.color_.blue)/255.0f;
	}
	
	*has_alpha=current_mat.has_alpha_;
	if(*has_alpha)
		*alpha=current_mat.alpha_;

	*has_texture=current_mat.has_texture_;
	if(*has_texture) {
		auto tex_info=exporter->skpdata_.texture_map_[current_mat.texture_key_];
		
		*tex_sscale=tex_info.texture_sscale_;
		*tex_tscale=tex_info.texture_tscale_;
		*bits_per_pixel=tex_info.bits_per_pixel_;
		*origin_bits_per_pixel = tex_info.origin_bits_per_pixel_;
		*width=tex_info.width_;
		*height=tex_info.height_;
		ExportUtils::GetTexturePixel(tex_info,
									pixel_data,
									current_mat.has_color_,
									current_mat.color_.red,
									current_mat.color_.green,
									current_mat.color_.blue
									);
	}

	return true;
}


EXPORT bool GetMaterialIDPerFace(CXmlExporter *exporter,
	int group_id,
	int front_material_id_per_face[],
	int back_material_id_per_face[])
{
	const XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;

	GetFaceMaterialData(exporter,
						front_material_id_per_face,
						back_material_id_per_face,
						current_entities);
	return true;
}