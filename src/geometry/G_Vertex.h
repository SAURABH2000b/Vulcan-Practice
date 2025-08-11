#pragma once
#include <vulkan/vulkan.h>
#include<glm.hpp>
#include<array>

struct Vertex {
	glm::vec2 m_Pos;
	glm::vec3 m_Color;

	static VkVertexInputBindingDescription sGetBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> mGetAttributeDescriptions();
};

class G_Vertex
{

};

