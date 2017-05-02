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

//get the group num in su scene.
//currently we have 2 groups by default,id 0 for normal face,1 for facing camera face
EXPORT  int GetGroupNum(CXmlExporter *exporter);






//get exporter instance for specified skp file,start with it
EXPORT CXmlExporter* GetExporter(const char *from_file);

//release memory,end with it
EXPORT void ReleaseExporter(CXmlExporter *exporter);










//get face and vertex num for specified group id
EXPORT void GetFaceDSize(CXmlExporter *exporter,
                        int group_id,
                        int *vertex_num, 
                        int *face_num );

//get geometry face data from exporter,you should malloc memory with the result of GetFaceDSize
EXPORT bool GetFace(CXmlExporter *exporter,
                        int group_id,
                        float vertices[],   //vertex position list
                        int vertex_num_per_face[],   //vertex num of each face
                        float face_normal[]);

EXPORT void GetFacingCameraId(CXmlExporter *exporter, int id,int *start_face,int *end_face);

EXPORT int GetFacingCameraIdSize(CXmlExporter *exporter);





//get uv size for the specified group id
EXPORT int GetFaceUVDSize(CXmlExporter *exporter,
                        int group_id,
                        bool front_or_back );//true for front,false for back

//get front uv or back uv data for each face,you should malloc uv size memory with the result of GetFaceUVDSize
EXPORT bool GetFaceUV(CXmlExporter *exporter,
                    	int group_id,
                    	bool front_or_back,   //true for front,false for back
                    	float u[],   //u list,-100 means no u
                    	float v[]);   //v list,-100 means no v

//get material num in su scene,traverse all materials in this range
EXPORT int GetMaterialNum(CXmlExporter *exporter);

//get the material name for specified material id.
//you can put it in a for loop get all the material data with length of GetMaterialNum
EXPORT const bool GetMaterialNameByID(CXmlExporter *exporter,int id, char *mat_name);






//get texture pixel size for specified material id if the material has texture,otherwise return 0
EXPORT int GetTexPixelDSize(CXmlExporter *exporter, 
                            int material_id );

//get material data for specified material id
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
                            float pixel_data[]);




//get material id for each face in specified group id
EXPORT bool GetMaterialIDPerFace(CXmlExporter *exporter,
                                int group_id,
								int front_material_id_per_face[],//-1 means the face has no material
								int back_material_id_per_face[]);

#endif