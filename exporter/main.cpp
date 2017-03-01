#include "./exportskpdata.h"
#include <ctime>

void face_data(CXmlExporter *exporter, int group_id) {
    int vertex_num;
    int face_num;

    GetFaceDSize(exporter,group_id,&vertex_num,&face_num);
    
    double *vertices=new double[vertex_num*3];
    int *vertex_num_per_face=new int[face_num];
    double *face_normal=new double[face_num*3];

    GetFace(exporter,
		group_id,
		vertices,
		vertex_num_per_face,
		face_normal);

}

void face_uv(CXmlExporter *exporter, bool front_or_back, int group_id) {
	double *u;
	double *v;
	int uv_num;

	VectorHandle u_handle;
	VectorHandle v_handle;

	GetFaceUV(exporter,
		group_id,
		front_or_back,
		&u,
		&v,
		&uv_num,
		&u_handle, &v_handle);

}

void material(CXmlExporter *exporter) {

	int mat_num = GetMaterialNum(exporter);
	cout << "Material num is : " << mat_num;

	for (int i = 0; i < mat_num; ++i)
	{
		char mat_name[100];
		GetMaterialNameByID(exporter, i, mat_name);
		cout << endl << "Material id&name : " << i << " " << mat_name << endl;

		bool has_color = false;
		double color[3];
		bool has_alpha = false;
		double alpha = 1;
		bool has_texture = false;
		double tex_sscale = 1.0;
		double tex_tscale = 1.0;

		int data_size_per_pixel = 0;
		int data_size = 0;
		int width = 0;
		int height = 0;
		double *pixel_data = NULL;
		VectorHandle pixel_data_handle;

		GetMaterialData(exporter,
			i,
			&has_color,
			color,
			&has_alpha,
			&alpha,
			&has_texture,
			&tex_sscale,
			&tex_tscale,
			&data_size_per_pixel,
			&data_size,
			&width,
			&height,
			&pixel_data,
			&pixel_data_handle);

	}
}

void face_material(CXmlExporter *exporter, int group_id) {

	int *front_material_id_per_face;
	int *back_material_id_per_face;
	VectorHandle front_mat_handle;
	VectorHandle back_mat_handle;
	int face_num;

	GetMaterialIDPerFace(exporter, group_id,
		&front_material_id_per_face,
		&back_material_id_per_face,
		&face_num,
		&front_mat_handle,
		&back_mat_handle);

}

void group_id_data(CXmlExporter *exporter, int group_id) {
	double xform[16];
	GetGroupTransformById(exporter, group_id, xform);

	cout << "Group xform is : ";
	for (size_t j = 0; j < 16; j++)
	{
		cout << xform[j] << " ";
	}
	cout << endl;
	clock_t start, end;

  #ifdef TIME_LOGGER
    start = clock();
  #endif

	face_data(exporter, group_id);
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Get Face data in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  #endif




  #ifdef TIME_LOGGER
    start = clock();
  #endif

	face_uv(exporter,true, group_id);
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Get face front uv in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  #endif

	face_uv(exporter,false,group_id);

  #ifdef TIME_LOGGER
    start = clock();
  #endif

	face_material(exporter,group_id);
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Get face material in "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  #endif

	int* children_id_list;
	int children_num;
	VectorHandle children_id_handle;;

	GetGroupChildrenById(exporter, group_id, &children_id_list, &children_num, &children_id_handle);
	cout << "Children num is : " << children_num << endl;
	if (children_num > 0) {
		for (size_t k = 0; k < children_num; k++)
		{
			cout <<"Group id:" <<*(children_id_list + k) << " Parent id: "<<group_id<<"-------------------------" << endl;
			group_id_data(exporter, *(children_id_list + k));
		}
	}
	cout << endl << endl;
}

int group_data(CXmlExporter *exporter )
{
	int grp_num = GetGroupNum(exporter);
	cout << "Group num is : "<< grp_num<< endl<<endl;
	cout << endl << "Root group id : " << -1 << endl;
	group_id_data(exporter, -1);
	return grp_num;
}

int main(int argc,char *argv[])
{
  cout<<argv[1]<<endl;

  clock_t start, end;
  srand ( time(NULL) );

  start = clock();

  CXmlExporter *exporter=NULL;
  exporter=GetExporter(argv[1]);
  if (exporter){
	  //exporter->skpdata_.debug_print();
	  material(exporter);
	  int grp_num=group_data(exporter);
	  
	  ReleaseExporter(exporter);
  }
  end = clock();
  cout<<"Time Logger : Finshed in : "<<(double((end - start)) / CLOCKS_PER_SEC)<<"s";
  //system("pause");
}
