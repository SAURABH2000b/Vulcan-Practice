#pragma once
#include <vulkan/vulkan.h>
#include<glm.hpp>
#include<array>

struct Vertex {
	glm::vec2 m_pos;
	glm::vec3 m_color;
	glm::vec2 m_texCoord;

	static VkVertexInputBindingDescription s_GetBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 3> s_GetAttributeDescriptions();
};

class G_Vertex
{

};

