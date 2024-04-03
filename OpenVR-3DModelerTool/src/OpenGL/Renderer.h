#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderer
{
private:
	mutable float r, g, b, a;
public:
	Renderer();
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
	void SetBackgroundClr(float v0, float v1, float v2, float v3) const;
	void Clear() const;
};