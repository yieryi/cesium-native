#include "CesiumGltf/AccessorUtility.h"

#include <catch2/catch.hpp>

#include <cstring>

using namespace CesiumGltf;

TEST_CASE("Test CountFromAccessor") {
  Model model;
  std::vector<uint8_t> featureIds{1, 2, 3, 4};

  Buffer& buffer = model.buffers.emplace_back();
  buffer.cesium.data.resize(featureIds.size() * sizeof(uint8_t));
  std::memcpy(
      buffer.cesium.data.data(),
      featureIds.data(),
      buffer.cesium.data.size());
  buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

  BufferView& bufferView = model.bufferViews.emplace_back();
  bufferView.buffer = 0;
  bufferView.byteLength = buffer.byteLength;

  Accessor& accessor = model.accessors.emplace_back();
  accessor.bufferView = 0;
  accessor.componentType = Accessor::ComponentType::UNSIGNED_BYTE;
  accessor.type = Accessor::Type::SCALAR;
  accessor.count = bufferView.byteLength;

  SECTION("Handles invalid accessor") {
    // Wrong type
    TexCoordAccessorType texcoordAccessor =
        AccessorView<AccessorTypes::VEC2<uint8_t>>(model, accessor);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texcoordAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, texcoordAccessor) == 0);

    // Wrong component type
    FeatureIdAccessorType featureIdAccessor =
        AccessorView<int16_t>(model, accessor);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIdAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, featureIdAccessor) == 0);
  }

  SECTION("Retrieves from valid accessor") {
    FeatureIdAccessorType featureIdAccessor =
        AccessorView<uint8_t>(model, accessor);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIdAccessor) ==
        AccessorViewStatus::Valid);
    int64_t count = std::visit(CountFromAccessor{}, featureIdAccessor);
    REQUIRE(count == static_cast<int64_t>(featureIds.size()));
  }
}

TEST_CASE("Test getFeatureIdAccessorView") {
  Model model;
  std::vector<uint8_t> featureIds0{1, 2, 3, 4};

  // First _FEATURE_ID set
  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(featureIds0.size() * sizeof(uint8_t));
    std::memcpy(
        buffer.cesium.data.data(),
        featureIds0.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 0;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 0;
    accessor.componentType = Accessor::ComponentType::UNSIGNED_BYTE;
    accessor.type = Accessor::Type::SCALAR;
    accessor.count = bufferView.byteLength;
  }

  std::vector<uint16_t> featureIds1{5, 6, 7, 8};

  // Second _FEATURE_ID set
  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(featureIds1.size() * sizeof(uint16_t));
    std::memcpy(
        buffer.cesium.data.data(),
        featureIds1.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 1;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 1;
    accessor.componentType = Accessor::ComponentType::UNSIGNED_SHORT;
    accessor.type = Accessor::Type::SCALAR;
    accessor.count =
        bufferView.byteLength / static_cast<int64_t>(sizeof(uint16_t));
  }

  Mesh& mesh = model.meshes.emplace_back();
  MeshPrimitive primitive = mesh.primitives.emplace_back();

  primitive.attributes.insert({"_FEATURE_ID_0", 0});
  primitive.attributes.insert({"_FEATURE_ID_1", 1});

  SECTION("Handles invalid feature ID set index") {
    FeatureIdAccessorType featureIDAccessor =
        getFeatureIdAccessorView(model, primitive, 2);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIDAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, featureIDAccessor) == 0);
  }

  SECTION("Handles invalid accessor type") {
    model.accessors[0].type = Accessor::Type::VEC2;

    FeatureIdAccessorType featureIDAccessor =
        getFeatureIdAccessorView(model, primitive, 0);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIDAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, featureIDAccessor) == 0);

    model.accessors[0].type = Accessor::Type::SCALAR;
  }

  SECTION("Handles invalid normalized accessor") {
    model.accessors[1].normalized = true;

    FeatureIdAccessorType featureIDAccessor =
        getFeatureIdAccessorView(model, primitive, 1);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIDAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, featureIDAccessor) == 0);

    model.accessors[1].normalized = false;
  }

  SECTION("Creates from valid feature ID sets") {
    FeatureIdAccessorType featureIDAccessor =
        getFeatureIdAccessorView(model, primitive, 0);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIDAccessor) ==
        AccessorViewStatus::Valid);
    REQUIRE(
        std::visit(CountFromAccessor{}, featureIDAccessor) ==
        static_cast<int64_t>(featureIds0.size()));

    featureIDAccessor = getFeatureIdAccessorView(model, primitive, 1);
    REQUIRE(
        std::visit(StatusFromAccessor{}, featureIDAccessor) ==
        AccessorViewStatus::Valid);
    REQUIRE(
        std::visit(CountFromAccessor{}, featureIDAccessor) ==
        static_cast<int64_t>(featureIds1.size()));
  }
}

TEST_CASE("FeatureIdFromAccessor") {
  Model model;
  std::vector<int8_t> featureIds{1, 2, 3, 4};

  Buffer& buffer = model.buffers.emplace_back();
  buffer.cesium.data.resize(featureIds.size() * sizeof(int8_t));
  std::memcpy(
      buffer.cesium.data.data(),
      featureIds.data(),
      buffer.cesium.data.size());
  buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

  BufferView& bufferView = model.bufferViews.emplace_back();
  bufferView.buffer = 0;
  bufferView.byteLength = buffer.byteLength;

  Accessor& accessor = model.accessors.emplace_back();
  accessor.bufferView = 0;
  accessor.componentType = Accessor::ComponentType::BYTE;
  accessor.type = Accessor::Type::SCALAR;
  accessor.count = bufferView.byteLength;

  SECTION("Handles invalid accessor") {
    // Wrong component type
    FeatureIdAccessorType featureIdAccessor =
        AccessorView<int16_t>(model, accessor);
    REQUIRE(std::visit(FeatureIdFromAccessor{0}, featureIdAccessor) == -1);
  }

  SECTION("Retrieves from valid accessor") {
    FeatureIdAccessorType featureIdAccessor =
        AccessorView<int8_t>(model, accessor);
    for (size_t i = 0; i < featureIds.size(); i++) {
      int64_t featureID = std::visit(
          FeatureIdFromAccessor{static_cast<int64_t>(i)},
          featureIdAccessor);
      REQUIRE(featureID == featureIds[i]);
    }
  }
}

TEST_CASE("Test getIndexAccessorView") {
  Model model;
  std::vector<uint8_t> indices{0, 1, 2, 0, 2, 3};

  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(indices.size() * sizeof(uint8_t));
    std::memcpy(
        buffer.cesium.data.data(),
        indices.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 0;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 0;
    accessor.componentType = Accessor::ComponentType::UNSIGNED_BYTE;
    accessor.type = Accessor::Type::SCALAR;
    accessor.count = bufferView.byteLength;
  }

  Mesh& mesh = model.meshes.emplace_back();
  MeshPrimitive primitive = mesh.primitives.emplace_back();
  primitive.indices = 0;

  SECTION("Handles invalid accessor type") {
    model.accessors[0].type = Accessor::Type::VEC2;

    IndexAccessorType indexAccessor = getIndexAccessorView(model, primitive);
    REQUIRE(
        std::visit(StatusFromAccessor{}, indexAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, indexAccessor) == 0);

    model.accessors[0].type = Accessor::Type::SCALAR;
  }

  SECTION("Handles unsupported accessor component type") {
    model.accessors[0].componentType = Accessor::ComponentType::BYTE;

    IndexAccessorType indexAccessor = getIndexAccessorView(model, primitive);
    REQUIRE(
        std::visit(StatusFromAccessor{}, indexAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, indexAccessor) == 0);

    model.accessors[0].componentType = Accessor::ComponentType::UNSIGNED_BYTE;
  }

  SECTION("Handles invalid normalized accessor") {
    model.accessors[0].normalized = true;

    IndexAccessorType indexAccessor = getIndexAccessorView(model, primitive);
    REQUIRE(
        std::visit(StatusFromAccessor{}, indexAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, indexAccessor) == 0);

    model.accessors[0].normalized = false;
  }

  SECTION("Creates from valid accessor") {
    IndexAccessorType indexAccessor = getIndexAccessorView(model, primitive);
    REQUIRE(
        std::visit(StatusFromAccessor{}, indexAccessor) ==
        AccessorViewStatus::Valid);
    REQUIRE(
        std::visit(CountFromAccessor{}, indexAccessor) ==
        static_cast<int64_t>(indices.size()));
  }

  SECTION("Creates from nonexistent accessor") {
    primitive.indices = -1;

    IndexAccessorType indexAccessor = getIndexAccessorView(model, primitive);
    REQUIRE(std::get_if<std::monostate>(&indexAccessor));
  }
}

TEST_CASE("Test FaceVertexIndicesFromAccessor") {
  Model model;
  std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 6, 7, 8};
  int64_t vertexCount = 9;

  Buffer& buffer = model.buffers.emplace_back();
  buffer.cesium.data.resize(indices.size() * sizeof(uint32_t));
  std::memcpy(
      buffer.cesium.data.data(),
      indices.data(),
      buffer.cesium.data.size());
  buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

  BufferView& bufferView = model.bufferViews.emplace_back();
  bufferView.buffer = 0;
  bufferView.byteLength = buffer.byteLength;

  Accessor& accessor = model.accessors.emplace_back();
  accessor.bufferView = 0;
  accessor.componentType = Accessor::ComponentType::UNSIGNED_INT;
  accessor.type = Accessor::Type::SCALAR;
  accessor.count =
      bufferView.byteLength / static_cast<int64_t>(sizeof(uint32_t));

  SECTION("Handles invalid accessor") {
    // Wrong component type
    IndexAccessorType indexAccessor = AccessorView<uint8_t>(model, accessor);
    auto indicesForFace =
        std::visit(IndicesForFaceFromAccessor{0, vertexCount}, indexAccessor);
    for (int64_t index : indicesForFace) {
      REQUIRE(index == -1);
    }
  }

  SECTION("Handles invalid face index") {
    IndexAccessorType indexAccessor = AccessorView<uint32_t>(model, accessor);
    auto indicesForFace =
        std::visit(IndicesForFaceFromAccessor{-1, vertexCount}, indexAccessor);
    for (int64_t index : indicesForFace) {
      REQUIRE(index == -1);
    }

    indicesForFace =
        std::visit(IndicesForFaceFromAccessor{10, vertexCount}, indexAccessor);
    for (int64_t index : indicesForFace) {
      REQUIRE(index == -1);
    }
  }

  SECTION("Retrieves from valid accessor and face index") {
    IndexAccessorType indexAccessor = AccessorView<uint32_t>(model, accessor);
    const size_t numFaces = indices.size() / 3;
    for (size_t i = 0; i < numFaces; i++) {
      auto indicesForFace = std::visit(
          IndicesForFaceFromAccessor{static_cast<int64_t>(i), vertexCount},
          indexAccessor);

      for (size_t j = 0; j < indicesForFace.size(); j++) {
        int64_t expected = static_cast<int64_t>(indices[i * 3 + j]);
        REQUIRE(indicesForFace[j] == expected);
      }
    }
  }

  SECTION("Handles invalid face index for nonexistent accessor") {
    IndexAccessorType indexAccessor;
    auto indicesForFace =
        std::visit(IndicesForFaceFromAccessor{-1, vertexCount}, indexAccessor);
    for (int64_t index : indicesForFace) {
      REQUIRE(index == -1);
    }

    indicesForFace =
        std::visit(IndicesForFaceFromAccessor{10, vertexCount}, indexAccessor);
    for (int64_t index : indicesForFace) {
      REQUIRE(index == -1);
    }
  }

  SECTION("Retrieves from valid face index for nonexistent accessor") {
    IndexAccessorType indexAccessor;
    const int64_t numFaces = vertexCount / 3;

    for (int64_t i = 0; i < numFaces; i++) {
      auto indicesForFace =
          std::visit(IndicesForFaceFromAccessor{i, vertexCount}, indexAccessor);

      for (size_t j = 0; j < indicesForFace.size(); j++) {
        int64_t expected = i * 3 + static_cast<int64_t>(j);
        REQUIRE(indicesForFace[j] == expected);
      }
    }
  }
}

TEST_CASE("Test getTexCoordAccessorView") {
  Model model;
  std::vector<glm::vec2> texCoords0{
      glm::vec2(0, 0),
      glm::vec2(1, 0),
      glm::vec2(0, 1),
      glm::vec2(1, 1)};

  // First TEXCOORD set
  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(texCoords0.size() * sizeof(glm::vec2));
    std::memcpy(
        buffer.cesium.data.data(),
        texCoords0.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 0;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 0;
    accessor.componentType = Accessor::ComponentType::FLOAT;
    accessor.type = Accessor::Type::VEC2;
    accessor.count =
        bufferView.byteLength / static_cast<int64_t>(sizeof(glm::vec2));
  }

  std::vector<glm::u8vec2> texCoords1{
      glm::u8vec2(0, 0),
      glm::u8vec2(0, 255),
      glm::u8vec2(255, 255),
      glm::u8vec2(255, 0)};

  // Second TEXCOORD set
  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(texCoords1.size() * sizeof(glm::u8vec2));
    std::memcpy(
        buffer.cesium.data.data(),
        texCoords1.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 1;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 1;
    accessor.componentType = Accessor::ComponentType::UNSIGNED_BYTE;
    accessor.type = Accessor::Type::VEC2;
    accessor.normalized = true;
    accessor.count =
        bufferView.byteLength / static_cast<int64_t>(sizeof(glm::u8vec2));
  }

  Mesh& mesh = model.meshes.emplace_back();
  MeshPrimitive primitive = mesh.primitives.emplace_back();

  primitive.attributes.insert({"TEXCOORD_0", 0});
  primitive.attributes.insert({"TEXCOORD_1", 1});

  SECTION("Handles invalid texture coordinate set index") {
    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 2);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texCoordAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, texCoordAccessor) == 0);
  }

  SECTION("Handles invalid accessor type") {
    model.accessors[0].type = Accessor::Type::SCALAR;

    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 0);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texCoordAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, texCoordAccessor) == 0);

    model.accessors[0].type = Accessor::Type::VEC2;
  }

  SECTION("Handles unsupported accessor component type") {
    model.accessors[0].componentType = Accessor::ComponentType::BYTE;

    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 0);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texCoordAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, texCoordAccessor) == 0);

    model.accessors[0].componentType = Accessor::ComponentType::FLOAT;
  }

  SECTION("Handles invalid un-normalized texcoord") {
    model.accessors[1].normalized = false;

    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 2);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texCoordAccessor) !=
        AccessorViewStatus::Valid);
    REQUIRE(std::visit(CountFromAccessor{}, texCoordAccessor) == 0);

    model.accessors[1].normalized = true;
  }

  SECTION("Creates from valid texture coordinate sets") {
    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 0);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texCoordAccessor) ==
        AccessorViewStatus::Valid);
    REQUIRE(
        std::visit(CountFromAccessor{}, texCoordAccessor) ==
        static_cast<int64_t>(texCoords0.size()));

    texCoordAccessor = getTexCoordAccessorView(model, primitive, 1);
    REQUIRE(
        std::visit(StatusFromAccessor{}, texCoordAccessor) ==
        AccessorViewStatus::Valid);
    REQUIRE(
        std::visit(CountFromAccessor{}, texCoordAccessor) ==
        static_cast<int64_t>(texCoords1.size()));
  }
}

TEST_CASE("Test TexCoordFromAccessor") {
  Model model;
  std::vector<glm::vec2> texCoords0{
      glm::vec2(0, 0),
      glm::vec2(1, 0),
      glm::vec2(0, 1),
      glm::vec2(1, 1)};

  // First TEXCOORD set
  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(texCoords0.size() * sizeof(glm::vec2));
    std::memcpy(
        buffer.cesium.data.data(),
        texCoords0.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 0;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 0;
    accessor.componentType = Accessor::ComponentType::FLOAT;
    accessor.type = Accessor::Type::VEC2;
    accessor.count =
        bufferView.byteLength / static_cast<int64_t>(sizeof(glm::vec2));
  }

  std::vector<glm::u8vec2> texCoords1{
      glm::u8vec2(0, 0),
      glm::u8vec2(0, 255),
      glm::u8vec2(255, 255),
      glm::u8vec2(255, 0)};

  // Second TEXCOORD set
  {
    Buffer& buffer = model.buffers.emplace_back();
    buffer.cesium.data.resize(texCoords1.size() * sizeof(glm::u8vec2));
    std::memcpy(
        buffer.cesium.data.data(),
        texCoords1.data(),
        buffer.cesium.data.size());
    buffer.byteLength = static_cast<int64_t>(buffer.cesium.data.size());

    BufferView& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = 1;
    bufferView.byteLength = buffer.byteLength;

    Accessor& accessor = model.accessors.emplace_back();
    accessor.bufferView = 1;
    accessor.componentType = Accessor::ComponentType::UNSIGNED_BYTE;
    accessor.type = Accessor::Type::VEC2;
    accessor.normalized = true;
    accessor.count =
        bufferView.byteLength / static_cast<int64_t>(sizeof(glm::u8vec2));
  }

  Mesh& mesh = model.meshes.emplace_back();
  MeshPrimitive primitive = mesh.primitives.emplace_back();

  primitive.attributes.insert({"TEXCOORD_0", 0});
  primitive.attributes.insert({"TEXCOORD_1", 1});

  SECTION("Handles invalid accessor") {
    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 2);
    REQUIRE(!std::visit(TexCoordFromAccessor{0}, texCoordAccessor));
  }

  SECTION("Handles invalid index") {
    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 0);
    REQUIRE(!std::visit(TexCoordFromAccessor{-1}, texCoordAccessor));
    REQUIRE(!std::visit(TexCoordFromAccessor{10}, texCoordAccessor));
  }

  SECTION("Retrieves from valid accessor and index") {
    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 0);
    for (size_t i = 0; i < texCoords0.size(); i++) {
      auto maybeTexCoord = std::visit(
          TexCoordFromAccessor{static_cast<int64_t>(i)},
          texCoordAccessor);
      REQUIRE(maybeTexCoord);

      auto expected = glm::dvec2(texCoords0[i][0], texCoords0[i][1]);
      REQUIRE(*maybeTexCoord == expected);
    }
  }
  SECTION("Retrieves from valid normalized accessor and index") {
    TexCoordAccessorType texCoordAccessor =
        getTexCoordAccessorView(model, primitive, 1);
    for (size_t i = 0; i < texCoords1.size(); i++) {
      auto maybeTexCoord = std::visit(
          TexCoordFromAccessor{static_cast<int64_t>(i)},
          texCoordAccessor);
      REQUIRE(maybeTexCoord);
      auto expected = glm::dvec2(texCoords1[i][0], texCoords1[i][1]);
      expected /= 255;

      REQUIRE(*maybeTexCoord == expected);
    }
  }
}
