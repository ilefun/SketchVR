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

class CXmlExporter {
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
  int GroupNum(){return int(group_list_.size());}
  
  const XmlGroupInfo* GroupById(int index){return group_list_[index];}

  std::vector<int> GroupChildrenById(int index){return group_children_[index];}

  //store group data
  void GetGroupList(const XmlEntitiesInfo *entities);
  
  void GetGroupChildren();

  XmlMaterialInfo GetMaterialInfo(SUMaterialRef material);

private:
  // Clean up slapi objects
  void ReleaseModelObjects();

  void WriteLayers();
  void WriteLayer(SULayerRef layer);

  void WriteMaterials();
  void WriteMaterial(SUMaterialRef material);

  void WriteComponentDefinitions();
  void WriteComponentDefinition(SUComponentDefinitionRef comp_def);

  void WriteGeometry();
  void WriteEntities(SUEntitiesRef entities,XmlEntitiesInfo *entity_info);
  void WriteFace(SUFaceRef face,XmlEntitiesInfo *entity_info);
  void WriteEdge(SUEdgeRef edge,XmlEntitiesInfo *entity_info);
  void WriteCurve(SUCurveRef curve,XmlEntitiesInfo *entity_info);

  XmlEdgeInfo GetEdgeInfo(SUEdgeRef edge) const;

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

  //
  std::vector<const XmlGroupInfo*> group_list_;
  std::vector<std::vector<int>> group_children_;

public:
	XmlModelInfo skpdata_;
	std::unordered_map<std::string, int> matname_id_map_;
};

#endif // SKPTOXML_COMMON_XMLEXPORTER_H
