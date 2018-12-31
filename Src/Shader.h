/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <GL/glew.h>
#include <string>
#include <memory>

class UniformBuffer;

namespace Shader {

	class Program;
	using ProgramPtr = std::shared_ptr<Program>; ///< �v���O�����I�u�W�F�N�g�|�C���^.

	/**
	* �V�F�[�_�[�v���O�����N���X.
	*/
	class Program
	{
	public:
		static ProgramPtr Create(const char* vsFilename, const char* fsFilename);

		bool UniformBlockBinding(const char* blockName, GLuint bindingPoint);
		bool UniformBlockBinding(const UniformBuffer&);
		void UseProgram();
		void BindTexture(GLenum unit, GLenum type, GLuint texture);
		void BindShadowTexture(GLenum type, GLuint texture);
		void SetViewIndex(int index);

	private:
		Program() = default;
		~Program();
		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		GLuint program = 0; ///< �v���O�����I�u�W�F�N�g.
		GLint samplerLocation = -1; ///< �T���v���[�̈ʒu.
		int samplerCount = 0; ///< �T���v���[�̐�.
		GLint viewIndexLocation = -1; ///< ���_�C���f�b�N�X�̈ʒu.
		GLint depthSamplerLocation = -1; ///< �[�x�T���v���[�̈ʒu.
		std::string name; ///< �v���O������.
	};

	GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);
}

#endif