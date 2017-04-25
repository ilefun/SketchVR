// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#ifndef SKPTOXML_COMMON_XMLINHERITANCEMANAGER_H
#define SKPTOXML_COMMON_XMLINHERITANCEMANAGER_H

#include "../common/xmlgeomutils.h"
#include "../common/xmlfile.h"

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/model/defs.h>
#include <vector>

// CInheritanceManager - A cross-platform class that manages the properties
// of geometric elements (faces and edges) that can be inherited from component
// instances, groups and images.  These properties are transformations to world
// space, layers and materials.
class CInheritanceManager {
 public:
  CInheritanceManager();
  CInheritanceManager(bool bMaterialsByLayer);
  virtual ~CInheritanceManager();

  void PushElement(SUGroupRef element);
  void PushElement(SUComponentInstanceRef element);
  void PushElement(SUFaceRef element);
  void PushElement(SUEdgeRef element);
  void PushComponentInfo(const DefinitionInfo *def_info);

  void PopComponentInfo();
  void PopElement();

  const DefinitionInfo* GetCurrentComponentInfo() const;
  SULayerRef GetCurrentLayer() const;
  SUMaterialRef GetCurrentFrontMaterial() const;
  SUMaterialRef GetCurrentBackMaterial() const;
  SUColor GetCurrentEdgeColor() const;

 protected: //Methods
  void PushMaterial(SUDrawingElementRef drawing_element);
  void PushLayer(SUDrawingElementRef drawing_element);

 protected: //Data
  bool materials_by_layer_;
  std::vector<SULayerRef> layers_;
  std::vector<SUMaterialRef> front_materials_;
  std::vector<SUMaterialRef> back_materials_;
  std::vector<SUColor> edge_colors_;
  std::vector<const DefinitionInfo*> definition_infos_;
};

#endif // SKPTOXML_COMMON_XMLINHERITANCEMANAGER_H
