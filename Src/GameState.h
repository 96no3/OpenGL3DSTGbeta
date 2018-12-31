/**
* @file GameState.h
*/
#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED
#include "Entity.h"

namespace GameState {

	/// �G���e�B�e�B�̏Փ˃O���[�vID.
	enum EntityGroupId
	{
		EntityGroupId_Background,
		EntityGroupId_Player,
		EntityGroupId_PlayerShot,
		EntityGroupId_Enemy,
		EntityGroupId_EnemyShot,
		EntityGroupId_Others
	};

	/// �����v���C���[��ID.
	enum AudioPlayerId
	{
		AudioPlayerId_Shot, ///< ���@�̃V���b�g��.
		AudioPlayerId_Bomb, ///< ������.
		AudioPlayerId_BGM, ///<�@BGM.
		AudioPlayerId_Max, ///< ���������\�Ȑ�.

		AudioPlayerId_UI = AudioPlayerId_Shot, ///<�@���[�U�[�C���^�[�t�F�C�X���쉹.
	};

	/*
	* �^�C�g�����.
	*/
	class Title
	{
	public:
		//explicit Title(Entity::Entity* p = nullptr);
		Title();
		void operator()(double delta);
	private:
		//Entity::Entity* pSpaceSphere = nullptr;
		bool initial = true;
		float timer = 0;
	};

	/**
	* ���C���Q�[�����.
	*/
	class MainGame
	{
	public:
		//explicit MainGame(Entity::Entity* p);
		MainGame();
		void operator()(double delta);
	private:
		//bool isInitialized = false;
		int enemyLevel = 0;
		double interval = 0;
		Entity::Entity* pPlayer = nullptr;
		//Entity::Entity* pSpaceSphere = nullptr;
		int stageNo = 0;
		double stageTimer = -1;

		bool initial = true;
		float timer = 0;
	};

	/*
	* �Q�[���N���A���.
	*/
	class GameClear
	{
	public:
		GameClear();
		void operator()(double delta);
	private:
		bool initial = true;
		float timer = 0;
	};
}

#endif