/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GLFWEW.h"
#include "Audio.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace /* unnamed */ {

	/// 頂点データ型.
	struct Vertex
	{
		glm::vec3 position; ///< 座標.
		glm::vec4 color; ///< 色.
		glm::vec2 texCoord; ///< テクスチャ座標.
	};


	/// 頂点データ.
	const Vertex vertices[] = {

		{ {-0.5f, -0.3f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
		{ { 0.3f, -0.3f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { 0.3f,  0.5f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
		{ {-0.5f,  0.5f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

		{ {-0.3f,  0.3f, 0.1f},{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
		{ {-0.3f, -0.5f, 0.1f},{ 0.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.1f},{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { 0.5f, -0.5f, 0.1f},{ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { 0.5f,  0.3f, 0.1f},{ 1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
		{ {-0.3f,  0.3f, 0.1f},{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },

		{ {-1.0f,-1.0f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
		{ { 1.0f,-1.0f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { 1.0f, 1.0f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
		{ {-1.0f, 1.0f, 0.5f},{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

	};

	/// インデックスデータ.
	const GLuint indices[] = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 7, 8, 9,
		10, 11, 12, 12, 13, 10,
	};

	/**
* 部分描画データ.
*/
	struct RenderingPart
	{
		GLvoid* offset; ///< 描画開始インデックスのバイトオフセット.
		GLsizei size; ///< 描画するインデックス数.
	};

	/**
	* RenderingPartを作成する.
	*
	* @param offset 描画開始インデックスのオフセット(インデックス単位).
	* @param size 描画するインデックス数.
	*
	* @return 作成した部分描画オブジェクト.
	*/
	constexpr RenderingPart MakeRenderingPart(GLsizei offset, GLsizei size) {
		return { reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)), size };
	}

	/**
	* 部分描画データリスト.
	*/
	static const RenderingPart renderingParts[] = {
		MakeRenderingPart(0, 12),
		MakeRenderingPart(12, 6),
	};

	/**
	* Vertex Buffer Objectを作成する.
	*
	* @param size 頂点データのサイズ.
	* @param data 頂点データへのポインタ.
	*
	* @return 作成したVBO.
	*/
	GLuint CreateVBO(GLsizeiptr size, const GLvoid* data)
	{
		GLuint vbo = 0;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return vbo;
	}

	/**
	* Index Buffer Objectを作成する.
	*
	* @param size インデックスデータのサイズ.
	* @param data インデックスデータへのポインタ.
	*
	* @return 作成したIBO.
	*/
	GLuint CreateIBO(GLsizeiptr size, const GLvoid* data)
	{
		GLuint ibo = 0;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		return ibo;
	}

	/**
	* 頂点アトリビュートを設定する.
	*
	* @param index 頂点アトリビュートのインデックス.
	* @param cls   頂点データ型名.
	* @param mbr   頂点アトリビュートに設定するclsのメンバ変数名.
	*/
#define SetVertexAttribPointer(index, cls, mbr) SetVertexAttribPointerI( \
  index, \
  sizeof(cls::mbr) / sizeof(float), \
  sizeof(cls), \
  reinterpret_cast<GLvoid*>(offsetof(cls, mbr)))

	void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
	}

	/**
	* Vertex Array Objectを作成する.
	*
	* @param vbo VAOに関連付けられるVBO.
	* @param ibo VAOに関連付けられるIBO.
	*
	* @return 作成したVAO.
	*/
	GLuint CreateVAO(GLuint vbo, GLuint ibo)
	{
		GLuint vao = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		SetVertexAttribPointer(0, Vertex, position);
		SetVertexAttribPointer(1, Vertex, color);
		SetVertexAttribPointer(2, Vertex, texCoord);
		glBindVertexArray(0);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
		return vao;
	}

} // unnamed namespace

/**
* ゲームエンジンのインスタンスを取得する.
*
* @return ゲームエンジンのインスタンス.
*/
GameEngine& GameEngine::Instance()
{
	static GameEngine instance;
	return instance;
}

/**
* ゲームエンジンを初期化する.
*
* @param w     ウィンドウの描画範囲の幅(ピクセル).
* @param h     ウィンドウの描画範囲の高さ(ピクセル).
* @param title ウィンドウタイトル(UTF-8の0終端文字列).
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*
* Run関数を呼び出す前に、一度だけ呼び出しておく必要がある.
* 一度初期化に成功すると、以後の呼び出しではなにもせずにtrueを返す.
*/
bool GameEngine::Init(int w, int h, const char* title)
{
	if (isInitialized) {
		return true;
	}
	if (!GLFWEW::Window::Instance().Init(w, h, title)) {
		return false;
	}

	width = w;
	height = h;
	vbo = CreateVBO(sizeof(vertices), vertices);
	ibo = CreateIBO(sizeof(indices), indices);
	vao = CreateVAO(vbo, ibo);
	uboLight = UniformBuffer::Create(sizeof(InterfaceBlock::LightData), InterfaceBlock::BINDINGPOINT_LIGHTDATA, "LightData");
	uboPostEffect = UniformBuffer::Create(sizeof(InterfaceBlock::PostEffectData), InterfaceBlock::BINDINGPOINT_POSTEFFECTDATA, "PostEffectData");
	progPosterization = Shader::Program::Create("Res/Shader/Posterization.vert", "Res/Shader/Posterization.frag");
	//offscreen = OffscreenBuffer::Create(width, height);
	offscreen = OffscreenBuffer::Create(width, height, GL_RGBA16F);
	if (!vbo || !ibo || !vao || !uboLight || !uboPostEffect || !offscreen || !progPosterization) {
		std::cerr << "ERROR: GameEngineの初期化に失敗" << std::endl;
		return false;
	}
	// 縮小バッファの作成
	for (int i = 0, scale = 4; i < bloomBufferCount; ++i, scale *= 4) {
		const int w = width / scale;
		const int h = height / scale;
		offBloom[i] = OffscreenBuffer::Create(w, h, GL_RGBA16F);
		if (!offBloom[i]) {
			return false;
		}
	}

	// オフスクリーンバッファの大きさを取得.
	int offWidth, offHeight;
	glBindTexture(GL_TEXTURE_2D, offBloom[bloomBufferCount - 1]->GetTexutre());
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &offWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &offHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	// PBOを作成.
	const int pboByteSize = offWidth * offHeight * sizeof(GLfloat) * 4;
	for (auto& e : pbo) {
		e.Init(GL_PIXEL_PACK_BUFFER, pboByteSize, nullptr, GL_DYNAMIC_READ);
	}

	static const char* const shaderNameList[][3] = {
	{ "Tutorial", "Res/Shader/Tutorial.vert", "Res/Shader/Tutorial.frag" },
	{ "ColorFilter", "Res/Shader/ColorFilter.vert", "Res/Shader/ColorFilter.frag" },
	{ "NonLighting", "Res/Shader/NonLighting.vert", "Res/Shader/NonLighting.frag" },
	{ "HiLumExtract", "Res/Shader/TexCoord.vert", "Res/Shader/HiLumExtraction.frag" },
	{ "Shrink", "Res/Shader/TexCoord.vert", "Res/Shader/Shrink.frag" },
	{ "Blur3x3", "Res/Shader/TexCoord.vert", "Res/Shader/Blur3x3.frag" },
	{ "RenderDepth", "Res/Shader/RenderDepth.vert", "Res/Shader/RenderDepth.frag" },
	};
	shaderMap.reserve(sizeof(shaderNameList) / sizeof(shaderNameList[0]));
	for (auto& e : shaderNameList) {
		Shader::ProgramPtr program = Shader::Program::Create(e[1], e[2]);
		if (!program) {
			return false;
		}
		shaderMap.insert(std::make_pair(std::string(e[0]), program));
	}

	meshBuffer = Mesh::Buffer::Create(60 * 1024, 60 * 1024);
	if (!meshBuffer) {
		std::cerr << "ERROR: GameEngineの初期化に失敗" << std::endl;
		return false;
	}
	textureStack.push_back(TextureLevel());

	entityBuffer = Entity::Buffer::Create(1024, sizeof(InterfaceBlock::VertexData), InterfaceBlock::BINDINGPOINT_VERTEXDATA, "VertexData");
	if (!entityBuffer) {
		std::cerr << "ERROR: GameEngineの初期化に失敗" << std::endl;
		return false;
	}

	offDepth = OffscreenBuffer::Create(2048, 2048, GL_DEPTH_COMPONENT16);
	if (!offDepth) {
		return false;
	}

	shaderMap["Tutorial"]->UniformBlockBinding(*entityBuffer->UniformBuffer());
	shaderMap["Tutorial"]->UniformBlockBinding(*uboLight);
	shaderMap["ColorFilter"]->UniformBlockBinding(*uboPostEffect);
	shaderMap["HiLumExtract"]->UniformBlockBinding(*uboPostEffect);

	rand.seed(std::random_device()());
	fontRenderer.Init(1024, glm::vec2(static_cast<float>(w), static_cast<float>(h)), 32);
	isInitialized = true;
	return true;
}

/**
* ゲームを実行する.
*/
void GameEngine::Run()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	double prevTime = glfwGetTime();
	while (!window.ShouldClose()) {
		// デバッグ中など特殊な状況でなければありえないと考えられるしきい値.
		static const double thresholdToInvalidate = 0.25f;

		const double curTime = glfwGetTime();
		double delta = curTime - prevTime;
		if (delta >= thresholdToInvalidate) {
			delta = 1.0 / 60.0;
		}
		prevTime = curTime;

		Update(delta);
		Render();
		window.SwapBuffers();
		// 書き込み用PBOを切り替える.
		if (pboIndexForWriting < 0) {
			pboIndexForWriting = 0;
		}
		else {
			pboIndexForWriting ^= 1;
		}
	}
}

/**
* 状態更新関数を設定する.
*
* @param func 設定する更新関数.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func)
{
	updateFunc = func;
}

/**
* 状態更新関数を取得する.
*
* @return 設定されている更新関数.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const
{
	return updateFunc;
}

/**
* テクスチャを読み込む.
*
* @param filename テクスチャファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool GameEngine::LoadTextureFromFile(const char* filename)
{
	if (GetTexture(filename)) {
		return true;
	}
	TexturePtr texture = Texture::LoadFromFile(filename);
	if (!texture) {
		return false;
	}
	//textureBuffer.insert(std::make_pair(std::string(filename), texture));
	textureStack.back().insert(std::make_pair(std::string(filename), texture));
	return true;
}

/**
* テクスチャを取得する.
*
* @param filename テクスチャファイル名.
*
* @return filenameに対応するテクスチャへのポインタ.
*         対応するテクスチャが見つからない場合はnullptrを返す.
*/
const TexturePtr& GameEngine::GetTexture(const char* filename) const
{
	/*const auto itr = textureBuffer.find(filename);
	if (itr != textureBuffer.end()) {
		return itr->second;
	}*/
	for (const auto& e : textureStack) {
		const auto itr = e.find(filename);
		if (itr != e.end()) {
			return itr->second;
		}
	}

	static const TexturePtr dummy;
	return dummy;
}

/**
* メッシュを読み込む.
*
* @param filename メッシュファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool GameEngine::LoadMeshFromFile(const char* filename)
{
	return meshBuffer->LoadMeshFromFile(filename);
}

/**
* エンティティを追加する.
*
* @param groupId  エンティティのグループID.
* @param pos      エンティティの座標.
* @param meshName エンティティの表示に使用するメッシュ名.
* @param texName  エンティティの表示に使うテクスチャファイル名.
* @param func     エンティティの状態を更新する関数(または関数オブジェクト).
* @param shader   エンティティの表示に使うシェーダ名.
*
* @return 追加したエンティティへのポインタ.
*         これ以上エンティティを追加できない場合はnullptrが返される.
*         回転や拡大率はこのポインタ経由で設定する.
*         なお、このポインタをアプリケーション側で保持する必要はない.
*/
Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName, const char* normalName,
	Entity::Entity::UpdateFuncType func, const char* shader)
{
	decltype(shaderMap)::const_iterator itr = shaderMap.end();
	if (shader) {
		itr = shaderMap.find(shader);
	}
	if (itr == shaderMap.end()) {
		itr = shaderMap.find("Tutorial");
		if (itr == shaderMap.end()) {
			return nullptr;
		}
	}

	const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
	//const TexturePtr& tex = textureBuffer.find(texName)->second;
	TexturePtr tex[2];
	tex[0] = GetTexture(texName);
	if (normalName) {
		tex[1] = GetTexture(normalName);
	}
	else {
		tex[1] = GetTexture("Res/Model/Dummy.Normal.bmp");
	}
	return entityBuffer->AddEntity(groupId, pos, mesh, tex, itr->second, func);
}

Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName,
	Entity::Entity::UpdateFuncType func, const char* shader)
{
	return AddEntity(groupId, pos, meshName, texName, nullptr, func, shader);
}

/**
*　エンティティを削除する.
*
* @param 削除するエンティティのポインタ.
*/
void GameEngine::RemoveEntity(Entity::Entity* e)
{
	entityBuffer->RemoveEntity(e);
}

/**
* 全てのエンティティを削除する.
*/
void GameEngine::RemoveAllEntity()
{
	entityBuffer->RemoveAllEntity();
}

/**
* ライトを設定する.
*
* @param indes  設定するライトのインデックス.
* @param light  ライトデータ.
*/
void GameEngine::Light(int index, const InterfaceBlock::PointLight& light)
{
	if (index < 0 || index >= InterfaceBlock::maxLightCount) {
		std::cerr << "WARNING: '" << index << "'は不正なライトインデックスです" << std::endl;
		return;
	}
	lightData.light[index] = light;
}

/**
* ライトを取得する.
*
* @param index 取得するライトのインデックス.
*
* @return ライトデータ.
*/
const InterfaceBlock::PointLight& GameEngine::Light(int index) const
{
	if (index < 0 || index >= InterfaceBlock::maxLightCount) {
		std::cerr << "WARNING: '" << index << "'は不正なライトインデックスです" << std::endl;
		static const InterfaceBlock::PointLight dummy;
		return dummy;
	}
	return lightData.light[index];
}

/**
* 環境光を設定する.
*
* @param color 環境光の明るさ.
*/
void GameEngine::AmbientLight(const glm::vec4& color)
{
	lightData.ambientColor = color;
}

/**
* 環境光を取得する.
*
* @return 環境光の明るさ.
*/
const glm::vec4& GameEngine::AmbientLight() const
{
	return lightData.ambientColor;
}

/**
* 視点の位置と姿勢を設定する.
*
* @param index カメラのインデックス.
* @param cam 設定するカメラデータ.
*/
//void GameEngine::Camera(const CameraData& cam)
void GameEngine::Camera(size_t index, const CameraData& cam)
{
	//camera = cam;
	camera[index] = cam;
	lightData.eyePos[index] = glm::vec4(cam.position, 0);
}

/**
* 視点の位置と姿勢を取得する.
*
* @param index カメラのインデックス.
* @return カメラデータ.
*/
//const GameEngine::CameraData& GameEngine::Camera() const
const GameEngine::CameraData& GameEngine::Camera(size_t index) const
{
	//return camera;
	return camera[index];
}

/**
* 乱数オブジェクトを取得する.
*
* @return 乱数オブジェクト.
*/
std::mt19937& GameEngine::Rand()
{
	return rand;
}

/**
* ゲームパッドの状態を取得する.
*/
const GamePad& GameEngine::GetGamePad() const
{
	return GLFWEW::Window::Instance().GetGamePad();
}

/**
* 衝突解決ハンドラを設定する.
*
* @param gid0    衝突対象のグループID.
* @param gid1    衝突対象のグループID.
* @param handler 衝突解決ハンドラ.
*
* 衝突が発生し衝突解決ハンドラが呼びされるとき、より小さいグループIDを持つエンティティから先に渡される.
* ここで指定したグループIDの順序とは無関係であることに注意すること.
* ex)
*   CollisionHandler(10, 1, Func)
*   というコードでハンドラを登録したとする. 衝突が発生すると、
*   Func(グループID=1のエンティティ、グループID=10のエンティティ)
*   のように呼び出される.
*/
void GameEngine::CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler)
{
	entityBuffer->CollisionHandler(gid0, gid1, handler);
}

/**
* 衝突解決ハンドラを取得する.
*
* @param gid0 衝突対象のグループID.
* @param gid1 衝突対象のグループID.
*
* @return 衝突解決ハンドラ.
*/
const Entity::CollisionHandlerType& GameEngine::CollisionHandler(int gid0, int gid1) const
{
	return entityBuffer->CollisionHandler(gid0, gid1);
}

/**
* 衝突解決ハンドラのリストをクリアする.
*/
void GameEngine::ClearCollisionHandlerList()
{
	entityBuffer->ClearCollisionHandlerList();
}

/**
* @copydoc Audio::Initialize
*/
bool GameEngine::InitAudio(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName, size_t playerCount)
{
	return Audio::Initialize(acfPath, acbPath, awbPath, dspBusName, playerCount);
}

/**
* @copydoc Audio::Play
*/
void GameEngine::PlayAudio(int playerId, int cueId)
{
	Audio::Play(playerId, cueId);
}

/**
* @copydoc Audio::Stop
*/
void GameEngine::StopAudio(int playerId)
{
	Audio::Stop(playerId);
}

/**
* リソーススタックに新しいリソースレベルを作成する.
*/
void GameEngine::PushLevel()
{
	meshBuffer->PushLevel();
	textureStack.push_back(TextureLevel());
}

/**
* リソーススタックの末尾のリソースレベルを除去する.
*/
void GameEngine::PopLevel()
{
	meshBuffer->PopLevel();
	if (textureStack.size() > minimalStackSize) {
		textureStack.pop_back();
	}
}

/**
* リソーススタックの末尾のリソースレベルを空の状態にする.
*/
void GameEngine::ClearLevel()
{
	meshBuffer->ClearLevel();
	textureStack.back().clear();
}


/**
* デストラクタ.
*/
GameEngine::~GameEngine()
{
	updateFunc = nullptr;
	Audio::Destroy();
	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}
	if (ibo) {
		glDeleteBuffers(1, &ibo);
	}
	if (vbo) {
		glDeleteBuffers(1, &vbo);
	}
}

/**
* ゲームの状態を更新する.
*
* @param delta 前回の更新からの経過時間(秒).
*/
void GameEngine::Update(double delta)
{
	Audio::Update();
	GLFWEW::Window::Instance().UpdateGamePad();
	fontRenderer.MapBuffer();
	if (updateFunc) {
		updateFunc(delta);
	}
	//const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float> (height), 1.0f, 200.0f);
	const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float> (height), 1.0f, 1000.0f);
	//const glm::mat4x4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	glm::mat4x4 matView[InterfaceBlock::maxViewCount];
	for (int i = 0; i < InterfaceBlock::maxViewCount; ++i) {
		const CameraData& cam = camera[i];
		matView[i] = glm::lookAt(cam.position, cam.target, cam.up);
	}

	const glm::vec2 range = shadowParameter.range * 0.5f;
	const glm::mat4 matDepthProj = glm::ortho<float>(-range.x, range.x, -range.y, range.y, shadowParameter.near, shadowParameter.far);
	const glm::mat4 matDepthView = glm::lookAt(shadowParameter.lightPos, shadowParameter.lightPos + shadowParameter.lightDir, shadowParameter.lightUp);

	//entityBuffer->Update(delta, matView, matProj);
	entityBuffer->Update(delta, matView, matProj, matDepthProj * matDepthView);
	fontRenderer.UnmapBuffer();
}

/**
* デプスシャドウマップを描画する.
*/
void GameEngine::RenderShadow() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, offDepth->GetFramebuffer());
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glViewport(0, 0, offDepth->Width(), offDepth->Height());
	glScissor(0, 0, offDepth->Width(), offDepth->Height());
	glClearDepth(1);
	glClear(GL_DEPTH_BUFFER_BIT);

	const Shader::ProgramPtr& progDepth = shaderMap.find("RenderDepth")->second;
	progDepth->UseProgram();
	entityBuffer->DrawDepth(meshBuffer);
}

/**
* ゲームの状態を描画する.
*/
void GameEngine::Render() const
{
	RenderShadow();
	glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderMap.find("Tutorial")->second->BindShadowTexture(GL_TEXTURE_2D, offDepth->GetTexutre());
	uboLight->BufferSubData(&lightData);
	entityBuffer->Draw(meshBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(vao);

#if 0	
	// ポスター化シェーダーの利用
	progPosterization->UseProgram();
	progPosterization->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

#else
	const Shader::ProgramPtr& progHiLumExtract = shaderMap.find("HiLumExtract")->second;
	progHiLumExtract->UseProgram();
	glBindFramebuffer(GL_FRAMEBUFFER, offBloom[0]->GetFramebuffer());
	//glViewport(0, 0, width, height);
	glViewport(0, 0, offBloom[0]->Width(), offBloom[0]->Height());
	progHiLumExtract->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
	glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);

	const Shader::ProgramPtr& progShrink = shaderMap.find("Shrink")->second;
	progShrink->UseProgram();
	/*for (int i = 1, scale = 4 * 4; i < bloomBufferCount; ++i, scale *= 4) {
		glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i]->GetFramebuffer());
		glViewport(0, 0, width / scale, height / scale);
		progShrink->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i - 1]->GetTexutre());
		glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
	}*/
	for (int i = 1; i < bloomBufferCount; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i]->GetFramebuffer());
		glViewport(0, 0, offBloom[i]->Width(), offBloom[i]->Height());
		progShrink->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i - 1]->GetTexutre());
		glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	const Shader::ProgramPtr& progBlur3x3 = shaderMap.find("Blur3x3")->second;
	progBlur3x3->UseProgram();
	/*for (int i = bloomBufferCount - 1, scale = 4 * 4 * 4 * 4; i > 0; --i, scale /= 4) {
		glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i - 1]->GetFramebuffer());
		glViewport(0, 0, width / scale, height / scale);
		progBlur3x3->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i]->GetTexutre());
		glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
	}*/
	for (int i = bloomBufferCount - 1; i > 0; --i) {
		glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i - 1]->GetFramebuffer());
		glViewport(0, 0, offBloom[i - 1]->Width(), offBloom[i - 1]->Height());
		progBlur3x3->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i]->GetTexutre());
		glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
	}

	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	const Shader::ProgramPtr& progColorFilter = shaderMap.find("ColorFilter")->second;
	progColorFilter->UseProgram();
	InterfaceBlock::PostEffectData postEffect;

	postEffect.luminanceScale = luminanceScale;
	postEffect.bloomThreshold = 1.0f / luminanceScale;
	// 初期
	postEffect.matColor = glm::mat4x4(1);
	//// セピア調
	//postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
	//postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
	//postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
	//postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
	//// モノトーン調
	//postEffect.matColor[0] = glm::vec4(0.299f, 0.299f, 0.299f, 0);
	//postEffect.matColor[1] = glm::vec4(0.587f, 0.587f, 0.587f, 0);
	//postEffect.matColor[2] = glm::vec4(0.114f, 0.114f, 0.114f, 0);
	//postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
	//// ネガポジ反転
	//postEffect.matColor[0] = glm::vec4(-1, 0, 0, 0);
	//postEffect.matColor[1] = glm::vec4(0, -1, 0, 0);
	//postEffect.matColor[2] = glm::vec4(0, 0, -1, 0);
	//postEffect.matColor[3] = glm::vec4(1, 1, 1, 1);
	uboPostEffect->BufferSubData(&postEffect);
	progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
	progColorFilter->BindTexture(GL_TEXTURE1, GL_TEXTURE_2D, offBloom[0]->GetTexutre());

#endif
	glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
	fontRenderer.Draw();

	{
		// オフスクリーンバッファの大きさを取得.
		int width, height;
		glBindTexture(GL_TEXTURE_2D, offBloom[bloomBufferCount - 1]->GetTexutre());
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		glBindTexture(GL_TEXTURE_2D, 0);

		// 初回(pboIndexForWriting<0の場合)はまだデータがないため、
		// 変換コピーするだけで輝度計算はしない.
		if (pboIndexForWriting < 0) {
			// オフスクリーンバッファの内容をPBOに変換コピー.
			const GLuint pboWrite = pbo[1].Id();
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pboWrite);
			glBindFramebuffer(GL_FRAMEBUFFER, offBloom[bloomBufferCount - 1]->GetFramebuffer());
			glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, 0);
		}
		else {
			// オフスクリーンバッファの内容をPBOに変換コピー.
			const GLuint pboWrite = pbo[pboIndexForWriting].Id();
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pboWrite);
			glBindFramebuffer(GL_FRAMEBUFFER, offBloom[bloomBufferCount - 1]->GetFramebuffer());
			glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, 0);

			// PBOの内容を読み取って輝度スケールを計算.
			const GLuint pboRead = pbo[pboIndexForWriting ^ 1].Id();
			glBindBuffer(GL_PIXEL_PACK_BUFFER, pboRead);
			const GLfloat* p = static_cast<GLfloat*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
			float totalLum = 0;
			for (int i = 0; i < width * height; ++i) {
				totalLum += p[i * 4 + 3];
			}
			//luminanceScale = 0.18f / std::exp(totalLum / static_cast<float>(width * height));
			luminanceScale = keyValue / std::exp(totalLum / static_cast<float>(width * height));

			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
}
