#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int size;
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
	inline unsigned int GetCount() const { return size; }
};