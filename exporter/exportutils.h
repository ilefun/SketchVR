#ifndef SKPTOXML_EXPORTER_EXPORTUTILS_H
#define SKPTOXML_EXPORTER_EXPORTUTILS_H

#include "../common/stringutils.h"
#include "../common/xmlfile.h"
#include "../common/xmlgeomutils.h"

#include <SketchUpAPI/import_export/pluginprogresscallback.h>
#include <SketchUpAPI/model/defs.h>

#include <unordered_map>
#include <vector>

using namespace std;

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

	static void CheckFaceMaterial(std::vector<SUFaceRef> &faces, SUMaterialRef mat_ref, std::vector<std::pair<size_t, bool>> &face_no_material);

	static void ClearFaceMaterial(std::vector<SUFaceRef> &faces, const std::vector<std::pair<size_t, bool>> &face_no_material);
};

#endif // SKPTOXML_EXPORTER_EXPORTUTILS_H