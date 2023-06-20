#include "Buffer.h"

VertexBuffer::VertexBuffer() : 
	m_VAO(0), m_VBO(0), m_EBO(0), 
	m_layouts({}), n_layouts(0), n_indices(0) {}

void VertexBuffer::createBuffers(const vector<info::VertexLayout>& layouts, const vector<unsigned int>& indices)
{	
	m_layouts = layouts;
	n_layouts = static_cast<unsigned int>(layouts.size());
	n_indices = static_cast<unsigned int>(indices.size());

	cout << " Generate buffer" << endl;
	cout << "  -Create vertex buffers " << m_layouts.size() << endl;
	cout << "  -Size of vertex layout: " << n_layouts << endl;
	cout << "  -Size of indices: " << n_indices << endl;

	// Generate buffers: VAO, VBO, EBO
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, n_layouts * sizeof(info::VertexLayout), &layouts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(POS_ATTRIB);
	glVertexAttribPointer(POS_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)0);

	glEnableVertexAttribArray(NORMAL_ATTRIB);
	glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)offsetof(info::VertexLayout, normal));

	glEnableVertexAttribArray(TANGENT_ATTRIB);
	glVertexAttribPointer(TANGENT_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)offsetof(info::VertexLayout, tangent));

	glEnableVertexAttribArray(TEXCOORD_ATTRIB);
	glVertexAttribPointer(TEXCOORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)offsetof(info::VertexLayout, texCoord));

	// Bone 
	//glEnableVertexAttribArray(BONE_ATTRIB);
	//glVertexAttribIPointer(BONE_ATTRIB, 4, GL_INT, sizeof(VertexLayout), (void*)offsetof(VertexLayout, bone_IDs));

	// Bone weights
	//glEnableVertexAttribArray(BONE_WEIGHT_ATTRIB);
	//glVertexAttribPointer(BONE_WEIGHT_ATTRIB, 4, GL_FLOAT, GL_FALSE, sizeof(VertexLayout), (void*)offsetof(VertexLayout, weights));

	// Reset the vertex array binder
	glBindVertexArray(0);
}

void VertexBuffer::bind() const
{
	glBindVertexArray(m_VAO);
}

void VertexBuffer::unbind() const
{
	glBindVertexArray(0);
}

FrameBuffer::FrameBuffer() : m_RBO(0), m_FBO(0), m_framebuffer_texture(0) {}

void FrameBuffer::createBuffers(int width, int height)
{
	// Create a framebuffer
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	// Create a color attachment texture
	glGenTextures(1, &m_framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer_texture, 0);

	// Create a renderbuffer object for depth and stencil attachment
	glGenRenderbuffers(1, &m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

	// Check framebuffer is complete 
	auto check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (check != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr << "Framebuffer error: " << check << endl;
		assert(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FrameBuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void FrameBuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::rescaleFrame(int width, int height)
{
	glGenTextures(1, &m_framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);
}


