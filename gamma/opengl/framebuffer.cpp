#include <map>

#include "opengl/framebuffer.h"
#include "system/console.h"

#include "glew.h"

namespace Gamma {
  const static float defaultBorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

  const static std::map<ColorFormat, GLint> glInternalFormatMap = {
    { ColorFormat::R, GL_R32F },
    { ColorFormat::R16, GL_R16F },
    { ColorFormat::RG, GL_RG32F },
    { ColorFormat::RG16, GL_RG16F },
    { ColorFormat::RGB, GL_RGB32F },
    { ColorFormat::RGB16, GL_RGB16F },
    { ColorFormat::RGBA, GL_RGBA32F },
    { ColorFormat::RGBA16, GL_RGBA16F },
    { ColorFormat::RGBA8, GL_RGBA8 }
  };

  const static std::map<ColorFormat, GLenum> glFormatMap = {
    { ColorFormat::R, GL_RED },
    { ColorFormat::R16, GL_RED },
    { ColorFormat::RG, GL_RG },
    { ColorFormat::RG16, GL_RG },
    { ColorFormat::RGB, GL_RGB },
    { ColorFormat::RGB16, GL_RGB },
    { ColorFormat::RGBA, GL_RGBA },
    { ColorFormat::RGBA16, GL_RGBA },
    { ColorFormat::RGBA8, GL_RGBA }
  };

  /**
   * OpenGLFrameBuffer
   * -----------------
   */
  void OpenGLFrameBuffer::init() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  }

  void OpenGLFrameBuffer::destroy() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteFramebuffers(1, &fbo);

    for (auto& attachment : colorAttachments) {
      glDeleteTextures(1, &attachment.textureId);
    }

    glDeleteTextures(1, &depthTextureId);
    glDeleteTextures(1, &depthStencilTextureId);
  }

  void OpenGLFrameBuffer::addColorAttachment(ColorFormat format) {
    addColorAttachment(format, colorAttachments.size());
  }

  void OpenGLFrameBuffer::addColorAttachment(ColorFormat format, u32 unit) {
    addColorAttachment(format, unit, GL_CLAMP_TO_BORDER);
  }

  void OpenGLFrameBuffer::addColorAttachment(ColorFormat format, u32 unit, GLint clamp) {
    GLuint textureId;
    GLuint index = GL_COLOR_ATTACHMENT0 + colorAttachments.size();
    GLint glInternalFormat = glInternalFormatMap.at(format);
    GLenum glFormat = glFormatMap.at(format);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, size.width, size.height, 0, glFormat, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, defaultBorderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, index, GL_TEXTURE_2D, textureId, 0);

    colorAttachments.push_back({ index, textureId, unit });
  }

  void OpenGLFrameBuffer::addDepthAttachment() {
    glGenTextures(1, &depthTextureId);
    glBindTexture(GL_TEXTURE_2D, depthTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.width, size.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);
  }

  void OpenGLFrameBuffer::addDepthStencilAttachment() {
    glGenTextures(1, &depthStencilTextureId);
    glBindTexture(GL_TEXTURE_2D, depthStencilTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.width, size.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTextureId, 0);
  }

  void OpenGLFrameBuffer::bindColorAttachments() {
    GLuint* attachments = new GLuint[colorAttachments.size()];

    for (u32 i = 0; i < colorAttachments.size(); i++) {
      attachments[i] = colorAttachments[i].index;
    }

    glDrawBuffers(colorAttachments.size(), attachments);

    delete[] attachments;
  }

  void OpenGLFrameBuffer::read(u32 offset) {
    for (u32 i = 0; i < colorAttachments.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + colorAttachments[i].textureUnit + offset);
      glBindTexture(GL_TEXTURE_2D, colorAttachments[i].textureId);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  }

  void OpenGLFrameBuffer::setSize(const Area<u32>& size) {
    this->size = size;
  }

  void OpenGLFrameBuffer::shareDepthStencilAttachment(const OpenGLFrameBuffer& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTextureId, 0);
  }

  void OpenGLFrameBuffer::write() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, size.width, size.height);
  }

  void OpenGLFrameBuffer::writeToAttachment(u32 attachment) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);
  }

  /**
   * OpenGLCubeMap
   * -------------
   */
  void OpenGLCubeMap::init() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  }

  void OpenGLCubeMap::destroy() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDeleteFramebuffers(1, &fbo);

    for (auto& attachment : colorAttachments) {
      glDeleteTextures(1, &attachment.textureId);
    }

    glDeleteTextures(1, &depthTextureId);
  }

  void OpenGLCubeMap::addColorAttachment(ColorFormat format, u32 unit) {
    GLuint textureId;
    GLuint index = GL_COLOR_ATTACHMENT0 + colorAttachments.size();
    GLint glInternalFormat = glInternalFormatMap.at(format);
    GLenum glFormat = glFormatMap.at(format);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    for (u32 i = 0; i < 6; i++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size.width, size.height, 0, glFormat, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, index, textureId, 0);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    colorAttachments.push_back({ index, textureId, unit });
  }

  void OpenGLCubeMap::addDepthAttachment(u32 unit) {
    this->depthTextureUnit = unit;

    glGenTextures(1, &depthTextureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthTextureId);

    for (u32 i = 0; i < 6; i++) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, size.width, size.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureId, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void OpenGLCubeMap::bindColorAttachments() {
    GLuint* attachments = new GLuint[colorAttachments.size()];

    for (u32 i = 0; i < colorAttachments.size(); i++) {
      attachments[i] = colorAttachments[i].index;
    }

    glDrawBuffers(colorAttachments.size(), attachments);

    delete[] attachments;
  }

  void OpenGLCubeMap::read() {
    for (auto& colorAttachment : colorAttachments) {
      glActiveTexture(GL_TEXTURE0 + colorAttachment.textureUnit);
      glBindTexture(GL_TEXTURE_CUBE_MAP, colorAttachment.textureId);
    }

    if (depthTextureId != 0) {
      glActiveTexture(GL_TEXTURE0 + depthTextureUnit);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depthTextureId);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  }

  void OpenGLCubeMap::setSize(const Area<u32>& size) {
    this->size = size;
  }

  void OpenGLCubeMap::write() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, size.width, size.height);
  }

  void OpenGLCubeMap::writeToFace(u8 face) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    for (auto& colorAttachment : colorAttachments) {
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, colorAttachment.index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, colorAttachment.textureId, 0);
    }

    glViewport(0, 0, size.width, size.height);
  }
}