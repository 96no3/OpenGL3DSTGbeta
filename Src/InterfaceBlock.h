/**
* @file InterfaceBlock.h
*/
#ifndef INTERFACEBLOCK_H_INCLUDED
#define INTERFACEBLOCK_H_INCLUDED
#include <glm/glm.hpp>

namespace InterfaceBlock {

	static const int maxViewCount = 4;

	/// ���_�V�F�[�_�̃p�����[�^�^.
	struct VertexData
	{
		glm::mat4 matMVP[maxViewCount];
		glm::mat4 matDepthMVP;
		glm::mat4 matModel;
		glm::mat3x4 matNormal;
		glm::vec4 color;
	};

	const int maxLightCount = 4; ///< ���C�g�̐�.

	/**
	* ���C�g�f�[�^(�_����).
	*/
	struct PointLight
	{
		glm::vec4 position; ///< ���W(���[���h���W�n).
		glm::vec4 color; ///< ���邳.
	};

	/**
	* ���C�e�B���O�p�����[�^���V�F�[�_�ɓ]�����邽�߂̍\����.
	*/
	struct LightData
	{
		glm::vec4 eyePos[maxViewCount]; ///< ���_.
		glm::vec4 ambientColor; ///< ����.
		PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
	};

	/**
	* �|�X�g�G�t�F�N�g�f�[�^���V�F�[�_�ɓ]�����邽�߂̍\����.
	*/
	struct PostEffectData
	{
		glm::mat4x4 matColor; ///< �F�ϊ��s��.
		float luminanceScale; ///< �P�x�����W��.
		float bloomThreshold; ///< �u���[���𔭐������邵�����l.
	};


	/// �o�C���f�B���O�|�C���g.
	enum BindingPoint
	{
		BINDINGPOINT_VERTEXDATA, ///< ���_�V�F�[�_�p�p�����[�^�̃o�C���f�B���O�|�C���g.
		BINDINGPOINT_LIGHTDATA, ///< ���C�e�B���O�p�����[�^�p�̃o�C���f�B���O�|�C���g.
		BINDINGPOINT_POSTEFFECTDATA, ///< �|�X�g�G�t�F�N�g�p�����[�^�p�̃o�C���f�B���O�|�C���g.
	};
}

#endif