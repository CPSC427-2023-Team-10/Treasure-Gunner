#include "sound_system.hpp"
// internal
#include "common.hpp"

void SoundSystem::loadSFX() {
    // Load sound effects from files
    background_music = Mix_LoadMUS(audio_path("bgm.wav").c_str());
    shopBgm = Mix_LoadMUS(audio_path("shopBgm.wav").c_str());
    bossBgm = Mix_LoadMUS(audio_path("bossBgm.wav").c_str());
    openDoor = Mix_LoadWAV(audio_path("openDoor.wav").c_str());
    closeDoor = Mix_LoadWAV(audio_path("closeDoor.wav").c_str());
    step = Mix_LoadWAV(audio_path("step.wav").c_str());
    error = Mix_LoadWAV(audio_path("error.wav").c_str());
    glassBreak = Mix_LoadWAV(audio_path("glassBreak.wav").c_str());
    playerShoot = Mix_LoadWAV(audio_path("playerShoot.wav").c_str());
    ricochet = Mix_LoadWAV(audio_path("ricochet.wav").c_str());
    powerUp = Mix_LoadWAV(audio_path("powerUp.wav").c_str());
    heal = Mix_LoadWAV(audio_path("heal.wav").c_str());
    dodge = Mix_LoadWAV(audio_path("dodge.wav").c_str());
    playerDeath = Mix_LoadWAV(audio_path("playerDeath.wav").c_str());
    playerDamage = Mix_LoadWAV(audio_path("playerDamage.wav").c_str());
    enemyDeath = Mix_LoadWAV(audio_path("enemyDeath.wav").c_str());
    enemyShoot = Mix_LoadWAV(audio_path("enemyShoot.wav").c_str());
    enemyDamage = Mix_LoadWAV(audio_path("enemyDamage.wav").c_str());
    bossDeath = Mix_LoadWAV(audio_path("bossDeath.wav").c_str());
    bossRoar = Mix_LoadWAV(audio_path("bossRoar.wav").c_str());
    bossShoot = Mix_LoadWAV(audio_path("bossShoot.wav").c_str());
    bossDamage = Mix_LoadWAV(audio_path("bossDamage.wav").c_str());
    fire = Mix_LoadWAV(audio_path("fire.wav").c_str());

    Mix_Volume(-1, 64);

    if (background_music == nullptr ) {
	    fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
		    audio_path("bgm.wav").c_str());
    }
}

void SoundSystem::playBGM() {
    int channel = Mix_PlayMusic(background_music, -1);
}

void SoundSystem::playBossBGM() {
    int channel = Mix_PlayMusic(bossBgm, -1);
}

void SoundSystem::playShopBGM() {
    int channel = Mix_PlayMusic(shopBgm, -1);
}

void SoundSystem::playError() {
    int channel = Mix_PlayChannel(-1, error, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playOpenDoor() {
    int channel = Mix_PlayChannel(-1, openDoor, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playCloseDoor() {
    int channel = Mix_PlayChannel(-1, closeDoor, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playGlassBreak() {
    int channel = Mix_PlayChannel(-1, glassBreak, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playPlayerShoot() {
    int channel =  Mix_PlayChannel(-1, playerShoot, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playSteps() {
    int channel =  Mix_PlayChannel(-1, step, 3);
    Mix_Volume(channel, 128);
}

void SoundSystem::playRicochet() {
    int channel =  Mix_PlayChannel(-1, playerShoot, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playPowerUp() {
    int channel = Mix_PlayChannel(-1, powerUp, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playHeal() {
    int channel = Mix_PlayChannel(-1, heal, 0);
    Mix_Volume(channel, 64);
}
void SoundSystem::playDodge() {
    int channel = Mix_PlayChannel(-1, dodge, 0);
    Mix_Volume(channel, 32);
}

void SoundSystem::playPlayerDeath() {
    int channel = Mix_PlayChannel(-1, playerDeath, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playPlayerDamage() {
    int channel = Mix_PlayChannel(-1, playerDamage, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playEnemyDeath() {
    int channel = Mix_PlayChannel(-1, enemyDeath, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playEnemyShoot() {
    int channel = Mix_PlayChannel(-1, enemyShoot, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playEnemyDamage() {
    int channel = Mix_PlayChannel(-1, enemyDamage, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playBossDeath() {
    int channel = Mix_PlayChannel(-1, bossDeath, 3);
    Mix_Volume(channel, 64);
}

void SoundSystem::playBossRoar() {
    int channel =  Mix_PlayChannel(-1, bossRoar, 0);
    Mix_Volume(channel, 50);
}

void SoundSystem::playBossShoot() {
    int channel = Mix_PlayChannel(-1, bossShoot, 0);
    Mix_Volume(channel, 75);
}

void SoundSystem::playBossDamage() {
    int channel =  Mix_PlayChannel(-1, bossDamage, 0);
    Mix_Volume(channel, 64);
}

void SoundSystem::playFire() {
    int channel = Mix_PlayChannel(-1, fire, -1);
    Mix_Volume(channel, 32);
    fireChannel = channel;
    playingFire = true;
}

void SoundSystem::stopFire() {
    Mix_HaltChannel(fireChannel);
    playingFire = false;
}