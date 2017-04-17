// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLEXPORTER_H
#define SKPTOXML_COMMON_XMLEXPORTER_H

#include "./xmlinheritancemanager.h"
#include "./xmloptions.h"
#include "./xmlstats.h"
#include "../common/xmlfile.h"

#include <SketchUpAPI/import_export/pluginprogresscallback.h>
#include <SketchUpAPI/model/defs.h>

#include <unordered_map>
#include <vector>

// su coordinate system is right-z-up,use matrix 
// 1 0 0 0
// 0 0 1 0
// 0 1 0 0
// 0 0 0 1
// to convert it to left-y-up hand.Transform the point with it.

struct StringConvertUtils
{
  static std::string UTF8_To_string(const std::string & str);
  static std::string string_To_UTF8(const std::string & str);
};

class CXmlExporter {
  public:
    typedef std::vector<XmlEntitiesInfo> EntityList;

 public:
  CXmlExporter();
  virtual ~CXmlExporter();

  // Convert
  bool Convert(const std::string& from_file,
               SketchUpPluginProgressCallback* callback);
  
  void debug_print();

  // Set user options
  void SetOptions(const CXmlOptions& options) { options_ = options; }

  // Get stats
  const CXmlExportStats& stats() const { return stats_; }

  //Get group info
  int GroupNum(){return int(final_faces_.size());}
  
  const XmlEntitiesInfo* GroupById(int index){return &final_faces_.at(index);}

  void SetMaxVertexNumPerGroup(int num) { max_vertex_num_pergroup_ = num; };

  XmlMaterialInfo GetMaterialInfo(SUMaterialRef material);

  void FixNormal();

private:
  // Clean up slapi objects
  void ReleaseModelObjects();

  void WriteLayers();
  void WriteLayer(SULayerRef layer);

  void WriteMaterials();
  void WriteMaterial(SUMaterialRef material);

  void CombineComponentDefinitions(std::vector<std::string> definition_name_list);
  void WriteComponentDefinitions();
  std::string WriteComponentDefinition(SUComponentDefinitionRef comp_def);

  void WriteGeometry();
  void WriteEntities(SUEntitiesRef entities,XmlEntitiesInfo *entity_info);
  void WriteFace(SUFaceRef face,XmlEntitiesInfo *entity_info);
  void WriteEdge(SUEdgeRef edge,XmlEntitiesInfo *entity_info);
  void WriteCurve(SUCurveRef curve,XmlEntitiesInfo *entity_info);

  XmlEdgeInfo GetEdgeInfo(SUEdgeRef edge) const;

  void CombineEntities(XmlEntitiesInfo *entities,
                				EntityList &faces_group,
                        std::vector<SUTransformation> &transform,
        			   		    size_t index=0,
                        bool combine_component=false,
                        std::string *override_mat_name=NULL);

  void GetTransformedFace(XmlEntitiesInfo *to_entities,
	  XmlEntitiesInfo *from_entities,
	  std::vector<SUTransformation> &transforms,
    std::string *override_mat_name=NULL);


private:
  CXmlOptions options_;

  // Export statistics. Filled in by this exporters class and used later by
  // the platform specific plugin classes to populate the results dialog.
  CXmlExportStats stats_;

  // SLAPI model and texture writer
  SUModelRef model_;
  SUTextureWriterRef texture_writer_;
  SUImageRepRef image_rep_;

  std::string skp_file_;

  // Stack
  CInheritanceManager inheritance_manager_;

  int max_vertex_num_pergroup_;
  EntityList final_faces_;
  std::unordered_map<std::string, EntityList> definition_faces_;

public:
	XmlModelInfo skpdata_;
	std::unordered_map<std::string, int> matname_id_map_;
};

#endif // SKPTOXML_COMMON_XMLEXPORTER_H
