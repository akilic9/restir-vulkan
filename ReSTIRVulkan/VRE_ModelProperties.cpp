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
    attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, mColor) });
    attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mNormal) });
    attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mTexCoord0) });
    attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mTexCoord1) });

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

VRE::glTFMesh::glTFMesh(VRE_Device& device, glm::mat4 matrix) : mMatrix(matrix)
{
    VRE_Buffer stagingBuffer(device, sizeof(glm::mat4), 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer(&mMatrix);

    mBuffer = std::make_unique<VRE_Buffer>(device, sizeof(glm::mat4), 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device.CopyBuffer(stagingBuffer.GetBuffer(), mBuffer->GetBuffer(), sizeof(glm::mat4));
}

VRE::glTFMesh::~glTFMesh()
{
}

VRE::glTFNode::glTFNode(VRE_Device& device, uint32_t index, std::string name, glm::mat4 matrix, std::shared_ptr<glTFNode> parent)
    : mIndex(index)
    , mName(name)
    , mMatrix(matrix)
    , mParent(parent)
{
    mMesh = std::make_unique<glTFMesh>(device, mMatrix);
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