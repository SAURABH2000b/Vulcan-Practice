#include "G_Vertex.h"

VkVertexInputBindingDescription Vertex::s_GetBindingDescription()
{

    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0; // Specify the index of this current binding in the array of bindings.
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;

}

std::array<VkVertexInputAttributeDescription, 3> Vertex::s_GetAttributeDescriptions()
{

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    // Position Attribute Description:
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0; // Refers the location directive of the input in the vertex shader 
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, m_pos);

    // Color Attribute Description:
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, m_color);

    // Texture Coordinates Attribute Description:
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, m_texCoord);

    return attributeDescriptions;

}
