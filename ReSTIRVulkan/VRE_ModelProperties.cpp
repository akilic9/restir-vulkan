#include "VRE_ModelProperties.h"

std::vector<VkVertexInputBindingDescription> VRE::Vertex::GetBindingDesc()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VRE::Vertex::GetAttributeDesc()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mPosition) });
    attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mColor) });
    attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mNormal) });
    attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mTexCoord0) });

    return attributeDescriptions;
}

bool VRE::Vertex::operator==(const Vertex& other) const
{
    return mPosition == other.mPosition && mColor == other.mColor &&
           mNormal == other.mNormal && mTexCoord0 == other.mTexCoord0 && mTexCoord1 == other.mTexCoord1;
}

VRE::glTFPrimitive::glTFPrimitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, glTFMaterial& material)
    : mFirstIndex(firstIndex)
    , mIndexCount(indexCount)
    , mVertexCount(vertexCount)
    , mMaterial(material)
    , mHasIndices(mIndexCount > 0) {}

VRE::glTFMesh::glTFMesh()
{
}

VRE::glTFMesh::~glTFMesh()
{
}

VRE::glTFNode::glTFNode(uint32_t index, std::string name, glm::mat4 matrix, std::shared_ptr<glTFNode> parent)
    : mIndex(index)
    , mName(name)
    , mMatrix(matrix)
    , mParent(parent)
{
    mMesh = std::make_unique<glTFMesh>();
}

glm::mat4 VRE::glTFNode::GetLocalMatrix()
{
    if (!mUseCachedMatrix)
        mCachedLocalMatrix = glm::translate(glm::mat4(1.0f), mTranslation) * glm::mat4(mRotation) * glm::scale(glm::mat4(1.0f), mScale) * mMatrix;

    return mCachedLocalMatrix;
}

glm::mat4 VRE::glTFNode::GetMatrix()
{
    if (!mUseCachedMatrix) {
        glm::mat4 m = GetLocalMatrix();
        while (mParent) {
            m = mParent->GetLocalMatrix() * m;
            mParent = mParent->mParent;
        }
        mCachedMatrix = m;
        mUseCachedMatrix = true;
        return m;
    }

    return mCachedMatrix;
}