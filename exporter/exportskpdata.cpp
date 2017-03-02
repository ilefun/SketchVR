#include "./exportskpdata.h"
#include <assert.h>
#include <SketchUpAPI/transformation.h>

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

XmlEntitiesInfo* GetEntitiesInfo(CXmlExporter *exporter, int group_id)
{
	XmlEntitiesInfo *current_entities;
	if (group_id >= -1 && group_id<exporter->GroupNum())

		if (group_id==-1)
			return &exporter->skpdata_.entities_;
		else
			return exporter->GroupById(group_id)->entities_;
	else
	{
		return NULL;
	}
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

		if(exporter->matname_id_map_.count(entities->faces_[i].front_mat_name_))
			for(int j=0;j<current_face_num;j++)
				front_id[front_index++]=exporter->matname_id_map_[entities->faces_[i].front_mat_name_];
		else
			for(int j=0;j<current_face_num;j++)
				front_id[front_index++]=-1;

		if(exporter->matname_id_map_.count(entities->faces_[i].back_mat_name_))
			for(int j=0;j<current_face_num;j++)
				back_id[back_index++]=exporter->matname_id_map_[entities->faces_[i].back_mat_name_];
		else
			for(int j=0;j<current_face_num;j++)
				back_id[back_index++]=-1;

	}
}

EXPORT CXmlExporter* GetExporter(const char *from_file)
{
	bool m_bExportMaterials = true;
	bool m_bExportFaces = true;
	bool m_bExportEdges = true;
	bool m_bExportMaterialsByLayer = false;
	bool m_bExportLayers = true;
	bool m_bExportOptions = true;
	bool m_bExportSelectionSet = false;
	bool m_bTriangleFace = true;

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
		exporter->SetOptions(options);

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
		//release pixel data memory
		for (size_t i = 0; i < exporter->skpdata_.materials_.size(); ++i)
			if(exporter->skpdata_.materials_[i].pixel_data_){
			    delete exporter->skpdata_.materials_[i].pixel_data_;
			    exporter->skpdata_.materials_[i].pixel_data_=NULL;
			}

		delete exporter;
		exporter = NULL;
		cout << endl<<"Release skp exporter memory." << endl;
	}
}

EXPORT  int GetGroupNum(CXmlExporter *exporter)
{
	return exporter->GroupNum();
}

EXPORT  int GetGroupChildrenNum(CXmlExporter *exporter, int group_id )
{
	if (group_id >= -1 && group_id<exporter->GroupNum())
		if (group_id == -1)
			return exporter->RootGroupChildren().size();
		else
			return exporter->GroupChildrenById(group_id).size();
	return 0;
}

EXPORT  void GetGroupChildrenById(CXmlExporter *exporter,int group_id,int children_id[])
{
	std::vector<int> children_id_list;
	if (group_id >= -1 && group_id<exporter->GroupNum())
		if (group_id == -1)
			children_id_list = exporter->RootGroupChildren();
		else
			children_id_list = exporter->GroupChildrenById(group_id);

		for (size_t i = 0; i < children_id_list.size(); ++i)
			children_id[i]=children_id_list[i];
}

EXPORT  void GetGroupTransformById(CXmlExporter *exporter,int group_id,float transform[16])
{
	if (group_id >= -1 && group_id<exporter->GroupNum())
		if (group_id ==-1) {
			float default_xform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			for (size_t i = 0; i < 16; i++)
				transform[i] = default_xform[i];
		}
		else {
			SUTransformation current_xform = exporter->GroupById(group_id)->transform_;
			for (int i = 0; i < 16; ++i)
				transform[i] = current_xform.values[i];
		}
}

EXPORT void GetFaceDSize(CXmlExporter *exporter,
                        int group_id,
                        int *vertex_num,
                        int *face_num )
{
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return ;

	int v_num=0,f_num=0;
	for (size_t i = 0; i<current_entities->faces_.size(); i++)
	{
		if (current_entities->faces_[i].has_single_loop_)
		{
			f_num++;
			v_num+= current_entities->faces_[i].vertices_.size();
		}
		else{
			f_num+= current_entities->faces_[i].face_num_;
			v_num+= current_entities->faces_[i].face_num_*3;
		}
	}
	*vertex_num = v_num;
	*face_num = f_num;
}

EXPORT bool GetFace(CXmlExporter *exporter,
                        int group_id,
						float vertices[],
						int vertex_num_per_face[],
						float face_normal[])
{
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;
	GetFaceData(vertex_num_per_face,
					vertices,
					face_normal,
					current_entities);
	return true;
}


EXPORT int GetFaceUVDSize(CXmlExporter *exporter,
                        int group_id,
                        bool front_or_back )
{
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
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
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
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
	strcpy_s(mat_name, exporter->skpdata_.materials_[id].name_.length(), exporter->skpdata_.materials_[id].name_.c_str());
	return true;
}

EXPORT int GetTexPixelDSize(CXmlExporter *exporter, 
                            int material_id )
{
	if(exporter->skpdata_.materials_.at(material_id).has_texture_)
		return exporter->skpdata_.materials_[material_id].data_size_;
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
		*tex_sscale=current_mat.texture_sscale_;
		*tex_tscale=current_mat.texture_tscale_;
		*bits_per_pixel=current_mat.bits_per_pixel_;
		*width=current_mat.width_;
		*height=current_mat.height_;

		for (size_t i = 0; i < current_mat.data_size_; ++i) {
			pixel_data[i] = float(current_mat.pixel_data_[i]) / 255.0f;
		}
	}

	return true;
}


EXPORT bool GetMaterialIDPerFace(CXmlExporter *exporter,
	int group_id,
	int front_material_id_per_face[],
	int back_material_id_per_face[])
{
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;

	GetFaceMaterialData(exporter,
						front_material_id_per_face,
						back_material_id_per_face,
						current_entities);
	return true;
}