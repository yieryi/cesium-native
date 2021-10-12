// This file was generated by generate-gltf-classes.
// DO NOT EDIT THIS FILE!
#pragma once

#include "Library.h"

#include <CesiumUtility/ExtensibleObject.h>

#include <optional>
#include <string>

namespace Cesium3DTiles {
/**
 * @brief Metadata about the entire tileset.
 */
struct CESIUM3DTILES_API Asset final : public CesiumUtility::ExtensibleObject {
  static inline constexpr const char* TypeName = "Asset";

  /**
   * @brief The 3D Tiles version. The version defines the JSON schema for the
   * tileset JSON and the base set of tile formats.
   */
  std::string version;

  /**
   * @brief Application-specific version of this tileset, e.g., for when an
   * existing tileset is updated.
   */
  std::optional<std::string> tilesetVersion;
};
} // namespace Cesium3DTiles