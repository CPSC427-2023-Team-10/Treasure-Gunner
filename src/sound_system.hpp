#pragma once

// internal
#include "common.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>


// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class SoundSystem
{
private:
	// music references
	Mix_Music* background_music;
	Mix_Music* shopBgm;
	Mix_Music* bossBgm;

	Mix_Chunk* openDoor;
	Mix_Chunk* closeDoor;
	Mix_Chunk* step;
	Mix_Chunk* error;
	Mix_Chunk* glassBreak;
	Mix_Chunk* playerShoot;
	Mix_Chunk* ricochet;
	Mix_Chunk* powerUp;
	Mix_Chunk* heal;
	Mix_Chunk* dodge;
	Mix_Chunk* playerDeath;
	Mix_Chunk* playerDamage;
	Mix_Chunk* enemyDeath;
	Mix_Chunk* enemyShoot;
	Mix_Chunk* enemyDamage;
	Mix_Chunk* bossDeath;
	Mix_Chunk* bossRoar;
	Mix_Chunk* bossShoot;
	Mix_Chunk* bossDamage;
	Mix_Chunk* fire;
	int fireChannel;



public:
	void loadSFX();
	void playBGM();
	void playBossBGM();
	void playShopBGM();
	void playError();
	void playGlassBreak();
	void playSteps();
	void playCloseDoor();
	void playOpenDoor();
	void playPlayerShoot();
	void playRicochet();
	void playPowerUp();
	void playHeal();
	void playDodge();
	void playPlayerDamage();
	void playPlayerDeath();
	void playEnemyDeath();
	void playEnemyShoot();
	void playEnemyDamage();
	void playBossDeath();
	void playBossRoar();
	void playBossShoot();
	void playBossDamage();
	void playFire();
	void stopFire();
	bool playingFire;
};
