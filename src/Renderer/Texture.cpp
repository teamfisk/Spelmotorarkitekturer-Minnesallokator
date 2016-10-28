#include "Texture.h"

Texture::Texture(std::shared_ptr<ResourceBundle::Block> block)
{
	ResourceHandle<PNG> handle = ResourceManager::Load<PNG>(block->Path());
	auto png = *handle;

	switch (png->Format)
	{
	case Image::ImageFormat::RGBA:
		m_Format = GL_RGBA;
		break;
	case Image::ImageFormat::RGB:
		m_Format = GL_RGB;
	default:
		throw std::runtime_error("Unsupported image format.");
		break;
	}

	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, png->Width, png->Height, 0, m_Format, GL_UNSIGNED_BYTE, png->Data);	
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_Texture);
}

void Texture::Bind(GLenum textureUnit /*= GL_TEXTURE0*/, GLenum access /*= GL_READ_ONLY*/)
{		
	glActiveTexture(textureUnit);	
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	
	// Think this one is for binding to image2D, not sampler2D.
	//glBindImageTexture(0/*this refers to the "image unit", which is apparently different from "texture image unit" i.e. GL_TEXTURE0, GL_TEXTURE1, etc.*/, 
	//	m_Texture, 0, GL_FALSE, 0, access, GL_RGBA32F);	// GL_RGBA seems to not be in this table https://www.opengl.org/sdk/docs/man4/html/glBindImageTexture.xhtml	
}

