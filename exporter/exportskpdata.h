#ifndef SKPDATA_EXPORT
#define SKPDATA_EXPORT

#include <iostream>

#include "./xmlexporter.h"
#include "../common/xmlgeomutils.h"

using namespace XmlGeomUtils;

using namespace std;
typedef intptr_t VectorHandle;
#define EXPORT extern "C" __declspec(dllexport)


//merge the nearest vertex,return the vertex id
int PushVertex(std::vector<CPoint3d> *collector, const CPoint3d *pt, CPoint3d &out_pt);

void GetFaceData(std::vector<int> *v_per_face_list,
                    std::vector<int> *face_vertex_list,
                    std::vector<CPoint3d> *vertices_collector,
                    const XmlEntitiesInfo *entities);

EXPORT  int GetGroupNum(CXmlExporter *exporter);
  
EXPORT  void GetGroupTransformById(CXmlExporter *exporter,int index,double transform[16]);

EXPORT  void GetGroupChildrenById(CXmlExporter *exporter, int index, int **children_id, VectorHandle *id_handle);

//get exporter instance for specified skp file
EXPORT CXmlExporter* GetExporter(const char *from_file, const char *to_folder);

EXPORT void ReleaseExporter(CXmlExporter *exporter);

//get geometry face data from exporter
EXPORT bool GetGroupFaceDataById(CXmlExporter *exporter,
                        int group_id,
                    	double **vertices,   //vertex position list
                    	int *vertex_num,   //vertex num
                    	int **vertex_num_per_face,   //vertex num of each face
                    	int *face_num,   //face count of the object
                    	int **face_vertex_index,   //vertex position index of each face
                    	VectorHandle *vertices_handle,
                    	VectorHandle *vertices_face_handle,
                    	VectorHandle *face_vindex_handle);

//get front uv or back uv data for each face
EXPORT bool GetFaceUV(CXmlExporter *exporter,
                        bool front_or_back,   //front uv or back uv
                        double **u,   //u list
                        double **v,   //v list
                        int *uv_num,   //u v list size
                        int **uv_id,   //uv id for each vertex of each face in uv list
						int *uv_id_num,
                        VectorHandle *uv_id_handle,
                        VectorHandle *u_handle,
                        VectorHandle *v_handle );

EXPORT int GetMaterialNum(CXmlExporter *exporter);

EXPORT const bool GetMaterialNameByID(CXmlExporter *exporter,int id, char *mat_name);

EXPORT bool GetMaterialData(CXmlExporter *exporter, 
                            int material_id,
                            bool *has_color,
							double color[3],
                            bool *has_alpha,
                            double *alpha,
                            bool *has_texture,
							char  *texture_path,
                            double *tex_sscale,
                            double *tex_tscale );

EXPORT bool GetMaterialIDPerFace(CXmlExporter *exporter,
								int **front_material_id_per_face,//-1 means the face has no material
								int **back_material_id_per_face,
								int *face_num,
								VectorHandle *front_mat_handle,
								VectorHandle *back_mat_handle);

//release memory of vector handle ,called in c#
EXPORT bool ReleaseDoubleHandle(VectorHandle handle);

EXPORT bool ReleaseIntHandle(VectorHandle handle);

EXPORT bool ReleaseBoolHandle(VectorHandle handle);


#endif