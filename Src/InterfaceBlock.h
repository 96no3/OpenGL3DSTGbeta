/**
* @file InterfaceBlock.h
*/
#ifndef INTERFACEBLOCK_H_INCLUDED
#define INTERFACEBLOCK_H_INCLUDED
#include <glm/glm.hpp>

namespace InterfaceBlock {

	static const int maxViewCount = 4;

	/// 頂点シェーダのパラメータ型.
	struct VertexData
	{
		glm::mat4 matMVP[maxViewCount];
		glm::mat4 matDepthMVP;
		glm::mat4 matModel;
		glm::mat3x4 matNormal;
		glm::vec4 color;
	};

	const int maxLightCount = 4; ///< ライトの数.

	/**
	* ライトデータ(点光源).
	*/
	struct PointLight
	{
		glm::vec4 position; ///< 座標(ワールド座標系).
		glm::vec4 color; ///< 明るさ.
	};

	/**
	* ライティングパラメータをシェーダに転送するための構造体.
	*/
	struct LightData
	{
		glm::vec4 eyePos[maxViewCount]; ///< 視点.
		glm::vec4 ambientColor; ///< 環境光.
		PointLight light[maxLightCount]; ///< ライトのリスト.
	};

	/**
	* ポストエフェクトデータをシェーダに転送するための構造体.
	*/
	struct PostEffectData
	{
		glm::mat4x4 matColor; ///< 色変換行列.
		float luminanceScale; ///< 輝度増減係数.
		float bloomThreshold; ///< ブルームを発生させるしきい値.
	};


	/// バインディングポイント.
	enum BindingPoint
	{
		BINDINGPOINT_VERTEXDATA, ///< 頂点シェーダ用パラメータのバインディングポイント.
		BINDINGPOINT_LIGHTDATA, ///< ライティングパラメータ用のバインディングポイント.
		BINDINGPOINT_POSTEFFECTDATA, ///< ポストエフェクトパラメータ用のバインディングポイント.
	};
}

#endif
