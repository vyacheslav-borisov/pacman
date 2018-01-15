#pragma once

namespace Pegas
{
	enum TextureResources
	{
		k_textureMaze = 1,
		k_texturePillTile,
		k_textureSuperPillTile,
		k_texturePacmanLogo,
		
		k_texturePacmanStaticSprite,
		k_textureBlinkyStaticSprite,
		k_texturePinkyStaticSprite,
		k_textureInkyStaticSprite,
		k_textureClydeStaticSprite,
		k_textureBonusStaticSprite,
		k_textureLivesText,
		k_textureScoresText,
		k_textureGetReadyText,
		k_texturePacmanEditorTile,
		k_textureBlinkyEditorTile,
		k_texturePinkyEditorTile,
		k_textureInkyEditorTile,
		k_textureClydeEditorTile,
		k_textureTunnelEditorTile,
		k_textureBonusEditorTile,
		k_textureTunnelStaticSprite,
		k_textureGameObjectEditorTile,
		k_textureCharactersSpriteSheet
	};

	enum FontResorces
	{
		k_fontMain = 1,
		k_fontMenuButton,
		k_fontHUD_Panel,
		k_fontHUD_Frag
	};

	enum SoundResources
	{
		k_soundEatPill = 1,
		k_soundPacmanDeath,
		k_soundGhostChase,
		k_soundGhostRunaway,
		k_soundEyes,
		k_soundGhostKilled,
		k_soundEatBonus
	};
}