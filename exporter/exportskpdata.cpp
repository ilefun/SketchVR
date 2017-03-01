#include "./exportskpdata.h"
#include <assert.h>
#include <SketchUpAPI/transformation.h>

using namespace std;

void GetFaceData(int v_per_face_list[],
					double vertices_list[],
                    double normal_list[],
					const XmlEntitiesInfo *entities)
{
	for (size_t i = 0; i<entities->faces_.size(); i++)
	{
		if (entities->faces_[i].has_single_loop_)
			v_per_face_list[0]=entities->faces_[i].vertices_.size();
		else
			for (size_t j = 0; j<entities->faces_[i].vertices_.size() / 3; j++)
				v_per_face_list[j]=3;
		
		for (size_t j = 0; j < entities->faces_[i].face_num_;j++)
		{
			normal_list[j*3]=entities->faces_[i].face_normal_.x();
			normal_list[j*3+1]=entities->faces_[i].face_normal_.y();
			normal_list[j*3+2]=entities->faces_[i].face_normal_.z();
		}

		for (size_t j = 0; j<entities->faces_[i].vertices_.size();j++)
		{
			vertices_list[j*3]=entities->faces_[i].vertices_[j].vertex_.x();
			vertices_list[j*3+1]=entities->faces_[i].vertices_[j].vertex_.y();
			vertices_list[j*3+2]=entities->faces_[i].vertices_[j].vertex_.z();
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
				double u_list[],
				double v_list[],
				const XmlEntitiesInfo *entities)
{
	if (front_or_back)
		for (size_t i = 0; i < entities->faces_.size(); i++) {
			size_t vertices_size = entities->faces_[i].vertices_.size();
			if (entities->faces_[i].has_front_texture_) {
				for (size_t j = 0; j < vertices_size; j++) {
					u_list[j]=entities->faces_[i].vertices_[j].front_texture_coord_.x();
					v_list[j]=entities->faces_[i].vertices_[j].front_texture_coord_.y();
				}
			}
			else
				for (size_t j = 0; j < vertices_size; ++j) {
					u_list[j]=-100;
					v_list[j]=-100;
				}
		}
	else
		for (size_t i = 0; i < entities->faces_.size(); i++) {
			size_t vertices_size = entities->faces_[i].vertices_.size();

			if (entities->faces_[i].has_back_texture_) {
				for (size_t j = 0; j < vertices_size; j++) {
					u_list[j]=entities->faces_[i].vertices_[j].back_texture_coord_.x();
					v_list[j]=entities->faces_[i].vertices_[j].back_texture_coord_.y();
				}
			}
			else
				for (size_t j = 0; j < vertices_size; ++j) {
					u_list[j]=-100;
					v_list[j]=-100;
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

EXPORT  void GetGroupTransformById(CXmlExporter *exporter,int group_id,double transform[16])
{
	if (group_id >= -1 && group_id<exporter->GroupNum())
		if (group_id ==-1) {
			double default_xform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
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
			v_num+= entities->faces_[i].vertices_.size();
		}
		else{
			f_num+=entities->faces_[i].vertices_.size() / 3;
			v_num+= entities->faces_[i].vertices_.size();
		}
	}
}

EXPORT bool GetFace(CXmlExporter *exporter,
                        int group_id,
						double vertices[],
						int vertex_num_per_face[],
						double face_normal[])
{
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;
	GetFaceData(vertex_num_per_face,
					vertices,
					face_normal,
					current_entities);

#ifdef PRINT_SKP_DATA
	cout << endl<<"Debug face data print------------------" << endl;
	cout << "Group id is : " << group_id << endl;
	cout << "Face num is : " << *face_num<< endl;
	if (*face_num > 0) {
		cout << "Vertex num is : " << *vertex_num << endl;
		for (int i = 0; i < (*vertex_num) * 3; i += 3)
			cout << *(*vertices + i) << " " << *(*vertices + i + 1) << " " << *(*vertices + i + 2) << endl;

		cout << "Vertex num per face : " << endl;
		for (int i = 0; i < *face_num; i++)
			cout << *(*vertex_num_per_face + i) << " ";

		cout << endl << "Face normal : " << endl;
		for (int i = 0; i < (*face_num) * 3; i += 3)
			cout << *(*face_normal + i) << " " << *(*face_normal + i + 1) << " " << *(*face_normal + i + 2) << endl;
	}
	cout <<endl<< "Debug data print ends------------------" << endl;

#endif // PRINT_SKP_DATA


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
						double u[],
						double v[])
{
	XmlEntitiesInfo *current_entities=GetEntitiesInfo(exporter,group_id);
	if (current_entities==NULL)
		return false;
	GetUVData(front_or_back,u,v,current_entities);

#ifdef PRINT_SKP_DATA
	cout << endl << "Debug face uv data print------------------" << endl;
	if (front_or_back)
		cout << "[Front UV]" << endl;
	else
		cout << "[Back UV]" << endl;
	cout << "UV num is : " << *uv_num << endl;
	for (int j = 0; j < *uv_num; j++) 
		cout << *(*u + j) << " " << *(*v + j) << endl;

	cout << endl << "Debug face uv data print ends------------------" << endl;

#endif // PRINT_SKP_DATA
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
                            double color[3],

                            bool *has_alpha,
                            double *alpha,

                            bool *has_texture,
                            double *tex_sscale,
                            double *tex_tscale,
                            int *bits_per_pixel,
                            int *width,
                            int *height,
                            double pixel_data[] )
{
	XmlMaterialInfo current_mat=exporter->skpdata_.materials_.at(material_id);
	
	*has_color=current_mat.has_color_;
	if(*has_color){
		color[0]=double(current_mat.color_.red)/255.0;
		color[1]=double(current_mat.color_.green)/255.0;
		color[2]=double(current_mat.color_.blue)/255.0;
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
			pixel_data[i] = double(current_mat.pixel_data_[i]) / 255.0;
		}
	}
#ifdef PRINT_SKP_DATA
	cout << endl << "Material "<< material_id <<" Data print starts" << endl;
	cout << "\tHas color " << *has_color << endl;
	if (*has_color)
		cout << "\tColor " << color[0] << " " << color[1] << " " << color[2] << endl;

	cout << "\tHas alpha " << *has_alpha << endl;
	if (*has_alpha)
		cout << "\tAlpha " << *alpha << endl;

	cout << "\tHas texture " << *has_texture << endl;
	if (*has_texture)
	{
		cout<<"\twidth : "<<*width<<",  height : "<<*height<<endl;
		cout<<"\tdata size : "<<current_mat.data_size_<<", bits_per_pixel : "<<*bits_per_pixel <<endl;
		for (size_t i = 0; i < 10; i++)
		{
			cout << *(*pixel_data + i) << " ";
		}
		cout << endl;
	}
		// cout << "\tTexture  " << texture_path << " " << *tex_sscale << " " << *tex_tscale << endl;
	cout << endl << "Material " << material_id << " Data print ends" << endl;
#endif
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

// #ifdef PRINT_SKP_DATA
// 	cout << endl << "Face material print starts--------------" << endl;

// 	cout << "Front mat id per face :" << endl;
// 	for (int i = 0; i < *face_num; ++i)
// 	{
// 		cout << *(*front_material_id_per_face + i) << " ";
// 	}

// 	cout << endl << "Back mat id per face :" << endl;
// 	for (int i = 0; i < *face_num; ++i)
// 	{
// 		cout << *(*back_material_id_per_face + i) << " ";
// 	}
// 	cout << endl << "Face material print ends--------------" << endl;
// #endif

	return true;
}