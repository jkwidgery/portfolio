//------------------------------------------------------------------------------
//
// File Name:	GameStateMainMenu.c
// Author(s):	Jasmine Widgery
// Project:		-Hint of Death
// Course:		CS230S21
//
// Copyright ?2021 DigiPen (USA) Corporation.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AEEngine.h"
#include "GameStateManager.h"
#include "GameStateMainMenu.h"
#include "CS230/SpriteSource.h"
#include "Button.h"
#include "TextManager.h"
#include "Window.h"
#include "MenuBackGround.h"
#include "MenuSelection.h"
#include "Input.h"

//------------------------------------------------------------------------------
// Private Constants:
//------------------------------------------------------------------------------

const static Vector2D maxButtonSize = { 80.f, 80.f };
const static Vector2D maxButtonSizeCred = { 70.f, 70.f };
const static Vector2D minButtonSize = { 50.f, 50.f };
const static colorRGB selectColor = { 84.f / 255.f, 222.f / 255.f, 125.f / 255.f, 1 };
const static colorRGB buttonColor = { 248.f / 255.f, 65.f / 255.f, 50.f / 255.f, 1 };

#define maxSelected 3

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

static AEGfxVertexList* buttonMesh;
static SpriteSourcePtr buttonSpriteSour;
static AEGfxTexturePtr fontSheet;

static buttonPtr bplay;
static buttonPtr bexit;
static buttonPtr bcredits;

static textStrPtr sbplay;
static textStrPtr sbexit;
static textStrPtr sbcredits;

static buttonPtr* allButtons[maxSelected] = { &bplay, &bcredits, &bexit };

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

// Load the resources associated with the game state.
void GameStateMainMenuLoad() {
	fontSheet = AEGfxTextureLoad("Assets/FontSheet.png");
	AE_WARNING_MESG(fontSheet, "Failed to load FontSheet!!");
	TextManagerLoad();
	MenuBackGroundLoad();
}

// Initialize the memory associated with the game state.
void GameStateMainMenuInit() {
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	buttonMesh = ButtonCreateTextMesh();
	buttonSpriteSour = ButtonCreateTextSpriteSource(fontSheet);

	ButtonInit();
	menuSelectionInit(maxSelected);

	int windowWidth;

	WindowGetSize(&windowWidth, NULL);
	
	colorRGB buttonShadowColor = { 77.f / 255.f, 7.f / 255.f, 2.f / 255.f, 1 };
	float buttonShadowOffset = 4.f;

	// Shadow for Buttons
	sbplay = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 450 + buttonShadowOffset, 50, 50, 0, &buttonShadowColor, 1, "Play");
	sbexit = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 650 + buttonShadowOffset, 50, 50, 0, &buttonShadowColor, 1, "Exit");
	sbcredits = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 550 + buttonShadowOffset, 50, 50, 0, &buttonShadowColor, 1, "Credits");

	// Top button
	bplay = ButtonCreate(windowWidth / 2.f, 450, 50, 50, &buttonColor, "Play", buttonSpriteSour, buttonMesh, true);
	bexit = ButtonCreate(windowWidth / 2.f, 650, 50, 50, &buttonColor, "Exit", buttonSpriteSour, buttonMesh, true);
	bcredits = ButtonCreate(windowWidth / 2.f, 550, 50, 50, &buttonColor, "Credits", buttonSpriteSour, buttonMesh, true);

	colorRGB titleColor = { 1, 1, 1, 1 };

	colorRGB titleShadowColor = { 60.f / 255.f, 60.f / 255.f, 60.f / 255.f, 1 };
	float shadowOffset = 5.f;

	TextManagerCreateText(windowWidth / 2.f + shadowOffset, 200 + shadowOffset, 40, 70, 0.f, &titleShadowColor, 1, "Untitled square Game");
	TextManagerCreateText(windowWidth / 2.f, 200, 40, 70, 0.f, &titleColor, 0, "Untitled square Game");
}

// Update the game state.
// Params:
//	 dt = Change in time (in seconds) since the last game loop.
void GameStateMainMenuPhysics(float dt) {
	ButtonUpdate();
	ButtonUpdateLook(bplay, minButtonSize, maxButtonSize, &selectColor, &buttonColor, dt);
	ButtonUpdateLook(bexit, minButtonSize, maxButtonSize, &selectColor, &buttonColor, dt);
	ButtonUpdateLook(bcredits, minButtonSize, maxButtonSizeCred, &selectColor, &buttonColor, dt);

	ButtonShadowUpdate(bplay, sbplay);
	ButtonShadowUpdate(bexit, sbexit);
	ButtonShadowUpdate(bcredits, sbcredits);
}

void GameStateMainMenuUpdate() {
	bool inputChaged = menuSelectionUpdate(allButtons);

	if (!inputChaged) {
		if (ButtonReleased(bplay)) {
			GameStateManagerSetNextState(GsModeMenu);

		} else if (ButtonReleased(bexit)) {
			GameStateManagerSetNextState(GsQuit);

		} else if (ButtonReleased(bcredits)) {
			GameStateManagerSetNextState(GsCredits);
		}
	}

	updateInput();

	// Draw
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	DrawMenuBackGround();
	TextManagerDraw(1);
	ButtonDraw(bplay);
	ButtonDraw(bexit);
	ButtonDraw(bcredits);
	TextManagerDraw(0);
}

// Free any memory associated with the game state.
void GameStateMainMenuShutdown() {
	// Free all objects.
	ButtonFree(bplay);
	ButtonFree(bexit);
	ButtonFree(bcredits);

	TextManagerShutdown();
}

// Unload the resources associated with the game state.
void GameStateMainMenuUnload() {
	TextManagerUnload();
	MenuBackGroundUnload();
	// Free all sprite sources.
	SpriteSourceFree(&buttonSpriteSour);

	// Unload all textures.
	AEGfxTextureUnload(fontSheet);

	// Free all meshes.
	AEGfxMeshFree(buttonMesh);
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------