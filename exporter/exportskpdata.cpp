#include "./exportskpdata.h"
#include <assert.h>
#include <SketchUpAPI/transformation.h>

using namespace std;

typedef intptr_t VectorHandle;

int PushVertex(std::vector<CPoint3d> *collector,const CPoint3d &pt)
{
	int nearest_index = -1;
	for(size_t i = 0;i<collector->size();i++)
		if ((*collector)[i].near_to(pt)) {
			nearest_index = i;
			break;
		}

	if (nearest_index == -1)
	{
		collector->push_back(pt);
		return int(collector->size()) - 1;
	}
	else
		return nearest_index;
}	

void GetFaceData(std::vector<int> *v_per_face_list,
					std::vector<int> *face_vertex_list,
					std::vector<double> *vertices_list,
                    std::vector<double> *normal_list,
					const XmlEntitiesInfo *entities)
{
	for (size_t i = 0; i<entities->faces_.size(); i++)
	{
		if (entities->faces_[i].has_single_loop_)
			v_per_face_list->push_back(entities->faces_[i].vertices_.size());
		else
			for (size_t j = 0; j<entities->faces_[i].vertices_.size() / 3; j++)
				v_per_face_list->push_back(3);
		
		for (size_t j = 0; j < entities->faces_[i].face_num_; ++j)
		{
			normal_list->push_back(entities->faces_[i].face_normal_.x());
			normal_list->push_back(entities->faces_[i].face_normal_.y());
			normal_list->push_back(entities->faces_[i].face_normal_.z());
		}

		for (size_t j = 0; j<entities->faces_[i].vertices_.size(); j++)
		{
			vertices_list->push_back(entities->faces_[i].vertices_[j].vertex_.x());
			vertices_list->push_back(entities->faces_[i].vertices_[j].vertex_.y());
			vertices_list->push_back(entities->faces_[i].vertices_[j].vertex_.z());
		}
	}
}

void GetUVData(bool front_or_back,
				std::vector<double> *u_list,
				std::vector<double> *v_list,
				const XmlEntitiesInfo *entities)
{
	if (front_or_back)
		for (size_t i = 0; i < entities->faces_.size(); i++) {
			size_t vertices_size = entities->faces_[i].vertices_.size();
			if (entities->faces_[i].has_front_texture_) {
				for (size_t j = 0; j < vertices_size; j++) {
					u_list->push_back(entities->faces_[i].vertices_[j].front_texture_coord_.x());
					v_list->push_back(entities->faces_[i].vertices_[j].front_texture_coord_.y());
				}
			}
			else
				for (size_t j = 0; j < vertices_size; ++j) {
					u_list->push_back(-100);
					v_list->push_back(-100);
				}
		}
	else
		for (size_t i = 0; i < entities->faces_.size(); i++) {
			size_t vertices_size = entities->faces_[i].vertices_.size();

			if (entities->faces_[i].has_back_texture_) {
				for (size_t j = 0; j < vertices_size; j++) {
					u_list->push_back(entities->faces_[i].vertices_[j].back_texture_coord_.x());
					v_list->push_back(entities->faces_[i].vertices_[j].back_texture_coord_.y());
				}
			}
			else
				for (size_t j = 0; j < vertices_size; ++j) {
					u_list->push_back(-100);
					v_list->push_back(-100);
				}
		}
}

void GetFaceMaterialData(CXmlExporter *exporter,std::vector<int> *front_id,std::vector<int> *back_id,const XmlEntitiesInfo *entities)
{
	for (size_t i = 0; i<entities->faces_.size(); i++)
	{
		int current_face_num=1;
		if (!entities->faces_[i].has_single_loop_)
			current_face_num=entities->faces_[i].vertices_.size() / 3;

		if(exporter->matname_id_map_.count(entities->faces_[i].front_mat_name_))
			for(int j=0;j<current_face_num;j++)
				front_id->push_back(exporter->matname_id_map_[entities->faces_[i].front_mat_name_]);
		else
			for(int j=0;j<current_face_num;j++)
				front_id->push_back(-1);

		if(exporter->matname_id_map_.count(entities->faces_[i].back_mat_name_))
			for(int j=0;j<current_face_num;j++)
				back_id->push_back(exporter->matname_id_map_[entities->faces_[i].back_mat_name_]);
		else
			for(int j=0;j<current_face_num;j++)
				back_id->push_back(-1);

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
		for (size_t i = 0; i < exporter->materials_.size(); ++i)
			if(exporter->materials_[i].pixel_data_){
			    delete exporter->materials_[i].pixel_data_;
			    exporter->materials_[i].pixel_data_=NULL;
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

EXPORT  void GetGroupChildrenById(CXmlExporter *exporter,int group_id,int **children_id, int *children_num,VectorHandle *id_handle)
{
	auto id_list = new std::vector<int>();
	
	auto children_id_list= exporter->GroupChildrenById(group_id);
	for (size_t i = 0; i < children_id_list.size(); ++i)
	{
		id_list->push_back(children_id_list[i]);
	}

	*id_handle=reinterpret_cast<VectorHandle>(id_list);
	*children_id=id_list->data();
	*children_num=int(id_list->size());
}

EXPORT  void GetGroupTransformById(CXmlExporter *exporter,int group_id,double transform[16])
{

	SUTransformation current_xform=exporter->GroupById(group_id)->transform_;
	for (int i = 0; i < 16; ++i)
		transform[i]=current_xform.values[i];
}

EXPORT bool GetFace(CXmlExporter *exporter,
                        int group_id,
						double **vertices,
						int *vertex_num,
						int **vertex_num_per_face,
						int *face_num,
						double **face_normal,
						VectorHandle *vertices_handle,
						VectorHandle *vertices_face_handle,
						VectorHandle *face_normal_handle)
{
	auto vertices_list = new std::vector<double>();
	auto normal_list = new std::vector<double>();
	auto v_per_face_list = new std::vector<int>();
	auto face_vertex_list = new std::vector<int>();

	XmlEntitiesInfo *current_entities;
	if (group_id<0)
		current_entities=&exporter->skpdata_.entities_;
	else
		current_entities=exporter->GroupById(group_id)->entities_;


	GetFaceData(v_per_face_list,
					face_vertex_list,
					vertices_list,
					normal_list,
					current_entities);

	*vertices_handle = reinterpret_cast<VectorHandle>(vertices_list);
	*vertices = vertices_list->data();
	*vertex_num = int(vertices_list->size() / 3);

	*face_normal_handle = reinterpret_cast<VectorHandle>(normal_list);
	*face_normal = normal_list->data();

	*vertices_face_handle = reinterpret_cast<VectorHandle>(v_per_face_list);
	*vertex_num_per_face = v_per_face_list->data();
	*face_num = int(v_per_face_list->size());


#ifdef _DEBUG
	cout << endl<<"Debug face data print------------------" << endl;
	cout << "Group id is : " << group_id << endl;
	cout << "Face num is : " << *face_num<< endl;

	cout <<"Vertex num is : " <<*vertex_num << endl;
	for (int i = 0; i < (*vertex_num )*3; i+=3)
		cout << *(*vertices+i) << " "<< *(*vertices + i+1)<< " "<<*(*vertices + i+2)<<endl;

	cout<<"Vertex num per face : " << endl;
	for (int i = 0; i < *face_num; i++)
		cout << *(*vertex_num_per_face+i) << " ";

	cout<<endl<<"Face normal : " << endl;
	for (int i = 0; i < (*face_num)*3 ;i+=3)
		cout << *(*face_normal+i) << " "<< *(*face_normal + i+1)<< " "<<*(*face_normal + i+2)<<endl;
	
	cout <<endl<< "Debug data print ends------------------" << endl;

#endif // _DEBUG


	return true;
}

EXPORT bool GetFaceUV(CXmlExporter *exporter,
						int group_id,
						bool front_or_back,
						double **u,
						double **v,
						int *uv_num,
						VectorHandle *u_handle,
						VectorHandle *v_handle)
{
	auto u_list = new std::vector<double>();
	auto v_list = new std::vector<double>();

	XmlEntitiesInfo *current_entities;
	if (group_id<0)
		current_entities=&exporter->skpdata_.entities_;
	else
		current_entities=exporter->GroupById(group_id)->entities_;

	GetUVData(front_or_back,u_list,v_list,current_entities);

	*u_handle = reinterpret_cast<VectorHandle>(u_list);
	*v_handle = reinterpret_cast<VectorHandle>(v_list);
	*u = u_list->data();
	*v = v_list->data();
	*uv_num = int(u_list->size());

#ifdef _DEBUG
	cout << endl << "Debug face uv data print------------------" << endl;
	if (front_or_back)
		cout << "[Front UV]" << endl;
	else
		cout << "[Back UV]" << endl;
	cout << "UV num is : " << *uv_num << endl;
	for (int j = 0; j < *uv_num; j++) 
		cout << *(*u + j) << " " << *(*v + j) << endl;

	cout << endl << "Debug face uv data print ends------------------" << endl;

#endif // _DEBUG
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
                            int *data_size,
                            int *width,
                            int *height,
                            double **pixel_data,
                            VectorHandle *pixel_data_handle )
{
	size_t mat_num= exporter->skpdata_.materials_.size();
	if(material_id>=mat_num) return false;
	
	XmlMaterialInfo current_mat=exporter->skpdata_.materials_[material_id];
	
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
		*data_size=current_mat.data_size_;
		*width=current_mat.width_;
		*height=current_mat.height_;

		auto pixel_data_list = new std::vector<double>(*data_size);
		for (size_t i = 0; i < pixel_data_list->size(); ++i)
			(*pixel_data_list)[i]=double(current_mat.pixel_data_[i])/255.0;
		*pixel_data=pixel_data_list->data();
		}
#ifdef _DEBUG
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
		cout<<"\tdata size : "<<*data_size<<", bits_per_pixel : "<<*bits_per_pixel <<endl;
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
	int **front_material_id_per_face,
	int **back_material_id_per_face,
	int *face_num,
	VectorHandle *front_mat_handle,
	VectorHandle *back_mat_handle)
{
	auto front_mat_id=new std::vector<int>;
	auto back_mat_id=new std::vector<int>;

	XmlEntitiesInfo *current_entities;
	if (group_id<0)
		current_entities=&exporter->skpdata_.entities_;
	else
		current_entities=exporter->GroupById(group_id)->entities_;

	GetFaceMaterialData(exporter,front_mat_id,back_mat_id,current_entities);

	*face_num=front_mat_id->size();
	*front_material_id_per_face=front_mat_id->data();
	*back_material_id_per_face=back_mat_id->data();

	*front_mat_handle = reinterpret_cast<VectorHandle>(front_mat_id);
	*back_mat_handle = reinterpret_cast<VectorHandle>(back_mat_id);

#ifdef _DEBUG
	cout << endl << "Face material print starts--------------" << endl;

	cout << "Face num is : " << *face_num << endl;
	cout << "Front mat id per face :" << endl;
	for (int i = 0; i < *face_num; ++i)
	{
		cout << *(*front_material_id_per_face + i) << " ";
	}

	cout << endl << "Back mat id per face :" << endl;
	for (int i = 0; i < *face_num; ++i)
	{
		cout << *(*back_material_id_per_face + i) << " ";
	}
	cout << endl << "Face material print ends--------------" << endl;
#endif

	return true;
}

EXPORT bool ReleaseDoubleHandle(VectorHandle handle)
{
	auto h = reinterpret_cast<std::vector<double>*>(handle);
	delete h;
	return true;
}

EXPORT bool ReleaseIntHandle(VectorHandle handle)
{
	auto h = reinterpret_cast<std::vector<int>*>(handle);
	delete h;
	return true;
}

EXPORT bool ReleaseBoolHandle(VectorHandle handle)
{
	auto h = reinterpret_cast<std::vector<bool>*>(handle);
	delete h;
	return true;
}