#pragma once
#include<vector>
#include "G_Vertex.h"

const std::vector<Vertex> vertices = { // Interleaving vertex attribute array.
 // mPos:            mColor: 
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f},  {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f},   {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};