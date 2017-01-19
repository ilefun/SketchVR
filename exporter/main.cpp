#include "./exportskpdata.h"

void group_data(CXmlExporter *exporter )
{
	int grp_num = GetGroupNum(exporter);
	cout << "Group num is : "<< grp_num<< endl;
	for (size_t i = 0; i < grp_num; i++)
	{
		double xform[16];
		cout << "Group index : " << i<<endl;
		GetGroupTransformById(exporter, i, xform);

		cout << "Group xform is : ";
		for (size_t i = 0; i < 16; i++)
		{
			cout << xform[i] << " ";
		}
		cout << endl;
	}
}
void face_data(CXmlExporter *exporter,int group_id) {
	double *vertices;
	int vertex_num;
	int *vertex_num_per_face;
	int face_num;
	int *face_vertex_index;
	VectorHandle vertices_handle;
	VectorHandle vertices_face_handle;
	VectorHandle face_vindex_handle;

	GetGroupFaceDataById(exporter, group_id, &vertices, &vertex_num, &vertex_num_per_face, &face_num, &face_vertex_index, &vertices_handle, &vertices_face_handle, &face_vindex_handle);

}

void face_uv(CXmlExporter *exporter,bool front_or_back) {
	double *u;
	double *v;
	int uv_num;
	int *uv_id;
	int uv_id_num;
	VectorHandle uv_id_handle;
	VectorHandle u_handle;
	VectorHandle v_handle;

	GetFaceUV(exporter,front_or_back,&u,&v, &uv_num,&uv_id, &uv_id_num, &uv_id_handle,&u_handle,&v_handle);

}

void material(CXmlExporter *exporter){
	
	int mat_num= GetMaterialNum(exporter);
	cout<<"Material num is : "<<mat_num;

	for (int i = 0; i < mat_num; ++i)
	{
		char mat_name[100];
		GetMaterialNameByID(exporter, i, mat_name);
		cout<<endl<<"Material id&name : "<<i<<" "<<mat_name<<endl;

		bool has_color=false;
        double color[3];
        bool has_alpha=false;
        double alpha=1;
        bool has_texture=false;
        char texture_path[100];
        double tex_sscale=1.0;
        double tex_tscale=1.0;	

		GetMaterialData(exporter,  i, &has_color, color, &has_alpha, &alpha, &has_texture, texture_path, &tex_sscale, &tex_tscale);

	}
}

void face_material(CXmlExporter *exporter){

	int *front_material_id_per_face;
	int *back_material_id_per_face;
	VectorHandle front_mat_handle;
	VectorHandle back_mat_handle;
	int face_num;

	GetMaterialIDPerFace(exporter,
						&front_material_id_per_face,
						&back_material_id_per_face,
						&face_num,
						&front_mat_handle,
						&back_mat_handle);

}

int main(int argc,char *argv[])
{
  cout<<argv[1]<<" -> "<<argv[2]<<endl;


  CXmlExporter *exporter=NULL;
  exporter=GetExporter(argv[1], argv[2]);
  if (exporter){
	  exporter->skpdata_.debug_print();
	  group_data(exporter);

	  //face_data(exporter);

	  //face_uv(exporter,true);
	  //face_uv(exporter,false);

	  //material(exporter);
	  //face_material(exporter);
	  
	  ReleaseExporter(exporter);
  }
  system("pause");
}
