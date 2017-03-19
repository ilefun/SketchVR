#ifndef SKPDATA_EXPORT
#define SKPDATA_EXPORT

#include <iostream>

#include "./xmlexporter.h"
#include "../common/xmlgeomutils.h"

using namespace XmlGeomUtils;

using namespace std;
#define EXPORT extern "C" __declspec(dllexport)


const XmlEntitiesInfo* GetEntitiesInfo(CXmlExporter *exporter, int group_id);

void GetFaceData(int v_per_face_list[],
                    float vertices_list[],
                    float normal_list[],
                    const XmlEntitiesInfo *entities);

void GetUVData(bool front_or_back,
                float u_list[],
                float v_list[],
                const XmlEntitiesInfo *entities);

void GetFaceMaterialData(CXmlExporter *exporter,
						 int front_id[],
                         int back_id[],
                         const XmlEntitiesInfo *entities);

void GetTexturePixel(const XmlMaterialInfo &current_mat, float pixel_data[]);

EXPORT  int GetGroupNum(CXmlExporter *exporter);






//get exporter instance for specified skp file
EXPORT CXmlExporter* GetExporter(const char *from_file);

EXPORT void ReleaseExporter(CXmlExporter *exporter);










//Face data-------------------------------------------
EXPORT void GetFaceDSize(CXmlExporter *exporter,
                        int group_id,
                        int *vertex_num, 
                        int *face_num );

//get geometry face data from exporter
EXPORT bool GetFace(CXmlExporter *exporter,
                        int group_id,
                        float vertices[],   //vertex position list
                        int vertex_num_per_face[],   //vertex num of each face
                        float face_normal[]);








//Face uv data-------------------------------------------
EXPORT int GetFaceUVDSize(CXmlExporter *exporter,
                        int group_id,
                        bool front_or_back );

//get front uv or back uv data for each face
EXPORT bool GetFaceUV(CXmlExporter *exporter,
	int group_id,
	bool front_or_back,   //front uv or back uv
	float u[],   //u list
	float v[]);   //v list

EXPORT int GetMaterialNum(CXmlExporter *exporter);

EXPORT const bool GetMaterialNameByID(CXmlExporter *exporter,int id, char *mat_name);






//Get material----------------------------
EXPORT int GetTexPixelDSize(CXmlExporter *exporter, 
                            int material_id );

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
                            float pixel_data[]);




//Material per face--------------
EXPORT bool GetMaterialIDPerFace(CXmlExporter *exporter,
                                int group_id,
								int front_material_id_per_face[],//-1 means the face has no material
								int back_material_id_per_face[]);

#endif