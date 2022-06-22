//------------------------------------------------------------------------------
//
// File Name:	GameStateModeMenu.c
// Author(s):	Jasmine Widgery
// Project:		Hint of Death Game
// Course:		CS230S21
//
// Copyright ?2021 DigiPen (USA) Corporation.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AEEngine.h"
#include "GameStateManager.h"
#include "GameStateMainMenu.h"
#include "GameStateModeMenu.h"
#include "GameStatePartyMode.h"
#include "TextManager.h"
#include "Trace.h"
#include "CS230/SpriteSource.h"
#include "CS230/Transform.h"
#include "PlayerCharacters.h"
#include "Button.h"
#include "ColorShift.h"
#include "Window.h"
#include "MenuBackGround.h"
#include "MenuSelection.h"
#include "Input.h"

//------------------------------------------------------------------------------
// Private Constants:
//------------------------------------------------------------------------------

const static Vector2D maxButtonSize = { 80.f, 80.f };
const static Vector2D modeMaxButtonSizeTimeTrial = { 70.f, 70.f };
const static Vector2D minButtonSize = { 50.f, 50.f };

const static Vector2D maxButtonSizeBack = { 40.f, 40.f };
const static Vector2D minButtonSizeBack = { 30.f, 30.f };

const static colorRGB selectColor = { 84.f / 255.f, 222.f / 255.f, 125.f / 255.f, 1 };
const static colorRGB buttonColor = { 248.f / 255.f, 65.f / 255.f, 50.f / 255.f, 1 };

#define maxSelected 3

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

static AEGfxVertexList* buttonMesh;
static SpriteSourcePtr buttonSpriteSour;
static AEGfxTexturePtr fontSheet;

static buttonPtr bback;
static buttonPtr btimetrial;
static buttonPtr bparty;

static GameObjectPtr player1;
static GameObjectPtr player4;
static AEGfxTexturePtr player1Tex;
static AEGfxTexturePtr player4Tex;
static SpriteSourcePtr player1SpriteSour;
static SpriteSourcePtr player4SpriteSour;
static AEGfxVertexList* playerMesh;


static textStrPtr sbback;
static textStrPtr sbtimetrial;
static textStrPtr sbparty;

static buttonPtr* allButtons[maxSelected] = { &btimetrial, &bparty, &bback };

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

static int lastInput;

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

// Load the resources associated with the Stub game state.
void GameStateModeMenuLoad() {
	TextManagerLoad();
	fontSheet = AEGfxTextureLoad("Assets/FontSheet.png");
	AE_WARNING_MESG(fontSheet, "Failed to load FontSheet!!");
	player1Tex = AEGfxTextureLoad("Assets/1Player.png");
	AE_WARNING_MESG(player1Tex, "Failed to load player1Tex!!");
	player4Tex = AEGfxTextureLoad("Assets/4Player.png");
	AE_WARNING_MESG(player4Tex, "Failed to load player4Tex!!");
	MenuBackGroundLoad();
}

// Initialize the memory associated with the Stub game state.
void GameStateModeMenuInit() {
	lastInput = GameStateGetLastInput();

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	ButtonInit();
	menuSelectionInit(maxSelected);

	buttonMesh = ButtonCreateTextMesh();
	buttonSpriteSour = ButtonCreateTextSpriteSource(fontSheet);

	int windowWidth;

	WindowGetSize(&windowWidth, NULL);

	playerMesh = ButtonCreateBasicMesh();
	player1SpriteSour = ButtonCreateTextSpriteSource(player1Tex);
	player4SpriteSour = ButtonCreateTextSpriteSource(player4Tex);
	SpritePtr player1Spr = SpriteCreate();
	SpritePtr player4Spr = SpriteCreate();
	SpriteSetSpriteSource(player1Spr, player1SpriteSour);
	SpriteSetSpriteSource(player4Spr, player4SpriteSour);
	SpriteSetMesh(player1Spr, playerMesh);
	SpriteSetMesh(player4Spr, playerMesh);
	SpriteSetAlpha(player1Spr, 0.7f);
	SpriteSetAlpha(player4Spr, 0.7f);
	TransformPtr player1Trans = TransformCreate();
	TransformPtr player4Trans = TransformCreate();
	Vector2D translation = { windowWidth * 0.1f, 450 };
	Vector2D scale = { 112.f / 2.0f, 163.f / 2.0f };
	TransformSetFull(player1Trans, &translation, &scale, 0);

	translation.x = windowWidth * 0.1f;
	translation.y = 600;
	scale.x = 224.f / 2.3f;
	scale.y = 265.f / 2.3f;
	TransformSetFull(player4Trans, &translation, &scale, 0);




	colorRGB buttonShadowColor = { 77.f / 255.f, 7.f / 255.f, 2.f / 255.f, 1 };
	float buttonShadowOffset = 4.f;
	float buttonShadowOffsetBack = 3.f;

	// Shawdow
	sbparty = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 600 + buttonShadowOffset, 50, 50, 0.f, &buttonShadowColor, 1, "Party");
	sbtimetrial = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 450 + buttonShadowOffset, 50, 50, 0.f, &buttonShadowColor, 1, "Time Trial");
	sbback = TextManagerCreateText(110 + buttonShadowOffsetBack, 50 + buttonShadowOffsetBack, 30, 30, 0, &buttonShadowColor, 1, "<Back");

	bback = ButtonCreate(110, 50, 30, 30, &buttonColor, "<Back", buttonSpriteSour, buttonMesh, true);
	bparty = ButtonCreate(windowWidth / 2.f, 600, 50, 50, &buttonColor, "Party", buttonSpriteSour, buttonMesh, true);
	btimetrial = ButtonCreate(windowWidth / 2.f, 450, 50, 50, &buttonColor, "Time Trial", buttonSpriteSour, buttonMesh, true);

	player1 = GameObjectCreate();
	GameObjectAddSprite(player1, player1Spr);
	GameObjectAddTransform(player1, player1Trans);
	player4 = GameObjectCreate();
	GameObjectAddSprite(player4, player4Spr);
	GameObjectAddTransform(player4, player4Trans);

	colorRGB titleColor = { 1, 1, 1, 1 };

	colorRGB titleShadowColor = { 60.f / 255.f, 60.f / 255.f, 60.f / 255.f, 1 };
	float shadowOffset = 5.f;

	TextManagerCreateText(windowWidth / 2.f + shadowOffset, 200 + shadowOffset, 40, 40, 0.f, &titleShadowColor, 1, "Choose a game mode:");
	TextManagerCreateText(windowWidth / 2.f, 200, 40, 40, 0.f, &titleColor, 0, "Choose a game mode:");

	globalRotationMode = false;
	globalDarkMode = false;
	globalDoubleJumpMode = false;
	globalKnockBackMode = false;
	globalTimeTrial = false;
}

// Update the Stub game state.
// Params:
//	 dt = Change in time (in seconds) since the last game loop.
void GameStateModeMenuPhysics(float dt) {
	ButtonUpdate();
	ButtonUpdateLook(bparty, minButtonSize, maxButtonSize, &selectColor, &buttonColor, dt);
	ButtonUpdateLook(btimetrial, minButtonSize, modeMaxButtonSizeTimeTrial, &selectColor, &buttonColor, dt);
	ButtonUpdateLook(bback, minButtonSizeBack, maxButtonSizeBack, &selectColor, &buttonColor, dt);

	ButtonShadowUpdate(bback, sbback);
	ButtonShadowUpdate(btimetrial, sbtimetrial);
	ButtonShadowUpdate(bparty, sbparty);

	/*Vector2D size = ButtonGetSize(btimetrial);
	Vector2D pos = ButtonGetPosition(btimetrial);
	TransformPtr trans = GameObjectGetTransform(player1);
	pos.x -= size.x * 6;
	TransformSetTranslation(trans, &pos);

	size = ButtonGetSize(bparty);
	pos = ButtonGetPosition(bparty);
	trans = GameObjectGetTransform(player4);
	pos.x -= size.x * 4;
	TransformSetTranslation(trans, &pos);*/
}

void GameStateModeMenuUpdate() {

	int result = check_Preset(4);
	if (result == -3 && lastInput != -3) // B button pressed on a controller
	{
		GameStateSetLastInput(result);
		GameStateManagerSetNextState(GsMainMenu);
	}
	lastInput = result;

	bool inputChanged = menuSelectionUpdate(allButtons);

	
	if (!inputChanged) {
		if (ButtonReleased(btimetrial)) {
			globalTimeTrial = true;
			PlayerSetCount(1);
			GameStateManagerSetNextState(GsPlayerSelection);
		}
		if (ButtonReleased(bparty)) {
			GameStateManagerSetNextState(GsPartyMode);
		}
		if (ButtonReleased(bback)) {
			GameStateManagerSetNextState(GsMainMenu);
		}
	}

	updateInput();

	// Draw
	DrawMenuBackGround();
	TextManagerDraw(1);
	ButtonDraw(bparty);
	ButtonDraw(btimetrial);
	ButtonDraw(bback);
	GameObjectDraw(player1);
	GameObjectDraw(player4);
	TextManagerDraw(0);
}

// Free any memory associated with the Stub game state.
void GameStateModeMenuShutdown() {
	// Free all objects.
	ButtonFree(bparty);
	ButtonFree(btimetrial);
	ButtonFree(bback);
	TextManagerShutdown();
}

// Unload the resources associated with the Stub game state.
void GameStateModeMenuUnload() {
	MenuBackGroundUnload();
	// Free all sprite sources.
	SpriteSourceFree(&buttonSpriteSour);

	// Unload all textures.
	AEGfxTextureUnload(fontSheet);

	// Free all meshes.
	AEGfxMeshFree(buttonMesh);

	TextManagerUnload();
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

