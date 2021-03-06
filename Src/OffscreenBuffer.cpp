/**
* @file OffscreenBuffer.cpp
*/
#include "OffscreenBuffer.h"
#include <iostream>

/**
* オフスクリーンバッファを作成する.
*
* @param w オフスクリーンバッファの幅(ピクセル単位).
* @param h オフスクリーンバッファの高さ(ピクセル単位).
* @param f テクスチャ形式.
*
* @return 作成したオフスクリーンバッファへのポインタ.
*/
//OffscreenBufferPtr OffscreenBuffer::Create(int w, int h)
OffscreenBufferPtr OffscreenBuffer::Create(int w, int h, GLenum iformat)
{
	struct Impl : OffscreenBuffer {};
	OffscreenBufferPtr offscreen = std::make_shared<Impl>();
	if (!offscreen) {
		return offscreen;
	}

	GLenum format = GL_RGBA;
	switch (iformat) {
	default:
		format = GL_RGBA;
		break;
	case GL_DEPTH_COMPONENT16:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH_COMPONENT32:
	case GL_DEPTH_COMPONENT32F:
		format = GL_DEPTH_COMPONENT;
		break;
	}

	// テクスチャを作成する.
	//offscreen->tex = Texture::Create(w, h, iformat, GL_RGBA, nullptr);
	offscreen->tex = Texture::Create(w, h, iformat, format, nullptr);
	if (!offscreen->tex) {
		return {};
	}

	//// 深度バッファを作成する.
	//glGenRenderbuffers(1, &offscreen->depthbuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//// フレームバッファを作成する.
	//glGenFramebuffers(1, &offscreen->framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// デプスシャドウマップを作成する.
	if (format == GL_DEPTH_COMPONENT) {
		glGenFramebuffers(1, &offscreen->framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
		glDrawBuffer(GL_NONE);
	}
	else {// 深度バッファを作成する.
		glGenRenderbuffers(1, &offscreen->depthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// フレームバッファを作成する.
		glGenFramebuffers(1, &offscreen->framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR: OffscreenBufferの作成に失敗." << std::endl;
		offscreen.reset();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return offscreen;
}

/**
* デストラクタ.
*/
OffscreenBuffer::~OffscreenBuffer()
{
	if (framebuffer) {
		glDeleteFramebuffers(1, &framebuffer);
	}
	if (depthbuffer) {
		glDeleteRenderbuffers(1, &depthbuffer);
	}
}

