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
	using ProgramPtr = std::shared_ptr<Program>; ///< プログラムオブジェクトポインタ.

	/**
	* シェーダープログラムクラス.
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

		GLuint program = 0; ///< プログラムオブジェクト.
		GLint samplerLocation = -1; ///< サンプラーの位置.
		int samplerCount = 0; ///< サンプラーの数.
		GLint viewIndexLocation = -1; ///< 視点インデックスの位置.
		GLint depthSamplerLocation = -1; ///< 深度サンプラーの位置.
		std::string name; ///< プログラム名.
	};

	GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);
}

#endif
