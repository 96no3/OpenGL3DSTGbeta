/**
* @file MainGameState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/Tutorial/TutorialCueSheet.h"
#include <algorithm>

namespace GameState {

	/// 衝突形状リスト.
	static const Entity::CollisionData collisionDataList[] = {
	  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
	  { glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
	  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
	  { glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
	};
	
	/**
	* 敵の弾の更新.
	*/
	struct UpdateEnemyShot
	{
		void operator()(Entity::Entity& entity, double delta)
		{
			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
				entity.Destroy();
				return;
			}
		}
	};

	/**
	* 移動目標に追いつく位置を計算する.
	*
	* @param follower       追従開始座標.
	* @param followingSpeed 追従速度.
	* @param target         目標の現在位置.
	* @param targetVelocity 目標の移動速度.
	*
	* @return 追いつくことができればその位置、できなければtargetを返す.
	*/
	glm::vec3 CalcCatchUpPosition(const glm::vec3& follower, const float followingSpeed, const glm::vec3& target, const glm::vec3& targetVelocity)
	{
		// V0x*t + P0x = V1x*t + P1x
		//   V0x = V1x + (P1x - P0x)/t
		// V0y*t + P0y = V1y*t + P1y
		//   (V0y - V1y)t = P1y - P0y
		//   t = (P1y - P0y)/(V0y - V1y)
		//   V0x = V1x + (P1x - P0x)/(P1y - P0y)(V0y - V1y)
		//
		// V0^2 = V0x^2 + V0y^2
		//   V0x^2 = V0^2 - V0y^2
		//   V0x=sqrt(V0^2 - V0y^2)
		//
		// sqrt(V0^2 - V0y^2) = V1x + (P1x - P0x)/(P1y - P0y)(V0y - V1y)
		// V0^2 - V0y^2 = (V1x + (P1x - P0x)/(P1y - P0y)(V0y - V1y))^2
		//            = (V1x - N*V1y + N*V0y)^2
		//            = (VN + N*V0y)^2
		//            = VN^2 + 2(VN*N*V0y) + (N*V0y)^2
		//
		// 0 = (N*V0y)^2 + 2(VN*N*V0y) + VN^2 - V0^2 + V0y^2
		// 0 = (N^2+1)V0y^2 + (2*VN*N)V0y + (VN^2 - V0^2)
		//
		const glm::vec3 P0 = follower;
		const glm::vec3 P1 = target;
		const glm::vec3 V1 = targetVelocity;
		const float V0 = followingSpeed;
		const float N = (P1.x - P0.x) / (P1.z - P0.z);
		const float VN = V1.x - N * V1.z;
		const float a = N * N + 1;
		const float b = 2 * VN * N;
		const float c = VN * VN - V0 * V0;
		const float D = b * b - 4 * a * c;
		glm::vec3 targetPos = P1;
		if (D >= 0) {
			const float sq = std::sqrt(D);
			const float t0 = (P1.z - P0.z) / ((-b + sq) / (2 * a) - V1.z);
			const float t1 = (P1.z - P0.z) / ((-b - sq) / (2 * a) - V1.z);
			const float t = std::max(t0, t1);
			targetPos += V1 * t;
		}
		return targetPos;
	}

	/**
	* 敵の円盤の状態を更新する.
	*/
	struct UpdateToroid
	{
		//explicit UpdateToroid(const Entity::Entity* t) : target(t) {}
		explicit UpdateToroid(const Entity::Entity* t, int level = 0) : target(t)
		{
			GameEngine& game = GameEngine::Instance();
			shotInterval = std::max(0.1, 1.0 - (level % 20) * 0.05);
			shotCount = std::min(5, level / 20 + 1);
		}

		void operator()(Entity::Entity& entity, double delta)
		{
			GameEngine& game = GameEngine::Instance();

			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
				GameEngine::Instance().RemoveEntity(&entity);
				return;
			}

			if (game.Variable("lank") > 10) {
				const float V0 = 16.0f;
				if (isEscape || (pos.z < 35.0f && std::abs(pos.x - target->Position().x) <= 3.0f)) {
					isEscape = true;
					bool doShot = false;
					glm::vec3 v = entity.Velocity();
					if (accelX) {
						v.x += accelX * static_cast<float>(delta);
						entity.Velocity(v);
						shotTimer -= delta;
						if (shotTimer <= 0) {
							shotTimer = shotInterval;
							doShot = true;
						}
					}
					else {
						accelX = v.x * -8.0f;
						shotTimer = shotInterval;
						doShot = true;
					}
					entity.Velocity(v);
					glm::quat q = glm::rotate(glm::quat(), -accelX * 0.2f * static_cast<float>(delta), glm::vec3(0, 0, 1));
					entity.Rotation(q * entity.Rotation());

					const glm::bvec3 flags = glm::lessThan(pos, glm::vec3(12, 100, 40)) && glm::greaterThan(pos, glm::vec3(-12, -100, 0));
					if (doShot && glm::all(flags)) {
						glm::vec3 targetPos = CalcCatchUpPosition(entity.Position(), V0, target->Position(), target->Velocity());
						targetPos.x += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
						targetPos.z += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
						targetPos = glm::min(glm::vec3(11, 100, 20), glm::max(targetPos, glm::vec3(-11, -100, 1)));
						const glm::vec3 velocity = glm::normalize(targetPos - entity.Position()) * V0;
						static const float rotList[][2] = {
						  { glm::radians(0.0f), glm::radians(0.0f) },
						  { glm::radians(-15.0f), glm::radians(30.0f) },
						  { glm::radians(-15.0f), glm::radians(15.0f) },
						  { glm::radians(-30.0f), glm::radians(20.0f) },
						  { glm::radians(-30.0f), glm::radians(15.0f) },
						  { glm::radians(-30.0f), glm::radians(15.0f) },
						};
						float rot = rotList[shotCount - 1][0];
						for (int i = 0; i < shotCount; ++i) {
							if (Entity::Entity* p = game.AddEntity(EntityGroupId_EnemyShot, pos, "Spario", "Res/Model/Toroid.dds", UpdateEnemyShot())) {
								p->Velocity(glm::rotate(glm::quat(glm::vec3(0, rot, 0)), velocity));
								p->Color(glm::vec4(3, 3, 3, 1));
								p->Collision(collisionDataList[EntityGroupId_EnemyShot]);
							}
							rot += rotList[shotCount - 1][1];
						}
					}
				}
				else {
					float rot = glm::angle(entity.Rotation());
					rot += glm::radians(120.0f) * static_cast<float>(delta);
					if (rot > glm::pi<float>() * 2.0f) {
						rot -= glm::pi<float>() * 2.0f;
					}
					entity.Rotation(glm::angleAxis(rot, glm::vec3(0.1, 1, 0)));
				}
			}
			else {
				// 一定時間ごとに弾を発射.
				/*std::uniform_real_distribution<> rndShotInterval(0.8, 2.0);
				shotInterval = rndShotInterval(game.Rand());*/
				shotInterval = 1.5;
				shotTimer += delta;
				if (shotTimer > shotInterval) {

					glm::vec3 shotPos = entity.Position();
					shotPos.x -= 0.4f; // 敵機の中心から左に0.4ずらした位置が1つめの発射点.
					for (int i = 0; i < 2; ++i) {

						// 画面外にいるときは発射しない.
						if (std::abs(pos.x) > 13 || pos.z > 41 || pos.z < -1) {
							continue;
						}

						// 自機の方向を計算.
						const glm::vec3 targetPos = target->Position();
						const glm::vec3 distance = targetPos - pos;
						const float radian = std::atan2(distance.x, distance.z);
						const float c = std::cos(radian);
						const float s = std::sin(radian);

						if (Entity::Entity* p = game.AddEntity(EntityGroupId_EnemyShot, shotPos, "Spario", "Res/Model/Toroid.dds", UpdateEnemyShot()))
						{
							p->Velocity(glm::vec3(20 * s, 0, 20 * c));
							p->Collision(collisionDataList[EntityGroupId_EnemyShot]);
							//p->Color(glm::vec4(1.0f,1.0f, 1.0f, 1.0f) * 1.5f);
							p->Color(glm::vec4(glm::vec3(1.0f, 1.0f, 1.0f) * 3.0f, 1.0f));
						}
						shotPos.x += 0.8f; // 中心からに右に0.4ずらした位置が2つめの発射点.
						game.PlayAudio(AudioPlayerId_Shot, CRI_TUTORIALCUESHEET_WEAPON_ENEMY);
					}
					shotTimer -= shotInterval;
				}

				// 円盤を回転させる.
				float rot = glm::angle(entity.Rotation());
				rot += glm::radians(60.0f) * static_cast<float>(delta);
				if (rot > glm::pi<float>() * 2.0f) {
					rot -= glm::pi<float>() * 2.0f;
				}
				entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));

				//// スケールを変化させる.
				//static std::mt19937 rand(std::random_device{}());

				//const int waitTime = 3600;
				//time = (time + 1) % waitTime;
				//if (time == 0) {
				//	const std::uniform_real_distribution<float> scaleRange(0.5, 2.0);
				//	scale = scaleRange(rand);
				//}
				//entity.Scale(glm::vec3(scale));
			}
		}

	private:
		const Entity::Entity* target;
		double shotTimer = 0;
		//int time = 0;
		//float scale = 1;

		bool isEscape = false;
		float accelX = 0;
		double shotInterval;
		int shotCount;
	};

	/**
	* 自機の弾の更新.
	*/
	struct UpdatePlayerShot
	{
		void operator()(Entity::Entity& entity, double delta)
		{
			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
				entity.Destroy();
				return;
			}
		}
	};

	/**
	* 爆発の更新.
	*/
	struct UpdateBlast
	{
		void operator()(Entity::Entity& entity, double delta) {
			timer += delta;
			if (timer >= 0.5) {
				entity.Destroy();
				return;
			}
			const float variation = static_cast<float>(timer * 4); // 変化量.
			entity.Scale(glm::vec3(static_cast<float>(1 + variation))); // 徐々に拡大する.
			// 時間経過で色と透明度を変化させる.
			static const float lumScale = 2;
			static const glm::vec4 color[] = {
			  glm::vec4(glm::vec3(1.0f, 1.0f, 0.75f) * lumScale, 1),
			  glm::vec4(glm::vec3(1.0f, 0.5f, 0.1f) * lumScale, 1),
			  glm::vec4(glm::vec3(0.25f, 0.1f, 0.1f) * lumScale, 0),
			};
			const glm::vec4 col0 = color[static_cast<int>(variation)];
			const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
			const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
			entity.Color(newColor);
			// Y軸回転させる.
			glm::vec3 euler = glm::eulerAngles(entity.Rotation());
			euler.y += glm::radians(60.0f) * static_cast<float>(delta);
			entity.Rotation(glm::quat(euler));
		}

		double timer = 0;
	};

	/**
	* 自機の更新.
	*/
	struct UpdatePlayer
	{
		void operator()(Entity::Entity& entity, double delta)
		{
			GameEngine& game = GameEngine::Instance();
			const GamePad gamepad = game.GetGamePad();

			if (entity.invincible) {
				// 自機を点滅させる
				float Alpha = 0.5f + 0.5f * (float)sin(count * 0.7f);
				count++;
				invincibleTimer -= delta;
				if (invincibleTimer <= 0) {
					entity.invincible = false;
					invincibleTimer = 5;
					Alpha = 1.0f;
					count = 0;
				}
				entity.Color(glm::vec4(1.0f, 1.0f, 1.0f, Alpha));
			}

			if (!entity.GetIsActive()) {
				glm::vec3 vec = glm::vec3(0, 0, 0);
				glm::vec3 pos = entity.Position();
				if (restartTimer >= 2.5) {
					vec.z -= 2;
					vec.y -= 4;
				}
				else if (restartTimer >= 2) {
					pos = glm::vec3(0, 0, -2);
				}
				else {
					vec.z += 2;
				}
				restartTimer -= delta;
				if (restartTimer <= 0) {
					entity.SetIsActive(true);
					restartTimer = 3;
				}
				entity.Velocity(vec);
				entity.Position(pos);
				entity.Rotation(glm::quat());
			}
			else {
				glm::vec3 vec = glm::vec3(0, 0, 0);

				float rotZ = 0;
				if (gamepad.buttons & GamePad::DPAD_LEFT) {
					vec.x = 1;
					rotZ = -glm::radians(30.0f);
				}
				else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
					vec.x = -1;
					rotZ = glm::radians(30.0f);
				}
				if (gamepad.buttons & GamePad::DPAD_UP) {
					vec.z = 1;
				}
				else if (gamepad.buttons & GamePad::DPAD_DOWN) {
					vec.z = -1;
				}
				if (vec.x || vec.z) {
					vec = glm::normalize(vec) * 15.0f;
				}
				entity.Velocity(vec);
				entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));
				glm::vec3 pos = entity.Position();
				pos = glm::min(glm::vec3(11, 100, 30), glm::max(pos, glm::vec3(-11, -100, 1)));
				entity.Position(pos);

				if (gamepad.buttons & GamePad::A) {
					shotInterval -= delta;
					if (shotInterval <= 0) {
						glm::vec3 pos = entity.Position();
						pos.x -= 0.9f;	// 自機の中心から左に0.9ずらした位置が1つめの発射点.
						for (int i = 0; i < 2; ++i) {
							if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot())) {
								p->Velocity(glm::vec3(0, 0, 80));
								p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
								p->Color(glm::vec4(glm::vec3(1.0f, 1.0f, 1.0f) * 3.0f, 1.0f));
							}
							pos.x += 1.8f; // 中心からに右に0.9ずらした位置が2つめの発射点.
						}
						shotInterval += 0.25; // 秒間4連射.
						game.PlayAudio(AudioPlayerId_Shot, CRI_TUTORIALCUESHEET_WEAPON_PLAYER);
					}
				}
				else {
					shotInterval = 0;
				}
			}
		}

	private:
		double shotInterval = 0;
		double restartTimer = 3;
		double invincibleTimer = 5;
		int count = 0;
	};


	/**
	* 自機の弾と敵の衝突処理.
	*/
	void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
	{
		GameEngine& game = GameEngine::Instance();
		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			p->Color(glm::vec4(glm::vec3(1.0f, 0.75f, 0.5f) * 3.0f, 1.0f));
			game.Variable("score") += 100;
			if (game.Variable("score") == 1000 * game.Variable("check")) {
				game.Variable("lank")++;
				game.Variable("check")++;
			}
			if (game.Variable("lank") > 10) {
				game.Variable("Level") = std::min(100, (int)game.Variable("Level") + 1);
			}
		}
		game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_ENEMY);
		lhs.Destroy();
		rhs.Destroy();
	}

	/**
	* 自機と敵の衝突処理.
	*/
	void PlayerAndEnemyCollisionHandler(Entity::Entity& player, Entity::Entity& enemy)
	{
		GameEngine& game = GameEngine::Instance();

		if (player.invincible) {
			return;
		}

		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, enemy.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			p->Color(glm::vec4(glm::vec3(1.0f, 0.75f, 0.5f) * 3.0f, 1.0f));
		}
		enemy.Destroy();
		game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_ENEMY);

		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, player.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			p->Color(glm::vec4(glm::vec3(1.0f, 0.75f, 0.5f) * 3.0f, 1.0f));
		}
		game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_PLAYER);
		game.Variable("life")--;
		if (game.Variable("life") < 1) {
			player.Destroy();
		}
		else {
			player.SetIsActive(false);
			player.invincible = true;
		}
	}

	/**
	* 自機と敵の弾の衝突処理.
	*/
	void PlayerAndEnemyShotCollisionHandler(Entity::Entity& player, Entity::Entity& enemyshot)
	{
		GameEngine& game = GameEngine::Instance();

		if (player.invincible) {
			return;
		}

		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, player.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			p->Color(glm::vec4(glm::vec3(1.0f, 0.75f, 0.5f) * 3.0f, 1.0f));
		}
		game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_PLAYER);
		game.Variable("life")--;
		if (game.Variable("life") < 1) {
			player.Destroy();
		}
		else {
			player.SetIsActive(false);
			player.invincible = true;
		}
		enemyshot.Destroy();
	}

	/**
	* 背景球を更新する.
	*/
	static void UpdateSpaceSphere(Entity::Entity& entity, double delta)
	{
		glm::vec3 rotSpace = glm::eulerAngles(entity.Rotation());
		rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
		entity.Rotation(rotSpace);
	}

	void UpdateLandscape(Entity::Entity& entity, double delta)
	{
		entity.Position(entity.Position() + glm::vec3(0, 0, -4.0f * delta));
	}

	/**
	* メインゲーム画面のコンストラクタ.
	*/
	MainGame::MainGame()
	{
		GameEngine& game = GameEngine::Instance();
		game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);
		game.CollisionHandler(EntityGroupId_Player, EntityGroupId_Enemy, &PlayerAndEnemyCollisionHandler);
		game.CollisionHandler(EntityGroupId_Player, EntityGroupId_EnemyShot, &PlayerAndEnemyShotCollisionHandler);
		game.Variable("score") = 0;
		game.Variable("lank") = 1;
		game.Variable("life") = 3;
		game.Variable("check") = 1;
		game.Variable("Level") = 1;
		//game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_BATTLE);
	}

	/**
	* メインゲーム画面の更新.
	*/
	void MainGame::operator()(double delta)
	{
		GameEngine& game = GameEngine::Instance();

		static const float stageTime = 90;
		if (stageTimer < 0) {
			++stageNo;
			stageTimer = stageTime;

			//game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
			game.Camera(0, { glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
			game.Camera(1, { glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
			game.GroupVisibility(EntityGroupId_Background, 0, false);
			game.GroupVisibility(EntityGroupId_Background, 1, true);
			game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
			game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) });

			GameEngine::ShadowParameter shadowParam;
			shadowParam.lightPos = glm::vec3(20, 50, 50);
			shadowParam.lightDir = glm::normalize(glm::vec3(-25, -50, 25));
			shadowParam.lightUp = glm::vec3(0, 0, 1);
			shadowParam.near = 10;
			shadowParam.far = 200;
			shadowParam.range = glm::vec2(300, 300);
			game.Shadow(shadowParam);

			game.RemoveAllEntity();
			game.ClearLevel();
			game.LoadMeshFromFile("Res/Model/Player.fbx");
			game.LoadMeshFromFile("Res/Model/Toroid.fbx");
			game.LoadMeshFromFile("Res/Model/Blast.fbx");
			game.LoadTextureFromFile("Res/Model/Player.bmp");
			game.LoadTextureFromFile("Res/Model/Toroid.dds");
			game.LoadTextureFromFile("Res/Model/Toroid.Normal.bmp");

			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);

			switch (stageNo % 3) {
			case 1: {
				game.StopAudio(AudioPlayerId_BGM);
				game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_STAGE01);
				game.KeyValue(0.16f);
				game.LoadMeshFromFile("Res/Model/Mountain/Landscape.fbx");
				game.LoadTextureFromFile("Res/Model/Mountain/Mountain.Diffuse.dds");
				game.LoadTextureFromFile("Res/Model/Mountain/Mountain.Normal.bmp");
				for (int z = 0; z < 5; ++z) {
					const float offsetZ = static_cast<float>(z * 40 * 5);
					for (int x = 0; x < 5; ++x) {
						const float offsetX = static_cast<float>(x * 40 - 80) * 5.0f;
						auto entity = game.AddEntity(EntityGroupId_Others, glm::vec3(offsetX, -100, offsetZ), "Landscape01", "Res/Model/Mountain/Mountain.Diffuse.dds",
							"Res/Model/Mountain/Mountain.Normal.bmp", &UpdateLandscape);
					}
				}
				break;
			}
			case 2: {
				game.StopAudio(AudioPlayerId_BGM);
				game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_STAGE02);
				game.KeyValue(0.24f);
				game.LoadMeshFromFile("Res/Model/City/City01.fbx");
				game.LoadTextureFromFile("Res/Model/City/City01.Diffuse.dds");
				game.LoadTextureFromFile("Res/Model/City/City01.Normal.bmp");
				for (int z = 0; z < 12; ++z) {
					const float offsetZ = static_cast<float>(z * 40);
					for (int x = 0; x < 5; ++x) {
						const float offsetX = static_cast<float>(x * 40 - 80);
						auto entity = game.AddEntity(EntityGroupId_Others, glm::vec3(offsetX, -10, offsetZ), "City01", "Res/Model/City/City01.Diffuse.dds",
							"Res/Model/City/City01.Normal.bmp", &UpdateLandscape);
						game.AddEntity(EntityGroupId_Others, glm::vec3(offsetX, -10, offsetZ), "City01.Shadow", "Res/Model/City/City01.Diffuse.dds",
							"Res/Model/City/City01.Normal.bmp", &UpdateLandscape);
					}
				}
				break;
			}
			case 0: {
				game.StopAudio(AudioPlayerId_BGM);
				game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_BATTLE);
				game.KeyValue(0.02f);
				game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
				game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");
				auto entity = game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", &UpdateSpaceSphere, "NonLighting");
				break;
			}
			}
		}

		if (game.Variable("life") < 1) {
			//game.StopAudio(AudioPlayerId_BGM);
			//game.UpdateFunc(GameOver());
			if (initial) {
				initial = false;
				game.ClearCollisionHandlerList();
				game.StopAudio(AudioPlayerId_BGM);
				game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_GAMEOVER);
			}
			const float offset = timer == 0 ? 0 : (2.0f - timer) * (2.0f - timer) * 2.0f * 400.0f;
			game.FontScale(glm::vec2(2.0f, 2.0f));
			game.FontColor(glm::vec4(1.0f, 0, 0, 1.0f));
			game.AddString(glm::vec2(300.0f + offset, 260.0f), "Game Over");
			game.FontScale(glm::vec2(0.5f, 0.5f));
			game.FontColor(glm::vec4(0.75f, 0.75f, 0.75f, 1.0f));
			game.AddString(glm::vec2(480.0f + offset, 328.0f), "Press Enter To Title");
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
		else {
			if (game.Variable("lank") > 25) {
				game.ClearCollisionHandlerList();
				game.StopAudio(AudioPlayerId_BGM);
				game.UpdateFunc(GameClear());
			}
			else {
				stageTimer -= delta;

				std::uniform_int_distribution<> posXRange(-15, 15);
				std::uniform_int_distribution<> posZRange(38, 40);
				interval -= delta;

				if (game.Variable("lank") > 10) {
					enemyLevel = game.Variable("Level");
					if (interval <= 0) {
						const std::uniform_real_distribution<> rndAddingTime(2.0 - (enemyLevel % 20) / 10, 6.0 - (enemyLevel % 20) / 6);
						std::uniform_int_distribution<> rndAddingCount(1, 5);
						for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
							const glm::vec3 pos(posXRange(game.Rand()), 0, posZRange(game.Rand()));
							if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.dds", "Res/Model/Toroid.Normal.bmp", UpdateToroid(pPlayer, enemyLevel)))
							{
								p->Velocity({ pos.x < 0 ? 1.0f : -1.0f, 0, -4.0f });
								p->Collision(collisionDataList[EntityGroupId_Enemy]);
							}
						}
						std::normal_distribution<> intervalRange(2.0, 0.5);
						interval = rndAddingTime(game.Rand());
					}
				}
				else {
					if (interval <= 0) {
						std::uniform_int_distribution<> rndAddingCount(1, 5);

						for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
							const glm::vec3 pos(posXRange(game.Rand()), 0, posZRange(game.Rand()));

							if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.dds", "Res/Model/Toroid.Normal.bmp", UpdateToroid(pPlayer)))
							{
								p->Velocity({ pos.x < 0 ? 3.0f : -3.0f, 0, -12.0f });
								p->Collision(collisionDataList[EntityGroupId_Enemy]);
							}
						}
						std::normal_distribution<> intervalRange(2.0, 0.5);
						interval += glm::clamp(intervalRange(game.Rand()), 0.5, 3.0);
					}
				}
			}
		}
		
		char str[32];
		char lank;
		if (game.Variable("highScore") <= game.Variable("score")) {
			game.Variable("highScore") = game.Variable("score");
		}		
		snprintf(str, sizeof(str), "SCORE:%06.0f", game.Variable("score"));
		game.FontScale(glm::vec2(1.5f, 1.5f));
		game.FontColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		game.AddString(glm::vec2(250.0f, 8.0f), str);
		snprintf(str, sizeof(str), "HIGHSCORE:%06.0f", game.Variable("highScore"));
		game.FontScale(glm::vec2(0.7f, 0.7f));
		game.FontColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		game.AddString(glm::vec2(590.0f, 8.0f), str);
		if (game.Variable("lank") > 20) {
			lank = 'A';
			game.FontColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		}
		else if (game.Variable("lank") > 15) {
			lank = 'B';
			game.FontColor(glm::vec4(0.8f, 0.8f, 0.0f, 1.0f));
		}
		else if (game.Variable("lank") > 5) {
			lank = 'C';
			game.FontColor(glm::vec4(0.6f, 0.6f, 0.0f, 1.0f));
		}
		else {
			lank = 'D';
			game.FontColor(glm::vec4(0.4f, 0.4f, 0.0f, 1.0f));
		}
		snprintf(str, sizeof(str), "LANK:%c", lank);
		game.FontScale(glm::vec2(0.8f, 0.8f));
		game.AddString(glm::vec2(590.0f, 30.0f), str);
		snprintf(str, sizeof(str), "STAGE:%d", stageNo);
		game.FontScale(glm::vec2(1.0f, 1.0f));
		game.FontColor(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
		game.AddString(glm::vec2(1.0f, 8.0f), str);
		snprintf(str, sizeof(str), "LIFE:%02.0f", game.Variable("life"));
		game.FontScale(glm::vec2(1.0f, 1.0f));
		if (game.Variable("life") <= 1) {
			game.FontColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		else {
			game.FontColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		game.AddString(glm::vec2(130.0f, 8.0f), str);

		GameEngine::CameraData camera = game.Camera(1);
		float cameraMoveValue = fmod(static_cast<float>(stageTimer), 45.0f) * (glm::radians(360.0f) / 45.0f);
		camera.position.x = glm::cos(cameraMoveValue) * 5.0f;
		game.Camera(1, camera);
	}
}