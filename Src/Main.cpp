/**
* @file Main.cpp
*/
#include "GameEngine.h"
#include "GameState.h"
#include "../Res/Audio/Tutorial/TutorialProject_acf.h"
#include "../Res/Audio/Tutorial/TutorialCueSheet.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>


int main()
{
	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}
	if (!game.InitAudio("Res/Audio/Tutorial/TutorialProject.acf", "Res/Audio/Tutorial/TutorialCueSheet.acb", "Res/Audio/Tutorial/TutorialCueSheet.awb",
		CRI_TUTORIALPROJECT_ACF_DSPSETTING_DSPBUSSETTING_0, GameState::AudioPlayerId_Max)) {
		return 1;
	}

	//game.LoadTextureFromFile("Res/Model/Toroid.bmp");
	/*game.LoadTextureFromFile("Res/Model/Toroid.dds");
	game.LoadTextureFromFile("Res/Model/Toroid.Normal.bmp");*/
	game.LoadTextureFromFile("Res/Model/Dummy.Normal.bmp");
	/*game.LoadTextureFromFile("Res/Model/Player.bmp");
	game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");

	game.LoadMeshFromFile("Res/Model/ToroidN.fbx");
	game.LoadMeshFromFile("Res/Model/Player.fbx");
	game.LoadMeshFromFile("Res/Model/Blast.fbx");
	game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");*/
	game.LoadFontFromFile("Res/Font/Font.fnt");

	game.PushLevel();

	game.UpdateFunc(GameState::Title());
	game.Run();
	return 0;
}