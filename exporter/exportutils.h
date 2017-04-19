#ifndef SKPTOXML_EXPORTER_EXPORTUTILS_H
#define SKPTOXML_EXPORTER_EXPORTUTILS_H

#include "../common/stringutils.h"
#include "../common/xmlfile.h"

#include <SketchUpAPI/import_export/pluginprogresscallback.h>
#include <SketchUpAPI/model/defs.h>

#include <unordered_map>
#include <vector>

struct ExportUtils
{
    // Utility function to get a material's name
    static std::string GetMaterialName(SUMaterialRef material);

    // Utility function to get a layer's name
    static std::string GetLayerName(SULayerRef layer);

    // Utility function to get a component definition's name
    static std::string GetComponentDefinitionName(SUComponentDefinitionRef comp_def);

    //get material info from materialref
    static XmlMaterialInfo GetMaterialInfo(SUMaterialRef material,SUImageRepRef image_rep);

    static void GetTransformedFace(XmlEntitiesInfo *to_entities,
                          XmlEntitiesInfo *from_entities,
                          std::vector<SUTransformation> &transforms);

    static void FixNormal(XmlEntitiesInfo &entity_info);

};

#endif // SKPTOXML_EXPORTER_EXPORTUTILS_H
