#include "./exportskpdata.h"
#include <unordered_map>
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

EXPORT CXmlExporter* GetExporter(const char *from_file, const char *to_folder )
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
		converted = exporter->Convert(from_file, to_folder, NULL);
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
		delete exporter;
		exporter = NULL;
		cout << endl<<"Release skp exporter memory." << endl;
	}
}

EXPORT  int GetGroupNum(CXmlExporter *exporter)
{
	return exporter->GroupNum();
}

EXPORT  void GetGroupChildrenById(CXmlExporter *exporter,int index,int **children_id,VectorHandle *id_handle)
{
	auto id_list = new std::vector<int>();
	
	auto children_id_list= exporter->GroupChildrenById(index);
	for (size_t i = 0; i < children_id_list.size(); ++i)
	{
		id_list->push_back(children_id_list[i]);
	}

	*id_handle=reinterpret_cast<VectorHandle>(id_list);
	*children_id=id_list->data();
}

EXPORT  void GetGroupTransformById(CXmlExporter *exporter,int index,double transform[16])
{

	SUTransformation current_xform=exporter->GroupById(index)->transform_;
	for (int i = 0; i < 16; ++i)
		transform[i]=current_xform.values[i];
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
		
		for (size_t j = 0; j < entities->faces_[i].face_num; ++j)
		{
			normal_list->push_back(entities->faces_[i].face_normal_.x());
			normal_list->push_back(entities->faces_[i].face_normal_.y());
			normal_list->push_back(entities->faces_[i].face_normal_.z());
		}

		for (size_t j = 0; j<entities->faces_[i].vertices_.size(); j++)
		{
			vertices_list.push_back(entities->faces_[i].vertices_[j].x());
			vertices_list.push_back(entities->faces_[i].vertices_[j].y());
			vertices_list.push_back(entities->faces_[i].vertices_[j].z());
		}
	}
}



EXPORT bool GetGroupFaceDataById(CXmlExporter *exporter,
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
	cout << "Face num is : " << *face_num<< endl;

	cout <<"Vertex num is : " <<*vertex_num << endl;
	for (int i = 0; i < (*vertex_num )*3; i+=3)
		cout << *(*vertices+i) << " "<< *(*vertices + i+1)<< " "<<*(*vertices + i+2)<<endl;

	cout<<"Vertex num per face : " << endl;
	for (int i = 0; i < *face_num; i++)
		cout << *(*vertex_num_per_face+i) << " ";

	cout<<"Face normal : " << endl;
	for (int i = 0; i < (*face_num)*3 i+=3)
		cout << *(*face_normal+i) << " "<< *(*face_normal + i+1)<< " "<<*(*face_normal + i+2)<<endl;
	
	cout <<endl<< "Debug data print ends------------------" << endl;

#endif // _DEBUG


	return true;
}

EXPORT bool GetFaceUV(CXmlExporter *exporter,
	bool front_or_back,
	double **u,
	double **v,
	int *uv_num,
	int **uv_id,
	int *uv_id_num,
	VectorHandle *uv_id_handle,
	VectorHandle *u_handle,
	VectorHandle *v_handle)
{
	auto uv_id_list = new std::vector<int>();
	auto u_list = new std::vector<double>();
	auto v_list = new std::vector<double>();

	std::vector<CPoint3d> uv_collector;

	int current_id = -1;
	int current_face_id=0;
	if (front_or_back)
		for (size_t i = 0; i < exporter->skpdata_.entities_.faces_.size(); i++) {
			size_t vertices_size = exporter->skpdata_.entities_.faces_[i].vertices_.size();

			if (exporter->skpdata_.entities_.faces_[i].has_front_texture_) {
				for (size_t j = 0; j < vertices_size; j++) {
					current_id = PushVertex(&uv_collector, exporter->skpdata_.entities_.faces_[i].vertices_[j].front_texture_coord_);
					uv_id_list->push_back(current_id);
				}
			}
			else
				for (size_t j = 0; j < vertices_size; ++j)
					uv_id_list->push_back(-1);
		}
	else
		for (size_t i = 0; i < exporter->skpdata_.entities_.faces_.size(); i++) {
			size_t vertices_size = exporter->skpdata_.entities_.faces_[i].vertices_.size();

			if (exporter->skpdata_.entities_.faces_[i].has_back_texture_) {
				for (size_t j = 0; j < vertices_size; j++) {
					current_id = PushVertex(&uv_collector, exporter->skpdata_.entities_.faces_[i].vertices_[j].back_texture_coord_);
					uv_id_list->push_back(current_id);
				}
			}
			else
				for (size_t j = 0; j < vertices_size; ++j)
					uv_id_list->push_back(-1);
		}

	
	for (size_t j = 0; j < uv_collector.size(); j++)
	{
		u_list->push_back(uv_collector[j].x());
		v_list->push_back(uv_collector[j].y());
	}

	*u_handle = reinterpret_cast<VectorHandle>(u_list);
	*v_handle = reinterpret_cast<VectorHandle>(v_list);
	*u = u_list->data();
	*v = v_list->data();
	*uv_num = int(u_list->size());

	*uv_id_handle = reinterpret_cast<VectorHandle>(uv_id_list);
	*uv_id = uv_id_list->data();
	*uv_id_num= int(uv_id_list->size());

#ifdef _DEBUG
	cout << endl << "Debug face uv data print------------------" << endl;
	if (front_or_back)
		cout << "[Front UV]" << endl;
	else
		cout << "[Back UV]" << endl;
	cout << "UV num is : " << *uv_num << endl;
	for (int j = 0; j < *uv_num; j++) 
		cout << *(*u + j) << " " << *(*v + j) << endl;
	
	cout <<endl<< "UV id num : "<< *uv_id_num << endl;
	cout << "UV id list per face-vertex : " << endl;
	for (int j = 0; j < *uv_id_num; j++)
		cout << *(*uv_id + j) << " ";

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
							char  *texture_path,
                            double *tex_sscale,
                            double *tex_tscale )
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
		strcpy_s(texture_path,current_mat.texture_path_.length(),current_mat.texture_path_.c_str());
		*tex_sscale=current_mat.texture_sscale_;
		*tex_tscale=current_mat.texture_tscale_;
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
		cout << "\tTexture  " << texture_path << " " << *tex_sscale << " " << *tex_tscale << endl;
	cout << endl << "Material " << material_id << " Data print ends" << endl;
#endif
	return true;
}

EXPORT bool GetMaterialIDPerFace(CXmlExporter *exporter,
	int **front_material_id_per_face,
	int **back_material_id_per_face,
	int *face_num,
	VectorHandle *front_mat_handle,
	VectorHandle *back_mat_handle)
{
	std::unordered_map<std::string,int> name_id_map;

	for (size_t i = 0; i<exporter->skpdata_.materials_.size(); i++)
		name_id_map[exporter->skpdata_.materials_[i].name_]=int(i);

	auto front_mat_id=new std::vector<int>;
	auto back_mat_id=new std::vector<int>;

	for (size_t i = 0; i<exporter->skpdata_.entities_.faces_.size(); i++)
	{
		int current_face_num=1;
		if (!exporter->skpdata_.entities_.faces_[i].has_single_loop_)
			current_face_num=exporter->skpdata_.entities_.faces_[i].vertices_.size() / 3;

		if(name_id_map.count(exporter->skpdata_.entities_.faces_[i].front_mat_name_))
			for(int j=0;j<current_face_num;j++)
				front_mat_id->push_back(name_id_map[exporter->skpdata_.entities_.faces_[i].front_mat_name_]);
		else
			for(int j=0;j<current_face_num;j++)
				front_mat_id->push_back(-1);

		if(name_id_map.count(exporter->skpdata_.entities_.faces_[i].back_mat_name_))
			for(int j=0;j<current_face_num;j++)
				back_mat_id->push_back(name_id_map[exporter->skpdata_.entities_.faces_[i].back_mat_name_]);
		else
			for(int j=0;j<current_face_num;j++)
				back_mat_id->push_back(-1);

	}


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