#include "Renderer.h"
#include <iostream>

Renderer::Renderer()
    : r(1.0f), g(1.0f), b(1.0f), a(1.0f)
{}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
    // Background color
    GLCall(glClearColor(r, g, b, a));
    // Start shader
    shader.Bind();
    // Bind Vertex Array
    va.Bind();
    // Bind Index Array
    ib.Bind();
	// Draw.
	//GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, 0));
    GLCall(glDrawElements(GL_LINES, ib.GetCount(), GL_UNSIGNED_INT, 0));
}

void Renderer::SetBackgroundClr(float v0, float v1, float v2, float v3) const
{
    r = v0;
    g = v1;
    b = v2;
    a = v3;
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
}