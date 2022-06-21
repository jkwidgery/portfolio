//------------------------------------------------------------------------------
//
// File Name:	PauseMenu.c
// Author(s):	Jasmine Widgery
// Project:		Project 0
// Course:		CS230S21
//
// Copyright ?2021 DigiPen (USA) Corporation.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AEEngine.h"
#include "Button.h"
#include "GameStateManager.h"
#include "CS230/SpriteSource.h"
#include "Window.h"
#include "TextManager.h"
#include "CS230/Mesh.h"
#include "PauseMenu.h"
#include "MenuSelection.h"
#include "ColorShift.h"
#include "Input.h"
#include "PlayerCharacters.h"

//------------------------------------------------------------------------------
// Private Consts:
//------------------------------------------------------------------------------

bool isPaused;

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

int playerPoints[4];

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

const static colorRGB selectColor = { 84.f / 255.f, 222.f / 255.f, 125.f / 255.f, 1 };
const static colorRGB buttonColor = { 248.f / 255.f, 65.f / 255.f, 50.f / 255.f, 1 };

const static colorRGB titleColor = { 1, 1, 1, 1 };

const static Vector2D minButtonSize = { 50.f, 50.f };
const static Vector2D maxButtonSize = { 80.f, 80.f };

const static Vector2D maxButtonSizeMenu = { 65.f, 65.f };

static AEGfxVertexList* buttonMesh;
static SpriteSourcePtr buttonSpriteSour;
static AEGfxTexturePtr fontSheet;
static AEGfxTexturePtr box;

static buttonPtr bresume;
static buttonPtr bexit;
static buttonPtr bmainmenu;
static buttonPtr btitle;

static textStrPtr sbresume;
static textStrPtr sbexit;
static textStrPtr sbmainmenu;
static textStrPtr sbtitle;

static buttonPtr* allButtons[3] = { &bresume, &bmainmenu, &bexit };

static AEGfxVertexList* boxMesh;

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

void PauseMenuLoad() 
{
	fontSheet = AEGfxTextureLoad("Assets/FontSheet.png");
	box = AEGfxTextureLoad("Assets/PauseBG.png");
	AE_WARNING_MESG(fontSheet, "Failed to load FontSheet!!");
}

// Initialize the ...
void PauseMenuInit()
{
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	boxMesh = MeshCreateQuad(0.5f, 0.5f, 1.f, 1.f, "1x1");
	buttonMesh = ButtonCreateTextMesh();
	buttonSpriteSour = ButtonCreateTextSpriteSource(fontSheet);

	ButtonInit();
	menuSelectionInit(3);

	int windowWidth;

	WindowGetSize(&windowWidth, NULL);

	colorRGB buttonShadowColor = { 77.f / 255.f, 7.f / 255.f, 2.f / 255.f, 1 };
	float buttonShadowOffset = 4.f;

	colorRGB titleShadowColor = { 60.f / 255.f, 60.f / 255.f, 60.f / 255.f, 1 };

	sbresume = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 400 + buttonShadowOffset, 50, 50, 0, &buttonShadowColor, 1, "Resume");
	sbmainmenu = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 500 + buttonShadowOffset, 50, 50, 0, &buttonShadowColor, 1, "Main Menu");
	sbexit = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 600 + buttonShadowOffset, 50, 50, 0, &buttonShadowColor, 1, "Exit");
	sbtitle = TextManagerCreateText(windowWidth / 2.f + buttonShadowOffset, 250 + buttonShadowOffset, 70, 70, 0, &titleShadowColor, 1, "Paused");

	// Top button
	bresume = ButtonCreate(windowWidth / 2.f, 400, 50, 50, &buttonColor, "Resume", buttonSpriteSour, buttonMesh, true);
	bmainmenu = ButtonCreate(windowWidth / 2.f, 500, 50, 50, &buttonColor, "Main Menu", buttonSpriteSour, buttonMesh, true);
	bexit = ButtonCreate(windowWidth / 2.f, 600, 50, 50, &buttonColor, "Exit", buttonSpriteSour, buttonMesh, true);
	btitle = ButtonCreate(windowWidth / 2.f, 250, 70, 70, &titleColor, "Paused", buttonSpriteSour, buttonMesh, true);
	
}

// Update the ...
// Params:
//	 dt = Change in time (in seconds) since the last game loop.

float dt = 0.010f;
void PauseMenuUpdate() {
	ButtonUpdate();
	menuSelectionUpdate(allButtons);
	ButtonUpdateLook(bresume, minButtonSize, maxButtonSize, &selectColor, &buttonColor, dt);
	ButtonUpdateLook(bmainmenu, minButtonSize, maxButtonSizeMenu, &selectColor, &buttonColor, dt);
	ButtonUpdateLook(bexit, minButtonSize, maxButtonSize, &selectColor, &buttonColor, dt);

	ButtonShadowUpdate(bresume, sbresume);
	ButtonShadowUpdate(bexit, sbexit);
	ButtonShadowUpdate(bmainmenu, sbmainmenu);
	ButtonShadowUpdate(btitle, sbtitle);

	if (ButtonReleased(bresume)) 
	{
		//TextManagerSetText(sbtitle, "");
		isPaused = !isPaused;
		GameStateManagerSetNextState(GsMainGame);

	}
	else if (ButtonReleased(bexit))
	{
		//TextManagerSetText(sbtitle, "");
		isPaused = !isPaused;
		GameStateManagerSetNextState(GsQuit);

	}
	else if (ButtonReleased(bmainmenu)) 
	{
		//TextManagerSetText(sbtitle, "");
		// Resets player points
		for (int i = 0; i < PlayerGetCount(); i++) {
			playerPoints[i] = 0;
		}
		isPaused = !isPaused;
		GameStateManagerSetNextState(GsMainMenu);
	}

	updateInput();

	AEGfxTextureSet(box, 0, 0);
	AEGfxSetFullTransform(500.f, 400.f, 0.f, 800.f, 800.f);
	AEGfxSetTintColor(0.f, 0.f, 0.f, 0.5f);
	AEGfxSetBlendColor(1.f, 1.f, 1.f, 1.f);
	AEGfxMeshDraw(boxMesh, AE_GFX_MDM_TRIANGLES);
	TextManagerDraw(1);
	ButtonDraw(bresume);
	ButtonDraw(bmainmenu);
	ButtonDraw(bexit);
	ButtonDraw(btitle);
}

// Shutdown the ...
void PauseMenuShutdown()
{
	ButtonFree(bresume);
	ButtonFree(bexit);
	ButtonFree(bmainmenu);
	ButtonFree(btitle);

	AEGfxMeshFree(boxMesh);
	
}

void PauseMenuUnload()
{
	// Free all sprite sources.
	SpriteSourceFree(&buttonSpriteSour);

	// Unload all textures.
	AEGfxTextureUnload(fontSheet);
	AEGfxTextureUnload(box);
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

