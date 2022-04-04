#pragma once
//SceneNode category used to dispatch commands
namespace Category
{
	enum Type
	{
		kNone = 0,
		kScene = 1 << 0,
		kPlatform = 1 << 1,
		kPlayerCharacter = 1 << 2,
		kActivePlatform = 1 << 3,
		kParticleSystem = 1 << 7,
		kSoundEffect = 1 << 8,
		kNetwork = 1 << 9,
	};
}