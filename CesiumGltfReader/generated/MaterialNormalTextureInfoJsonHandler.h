// This file was generated by generate-gltf-classes.
// DO NOT EDIT THIS FILE!
#pragma once

#include "DoubleJsonHandler.h"
#include "TextureInfoJsonHandler.h"

namespace CesiumGltf {
  struct MaterialNormalTextureInfo;

  class MaterialNormalTextureInfoJsonHandler : public TextureInfoJsonHandler {
  public:
    void reset(IJsonHandler* pHandler, MaterialNormalTextureInfo* pObject);
    MaterialNormalTextureInfo* getObject();
    virtual void reportWarning(const std::string& warning, std::vector<std::string>&& context = std::vector<std::string>()) override;

    virtual IJsonHandler* Key(const char* str, size_t length, bool copy) override;

  protected:
    IJsonHandler* MaterialNormalTextureInfoKey(const char* str, MaterialNormalTextureInfo& o);

  private:

    MaterialNormalTextureInfo* _pObject;
    DoubleJsonHandler _scale;
  };
}