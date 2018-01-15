#include "stdafx.h"
#include "game_events.h"

using namespace Pegas;

const EventType Event_ChangeDirection::k_type = "Event_ChangeDirection";
const EventType Event_CancelChangingDirection::k_type = "Event_CancelChangingDirection";
const EventType Event_DirectionChanged::k_type = "Event_DirectionChanged";
const EventType Event_CharacterStopped::k_type = "Event_CharacterStopped";
const EventType Event_CharacterMoveOn::k_type = "Event_CharacterMoveOn";
const EventType Event_CharacterKilled::k_type = "Event_CharacterKilled";

const EventType Event_CharacterMoved::k_type = "Event_CharacterMoved";
const EventType Event_CharacterStateChanged::k_type = "Event_CharacterStateChanged";
const EventType Event_CharacterChangeState::k_type = "Event_CharacterChangeState";

const EventType Event_CharacterTonnelIn::k_type = "Event_CharacterTonnelIn";
const EventType Event_CharacterTonnelOut::k_type = "Event_CharacterTonnelOut";

const EventType Event_ShowCharacter::k_type = "Event_ShowCharacter";
const EventType Event_HideCharacter::k_type = "Event_HideCharacter";

const EventType Event_EnableCharacterControl::k_type = "Event_EnableCharacterControl";
const EventType Event_DisableCharacterControl::k_type = "Event_DisableCharacterControl";
const EventType Event_PacmanSwallowedPill::k_type = "Event_PacmanSwallowedPill";
const EventType Event_PacmanDeath::k_type = "Event_PacmanDeath";
const EventType Event_PacmanDeathComplete::k_type = "Event_PacmanDeathComplete";

const EventType Event_SuperForceOn::k_type = "Event_SuperForceOn";
const EventType Event_SuperForceOff::k_type = "Event_SuperForceOff";
const EventType Event_SuperForcePreOff::k_type = "Event_SuperForcePreOff";
const EventType Event_BonusOn::k_type = "Event_BonusOn";
const EventType Event_BonusOff::k_type = "Event_BonusOff";

const EventType Event_ResetActors::k_type = "Event_ResetActors";
const EventType Event_RestartGame::k_type = "Event_RestartGame";


const EventType Event_GUI_ButtonClick::k_type = "Event_GUI_ButtonClick";
const EventType Event_GUI_StartFadein::k_type = "Event_GUI_StartFadein";
const EventType Event_GUI_StartFadeout::k_type = "Event_GUI_StartFadeout";
const EventType Event_GUI_FadeinComplete::k_type = "Event_GUI_FadeinComplete";
const EventType Event_GUI_FadeoutComplete::k_type = "Event_GUI_FadeoutComplete";
const EventType Event_GUI_FadeOn::k_type = "Event_GUI_FadeOn";
const EventType Event_GUI_FadeOff::k_type = "Event_GUI_FadeOff";


const EventType Event_Game_ChangeState::k_type = "Event_Game_ChangeState";
const EventType Event_Game_ForwardToState::k_type = "Event_Game_ForwardToState";
const EventType Event_Game_BackwardToPreviousState::k_type = "Event_Game_BackwardToPreviousState";

const EventType Event_HUD_LevelChanged::k_type = "Event_HUD_LevelChanged";
const EventType Event_HUD_ScoresChanged::k_type = "Event_HUD_ScoresChanged";
const EventType Event_HUD_LivesChanged::k_type = "Event_HUD_LivesChanged";
const EventType Event_HUD_Frag::k_type = "Event_HUD_Frag";
const EventType Event_HUD_NewLevel::k_type = "Event_HUD_NewLevel";

const EventType Event_HUD_GetReady::k_type = "Event_HUD_GetReady";
const EventType Event_HUD_GameOver::k_type = "Event_HUD_GameOver";

const EventType Event_Game_Pause::k_type = "Event_Game_Pause";
const EventType Event_Game_Resume::k_type = "Event_Game_Resume";
