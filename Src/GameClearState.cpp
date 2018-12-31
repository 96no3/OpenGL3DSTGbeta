/**
* @file GameClearState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/Tutorial/TutorialCueSheet.h"

namespace GameState {

	/**
	* 背景球を更新する.
	*/
	static void UpdateSpaceSphere(Entity::Entity& entity, double delta)
	{
		glm::vec3 rotSpace = glm::eulerAngles(entity.Rotation());
		rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
		entity.Rotation(rotSpace);
	}

	/**
	* ゲームクリア画面のコンストラクタ.
	*/
	GameClear::GameClear() {
		GameEngine& game = GameEngine::Instance();
		game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_CLEAR);
	}

	/// ゲームオーバー画面の更新.
	void GameClear::operator()(double delta)
	{
		GameEngine& game = GameEngine::Instance();
		game.Camera(0,{ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
		game.KeyValue(0.02f);

		if (initial) {
			initial = false;
			game.RemoveAllEntity();
			game.ClearLevel();
			game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
			game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");
			game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", &UpdateSpaceSphere, "NonLighting");
		}
		
		const float offset = timer == 0 ? 0 : (2.0f - timer) * (2.0f - timer) * 2.0f * 400.0f;
		game.FontScale(glm::vec2(2.0f, 2.0f));
		game.FontColor(glm::vec4(1.0f, 0, 0, 1.0f));
		game.AddString(glm::vec2(300.0f + offset, 260.0f), "Game Clear!!");
		game.FontScale(glm::vec2(0.5f, 0.5f));
		game.FontColor(glm::vec4(0.75f, 0.75f, 0.75f, 1.0f));
		game.AddString(glm::vec2(440.0f + offset, 328.0f), "Thank You For Playing");
		if (timer > 0) {
			timer -= static_cast<float>(delta);
			if (timer <= 0) {
				game.StopAudio(AudioPlayerId_BGM);
				game.UpdateFunc(Title());
			}
		}
		else if (game.GetGamePad().buttonDown & GamePad::START) {
			game.PlayAudio(AudioPlayerId_UI, CRI_TUTORIALCUESHEET_START);
			timer = 2;
		}
	}

}
