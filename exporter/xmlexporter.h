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

class CXmlExporter {
  public:
    typedef std::vector<XmlEntitiesInfo> EntityList;

 public:
  CXmlExporter();
  virtual ~CXmlExporter();

  // Convert
  bool Convert(const std::string& from_file,
               SketchUpPluginProgressCallback* callback);
  
  // Set user options
  void SetOptions(const CXmlOptions& options) { options_ = options; }

  // Get stats
  const CXmlExportStats& stats() const { return stats_; }

  //Get group info
  int GroupNum(){return int(final_faces_.size());}
  
  const XmlEntitiesInfo* GroupById(int index){return &final_faces_.at(index);}

  void SetMaxVertexNumPerGroup(int num) { max_vertex_num_pergroup_ = num; };

  XmlMaterialInfo GetMaterialInfo(SUMaterialRef material);

  SUMaterialRef GetMaterialRefByName(std::string mat_name);

  int GetMaterialIdByName(std::string mat_name);

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
                        bool combine_component=false);

  void CheckComponentFaceMaterial(SUEntitiesRef entities, string mat_name);


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

  EntityList final_faces_;
  std::unordered_map<std::string, EntityList> definition_faces_;

public:
	XmlModelInfo skpdata_;
};

#endif // SKPTOXML_COMMON_XMLEXPORTER_H
