#include "Buffer.h"

VertexBuffer::VertexBuffer() : 
	m_VAO(0), m_VBO(0), m_EBO(0), m_IBO(0),
	m_layouts({}), m_matrices({}), n_layouts(0), n_indices(0)
{
}

void VertexBuffer::createBuffers(const vector<info::VertexLayout>& layouts)
{
	m_layouts = layouts;
	n_layouts = static_cast<unsigned int>(layouts.size());

	//cout << " Generate buffer" << endl;
	//cout << "  -Create vertex buffers " << m_layouts[10].position.x << endl;
	//cout << "  -Size of vertex layout: " << n_layouts << endl;

	// Generate buffers: VAO, VBO, EBO
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, n_layouts * sizeof(info::VertexLayout), &layouts[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(POS_ATTRIB);
	glVertexAttribPointer(POS_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)0);

	glEnableVertexAttribArray(NORMAL_ATTRIB);
	glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)offsetof(info::VertexLayout, normal));

	glEnableVertexAttribArray(TANGENT_ATTRIB);
	glVertexAttribPointer(TANGENT_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)offsetof(info::VertexLayout, tangent));

	glEnableVertexAttribArray(TEXCOORD_ATTRIB);
	glVertexAttribPointer(TEXCOORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, sizeof(info::VertexLayout), (void*)offsetof(info::VertexLayout, texCoord));
	
	glBindVertexArray(0);
}

void VertexBuffer::createBuffers(const vector<info::VertexLayout>& layouts, const vector<unsigned int>& indices)
{	
	m_layouts = layouts;
	n_layouts = static_cast<unsigned int>(layouts.size());
	n_indices = static_cast<unsigned int>(indices.size());

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


	if (m_matrices.size() > 1)
	{
		//cout << "Setup instance mesh" << endl;
		glGenBuffers(1, &m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ARRAY_BUFFER, m_matrices.size() * sizeof(glm::mat4), &m_matrices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(TEXCOORD_ATTRIB + 1);
		glVertexAttribPointer(TEXCOORD_ATTRIB + 1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(TEXCOORD_ATTRIB + 2);
		glVertexAttribPointer(TEXCOORD_ATTRIB + 2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB + 3);
		glVertexAttribPointer(TEXCOORD_ATTRIB + 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB + 4);
		glVertexAttribPointer(TEXCOORD_ATTRIB + 4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(TEXCOORD_ATTRIB + 1, 1);
		glVertexAttribDivisor(TEXCOORD_ATTRIB + 2, 1);
		glVertexAttribDivisor(TEXCOORD_ATTRIB + 3, 1);
		glVertexAttribDivisor(TEXCOORD_ATTRIB + 4, 1);

	}

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

FrameBuffer::FrameBuffer() : m_RBO(0), m_FBO(0), m_framebuffer_texture(0) 
{
}

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FrameBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void FrameBuffer::unbind()
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

void FrameBuffer::bindFrameTexture()
{
	glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
}

ShadowBuffer::ShadowBuffer() : m_shadow_map(0)
{}

ShadowBuffer::~ShadowBuffer()
{}

void ShadowBuffer::createBuffers(int width, int height)
{
	cout << "Create a shadow buffer" << endl;
	
	// Create a texture to store shadow values 
	glGenTextures(1, &m_shadow_map);
	glBindTexture(GL_TEXTURE_2D, m_shadow_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	// Framebuffer to store depth values(shadow map)
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadow_map, 0);
	
	// Disable writes to color buffer, as shadow map will not output the color
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr << "Framebuffer error " << status << endl;
		assert(0);
	}

	// Check any errors
	if (glGetError() != GL_NO_ERROR)
	{
		cerr << "Error: " << glGetError() << endl;
		assert(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void ShadowBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowBuffer::bindFrameTexture()
{
	glBindTexture(GL_TEXTURE_2D, m_shadow_map);
}

CubemapBuffer::CubemapBuffer() : m_cubemap(0), m_width(0), m_height(0)
{}

CubemapBuffer::~CubemapBuffer()
{}

void CubemapBuffer::createBuffers(int width, int height, bool mipmap)
{
	cout << "Create a Framebuffer" << endl;
	// Create a framebuffer
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glGenRenderbuffers(1, &m_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

	cout << "Create cubemap buffer" << endl;
	glGenTextures(1, &m_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
	// Create a six faces for cubemap texture
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (mipmap == false)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	if (glGetError() != GL_NO_ERROR)
	{
		cerr << "Error " << glGetError() << endl;
		assert("error from buffer.cpp");
	}
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr << "Framebuffer error " << status << endl;
		assert("error from buffer.cpp");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void CubemapBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapBuffer::bindFrameTexture(int i)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubemap, 0);
}

void CubemapBuffer::bindMipMapTexture(int i, int mip)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubemap, mip);
}

void CubemapBuffer::bindCubemapTexture()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
}

void CubemapBuffer::bindRenderBuffer(int width, int height)
{
	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
}

