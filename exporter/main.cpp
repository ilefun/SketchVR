#include "./exportskpdata.h"
#include "../common/stringutils.h"

#include <ctime>



void face_data(CXmlExporter *exporter, int group_id) {
    int vertex_num=0;
    int face_num=0;

    GetFaceDSize(exporter,group_id,&vertex_num,&face_num);
    
    float *vertices=new float[vertex_num*3];
    int *vertex_num_per_face=new int[face_num];
    float *face_normal=new float[face_num*3];

    GetFace(exporter,
		group_id,
		vertices,
		vertex_num_per_face,
		face_normal);

    #ifdef PRINT_SKP_DATA
	cout << endl<<"Debug face data print------------------" << endl;
	cout << "Group id is : " << group_id << endl;
	cout << "Face num is : " << face_num<< endl;
	if (face_num > 0) {
		cout << "Vertex num is : " << vertex_num << endl;
		for (int i = 0; i < vertex_num * 3; i += 3)
			cout << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << endl;

		cout << "Vertex num per face : " << endl;
		for (int i = 0; i < face_num; i++)
			cout << vertex_num_per_face [i] << " ";

		cout << endl << "Face normal : " << endl;
		for (int i = 0; i < face_num * 3; i += 3)
			cout << face_normal[i] << " " << face_normal[i + 1] << " " << face_normal[i + 2] << endl;
	}
	cout <<endl<< "Debug data print ends------------------" << endl;

#endif // PRINT_SKP_DATA
	delete vertices;
	delete vertex_num_per_face;
	delete face_normal;

}

void face_uv(CXmlExporter *exporter, bool front_or_back, int group_id) {
	int uv_num=GetFaceUVDSize(exporter,group_id,front_or_back);

	float *u=new float[uv_num];
	float *v=new float[uv_num];
	GetFaceUV(exporter,
		group_id,
		front_or_back,
		u,
		v);

	#ifdef PRINT_SKP_DATA
	cout << endl << "Debug face uv data print------------------" << endl;
	if (front_or_back)
		cout << "[Front UV]" << endl;
	else
		cout << "[Back UV]" << endl;
		cout << "UV num is : " << uv_num << endl;
	for (int j = 0; j < uv_num; j++)
		cout << u[j] << " " << v[j] << endl;

	cout << endl << "Debug face uv data print ends------------------" << endl;

	#endif // PRINT_SKP_DATA

	delete u;
	delete v;

}


void material(CXmlExporter *exporter) {

	int mat_num = GetMaterialNum(exporter);
	cout << "Material num is : " << mat_num;

	for (int i = 0; i < mat_num; ++i)
	{
		char mat_name[100];
		GetMaterialNameByID(exporter, i, mat_name);
		
		int data_size = 0;
		data_size= GetTexPixelDSize(exporter, i);

		bool has_color = false;
		float color[3];
		bool has_alpha = false;
		float alpha = 1;
		bool has_texture = false;
		float tex_sscale = 1.0;
		float tex_tscale = 1.0;

		int origin_bits_per_pixel = 0;
		int bits_per_pixel = 0;
		int width = 0;
		int height = 0;
		float *pixel_data = NULL;
		if(data_size>0)
			pixel_data=new float[data_size];

		GetMaterialData(exporter,
			i,
			&has_color,
			color,
			&has_alpha,
			&alpha,
			&has_texture,
			&tex_sscale,
			&tex_tscale,
			&origin_bits_per_pixel,
			&bits_per_pixel,
			&width,
			&height,
			pixel_data);

		#ifdef PRINT_SKP_DATA
			cout << endl << "Material " << i  << endl;
			cout << "\tName : " << StringConvertUtils::UTF8_To_string(mat_name) << endl;
			cout << "\tHas color " << has_color << endl;
			if (has_color)
			cout << "\tColor " << color[0] << " " << color[1] << " " << color[2] << endl;

			cout << "\tHas alpha " << has_alpha << endl;
			if (has_alpha)
			cout << "\tAlpha " << alpha << endl;

			cout << "\tHas texture " << has_texture << endl;
			if (has_texture)
			{
				cout << "\tS scale : " << tex_sscale << ", T scale : " << tex_tscale << endl;
				cout << "\twidth : " << width << ",  height : " << height << endl;
				cout << "\tdata size : " << data_size << ", bits_per_pixel : " << bits_per_pixel <<", origin_bits_per_pixel : "<< origin_bits_per_pixel << endl;
				cout << "\t";
				for (size_t j = 0; j < 10; j++)
				{
					cout << pixel_data[j] << " ";
				}
				cout << endl;
			}
			// cout << "\tTexture  " << texture_path << " " << *tex_sscale << " " << *tex_tscale << endl;
		#endif
		_ASSERTE(_CrtCheckMemory());
		if(data_size && pixel_data)
			delete pixel_data;
	}
}

 void face_material(CXmlExporter *exporter, int group_id) {

    int vertex_num=0;
    int face_num=0;

    GetFaceDSize(exporter,group_id,&vertex_num,&face_num);

 	int *front_material_id_per_face=new int[face_num];
 	int *back_material_id_per_face=new int[face_num];

 	GetMaterialIDPerFace(exporter, group_id,
 		front_material_id_per_face,
 		back_material_id_per_face);

 	#ifdef PRINT_SKP_DATA
	cout << endl << "Face material print starts--------------" << endl;

	cout << "Front mat id per face :" << endl;
	for (int i = 0; i < face_num; ++i)
	{
		cout << front_material_id_per_face[i] << " ";
	}

	cout << endl << "Back mat id per face :" << endl;
	for (int i = 0; i < face_num; ++i)
	{
		cout << back_material_id_per_face[i] << " ";
	}
	cout << endl << "Face material print ends--------------" << endl;
#endif
	delete front_material_id_per_face;
	delete back_material_id_per_face;
 }

void group_id_data(CXmlExporter *exporter, int group_id) {

	clock_t start, end;

  #ifdef TIME_LOGGER
    start = clock();
  #endif

	face_data(exporter, group_id);
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Get Face data in "<<(float((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  #endif




  #ifdef TIME_LOGGER
    start = clock();
  #endif

	face_uv(exporter,true, group_id);
	face_uv(exporter,false,group_id);
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Get face front uv in "<<(float((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  #endif


  #ifdef TIME_LOGGER
    start = clock();
  #endif

	face_material(exporter,group_id);
  #ifdef TIME_LOGGER
    end = clock();
    cout<<"Time Logger : Get face material in "<<(float((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  #endif

	cout << endl << endl;
}

void facing_camera_data(CXmlExporter *expoerter)
{
	int grp_size=GetFacingCameraIdSize(expoerter);
	int *id_list = new int[grp_size];
	GetFacingCameraId(expoerter, id_list);

#ifdef PRINT_SKP_DATA
	std::cout << "Facing camera grp size : " << grp_size << endl;
	cout << "Facing camera id list : ";
	for (size_t i = 0; i < grp_size; i++)
	{
		cout << id_list[i] << " ";
	}
	cout << endl;
#endif

	delete id_list;
}
int group_data(CXmlExporter *exporter )
{
	int grp_num = GetGroupNum(exporter);
	cout << "Group num is : "<< grp_num<< endl<<endl;
	for (size_t i = 0; i < grp_num; i++)
	{
		group_id_data(exporter, int(i));
	}
	
	facing_camera_data(exporter);

	return grp_num;
}

int main(int argc,char *argv[])
{
  cout<<argv[1]<<endl;

  clock_t start, end;
  srand ( time(NULL) );

  start = clock();

  CXmlExporter *exporter=NULL;
  exporter=GetExporter(StringConvertUtils::string_To_UTF8(argv[1]).c_str());
  if (exporter) {
  	  //traverse material
	  material(exporter);

	  //traverse group
	  int grp_num = group_data(exporter);
	  
	  //release memory in the end
	  ReleaseExporter(exporter);
  }
  else
	  cout << "Failed to open file "<<argv[1] << endl;
  end = clock();
  cout<<"Time Logger : Finshed in : "<<(float((end - start)) / CLOCKS_PER_SEC)<<"s"<<endl;
  //system("pause");
}
