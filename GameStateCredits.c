//------------------------------------------------------------------------------
//
// File Name:	GameStateCredits.c
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
#include "GameStateCredits.h"
#include "Button.h"
#include "TextManager.h"
#include "CS230/SpriteSource.h"
#include "MenuBackGround.h"
#include "Window.h"
#include "MenuSelection.h"
#include "Input.h"
#include "Camera.h"


//------------------------------------------------------------------------------
// Private Constants:
//------------------------------------------------------------------------------

const static Vector2D maxButtonSize = { 40.f, 40.f };
const static Vector2D minButtonSize = { 30.f, 30.f };
const static colorRGB selectColor = { 84.f / 255.f, 222.f / 255.f, 125.f / 255.f, 1 };
const static colorRGB buttonColor = { 248.f / 255.f, 65.f / 255.f, 50.f / 255.f, 1 };

const static colorRGB arrowColor = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 };

const static Vector2D arrowMaxButtonSize = { 70.f, 70.f };
const static Vector2D arrowMinButtonSize = { 50.f, 50.f };

const float scrollSpeed = 250.f;

#define maxSelected 3
#define maxText 23

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

static AEGfxVertexList* buttonMesh;
static AEGfxVertexList* squareMesh;
static SpriteSourcePtr buttonSpriteSour;
static SpriteSourcePtr arrowSpriteSour;
static AEGfxTexturePtr fontSheet;
static AEGfxTexturePtr arrowTexture;

static buttonPtr bback;
static textStrPtr sbback;

static buttonPtr arrowDown;
static buttonPtr arrowUp;
static buttonPtr sarrowDown;
static buttonPtr sarrowUp;

static buttonPtr* allButtons[maxSelected] = { &arrowUp, &arrowDown, &bback };

static textStrPtr text[maxText];
static textStrPtr stext[maxText];

static float scroll = 0.f;

static float deltaDT = 0;

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
void GameStateCreditsLoad() {
	fontSheet = AEGfxTextureLoad("Assets/FontSheet.png");
	AE_WARNING_MESG(fontSheet, "Failed to load FontSheet!!");
	arrowTexture = AEGfxTextureLoad("Assets/Arrow.png");
	AE_WARNING_MESG(arrowTexture, "Failed to load arrowTexture!!");
	TextManagerLoad();
	MenuBackGroundLoad();
}

// Initialize the memory associated with the Stub game state.
void GameStateCreditsInit() {

	lastInput = GameStateGetLastInput();

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	buttonMesh = ButtonCreateTextMesh();
	squareMesh = ButtonCreateBasicMesh();
	buttonSpriteSour = ButtonCreateTextSpriteSource(fontSheet);

	arrowSpriteSour = SpriteSourceCreate(1, 1, arrowTexture);
	AE_WARNING_MESG(arrowSpriteSour, "Failed to create sprite source for arrow!!");

	ButtonInit();
	menuSelectionInit(maxSelected);

	int windowWidth, windowHeight;

	WindowGetSize(&windowWidth, &windowHeight);

	colorRGB buttonShadowColor = { 77.f / 255.f, 7.f / 255.f, 2.f / 255.f, 1 };
	float buttonShadowOffset = 3.f;

	// Shadow for Buttons
	sbback = TextManagerCreateText(110 + buttonShadowOffset, 50 + buttonShadowOffset, 30, 30, 0, &buttonShadowColor, 1, "<Back");

	bback = ButtonCreate(110, 50, 30, 30, &buttonColor, "<Back", buttonSpriteSour, buttonMesh, true);

	// Arrows

	colorRGB arrowShadowColor = { 200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1 };

	arrowDown = ButtonCreate(110, (float)windowHeight - 50.f, 50, 50, NULL, "arrowDown", arrowSpriteSour, squareMesh, false);
	arrowUp = ButtonCreate(110, 100, 50, 50, NULL, "arrowUp", arrowSpriteSour, squareMesh, false);
	ButtonSetRotation(arrowDown, PI / 2.f);
	ButtonSetRotation(arrowUp, PI / -2.f);

	sarrowDown = ButtonCreate(110 + buttonShadowOffset, windowHeight - 50 + buttonShadowOffset, 50, 50, &arrowShadowColor, "sarrowDown", arrowSpriteSour, squareMesh, false);
	sarrowUp = ButtonCreate(110 + buttonShadowOffset, 100 + buttonShadowOffset, 50, 50, &arrowShadowColor, "sarrowUp", arrowSpriteSour, squareMesh, false);
	ButtonSetRotation(sarrowDown, PI / 2.f);
	ButtonSetRotation(sarrowUp, PI / -2.f);

	// Text

	const colorRGB nameColor = { 190.f / 255.f, 190.f / 255.f, 190.f / 255.f, 1 };
	const colorRGB nameShadowColor = { 50.f / 255.f, 50.f / 255.f, 50.f / 255.f, 1 };
	const colorRGB headerColor = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 };
	const colorRGB headerShadowColor = { 60.f / 255.f, 60.f / 255.f, 60.f / 255.f, 1 };
	const colorRGB copyrightColor = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 1 };

	const float startY = 110.f;
	const float lineSep = 40.f;
	const float SectSep = 30.f;

	const float middle = (float)windowWidth / 2.f;

	const float shadowOffset = 3.f;
	const float headerShadowOffset = 2.f;

	// Shadows
	int line = 0;

	stext[line] = TextManagerCreateText(middle - headerShadowOffset, startY + headerShadowOffset, 30.f, 30.f, 0.f, &headerShadowColor, 1, "-HINT OF DEATH");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "DAVID COLACURCIO");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "BRENDEN EPP");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "DONGHO LEE");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "BRYAN SHUMWAY");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "JASMINE WIDGERY");
	++line;

	stext[line] = TextManagerCreateText(middle - headerShadowOffset, startY + (lineSep * line) + (SectSep * 1) + headerShadowOffset, 30.f, 30.f, 0.f, &headerShadowColor, 1, "PROFESSORS:");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 1) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "JUSTIN CHAMBERS");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 1) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "MATTHEW PICOCCIO");
	++line;

	stext[line] = TextManagerCreateText(middle - headerShadowOffset, startY + (lineSep * line) + (SectSep * 2) + headerShadowOffset, 30.f, 30.f, 0.f, &headerShadowColor, 1, "SPECIAL THANKS:");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 2) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "Kenny Mecham");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 2) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "Justin Stachowiak");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 2) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "GAM150 TAs");
	++line;

	stext[line] = TextManagerCreateText(middle - headerShadowOffset, startY + (lineSep * line) + (SectSep * 3) + headerShadowOffset, 30.f, 30.f, 0.f, &headerShadowColor, 1, "Created at");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 3) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "digipen institute of technology");
	++line;

	stext[line] = TextManagerCreateText(middle - headerShadowOffset, startY + (lineSep * line) + (SectSep * 4) + headerShadowOffset, 30.f, 30.f, 0.f, &headerShadowColor, 1, "President:");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 4) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "claude comair");
	++line;

	stext[line] = TextManagerCreateText(middle - headerShadowOffset, startY + (lineSep * line) + (SectSep * 5) + headerShadowOffset, 30.f, 30.f, 0.f, &headerShadowColor, 1, "EXECUTIVES:");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 5) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "JASON CHU  SAMIR ABOU SAMRA   MICHELE COMAIR");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 5) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "ANGELA KUGLER  ERIK MOHRMANN");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 5) + shadowOffset, 20.f, 20.f, 0.f, &nameShadowColor, 1, "BENJAMIN ELLINGER   MELVIN GONSALVEZ");
	++line;

	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 6) + shadowOffset, 13.f, 13.f, 0.f, &nameShadowColor, 1, "FMOD Sound System (c) FireLight Technologies Pty Ltd (1998 - 2020)");
	++line;
	stext[line] = TextManagerCreateText(middle + shadowOffset, startY + (lineSep * line) + (SectSep * 6) + shadowOffset, 16.f, 16.f, 0.f, &nameShadowColor, 1, "COPYRIGHT (C) 2021 BY DIGIPEN (USA), LLC. ALL RIGHTS RESERVED");

	// Main Txt
	line = 0;

	text[line] = TextManagerCreateText(middle, startY, 30.f, 30.f, 0.f, &headerColor, 0, "-HINT OF DEATH");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line), 20.f, 20.f, 0.f, &nameColor, 0, "DAVID COLACURCIO");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line), 20.f, 20.f, 0.f, &nameColor, 0, "BRENDEN EPP");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line), 20.f, 20.f, 0.f, &nameColor, 0, "DONGHO LEE");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line), 20.f, 20.f, 0.f, &nameColor, 0, "BRYAN SHUMWAY");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line), 20.f, 20.f, 0.f, &nameColor, 0, "JASMINE WIDGERY");
	++line;

	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 1), 30.f, 30.f, 0.f, &headerColor, 0, "PROFESSORS:");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 1), 20.f, 20.f, 0.f, &nameColor, 0, "JUSTIN CHAMBERS");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 1), 20.f, 20.f, 0.f, &nameColor, 0, "MATTHEW PICOCCIO");
	++line;

	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 2), 30.f, 30.f, 0.f, &headerColor, 0, "SPECIAL THANKS:");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 2), 20.f, 20.f, 0.f, &nameColor, 0, "Kenny Mecham");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 2), 20.f, 20.f, 0.f, &nameColor, 0, "Justin Stachowiak");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 2), 20.f, 20.f, 0.f, &nameColor, 0, "GAM150 TAs");
	++line;

	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 3), 30.f, 30.f, 0.f, &headerColor, 0, "Created at");
	++line;
	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 3), 20.f, 20.f, 0.f, &nameColor, 0, "digipen institute of technology");
	++line;

	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 4), 30.f, 30.f, 0.f, &headerColor, 0, "President:");
	++line;
	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 4), 20.f, 20.f, 0.f, &nameColor, 0, "claude comair");
	++line;

	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 5), 30.f, 30.f, 0.f, &headerColor, 0, "EXECUTIVES:");
	++line;
	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 5), 20.f, 20.f, 0.f, &nameColor, 0, "JASON CHU  SAMIR ABOU SAMRA   MICHELE COMAIR");
	++line;
	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 5), 20.f, 20.f, 0.f, &nameColor, 0, "ANGELA KUGLER  ERIK MOHRMANN");
	++line;
	text[line] =TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 5), 20.f, 20.f, 0.f, &nameColor, 0, "BENJAMIN ELLINGER   MELVIN GONSALVEZ");
	++line;

	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 6), 13.f, 13.f, 0.f, &copyrightColor, 0, "FMOD Sound System (c) FireLight Technologies Pty Ltd (1998 - 2020)");
	++line;
	text[line] = TextManagerCreateText(middle, startY + (lineSep * line) + (SectSep * 6), 16.f, 16.f, 0.f, &copyrightColor, 0, "COPYRIGHT (C) 2021 BY DIGIPEN (USA), LLC. ALL RIGHTS RESERVED");

}

void GameStateCreditsPhysics(float dt) {
	ButtonUpdate();
	ButtonUpdateLook(bback, minButtonSize, maxButtonSize, &selectColor, &buttonColor, dt);

	ButtonShadowUpdate(bback, sbback);

	// Update arrow
	ButtonUpdateLook(arrowDown, arrowMinButtonSize, arrowMaxButtonSize, NULL, NULL, dt);
	ButtonUpdateLook(arrowUp, arrowMinButtonSize, arrowMaxButtonSize, NULL, NULL, dt);

	// Update arrow shadow
	Vector2D size = ButtonGetSize(arrowDown);
	ButtonSetSize(sarrowDown, size.x, size.y);

	size = ButtonGetSize(arrowUp);
	ButtonSetSize(sarrowUp, size.x, size.y);

	deltaDT += dt;
}

void GameStateCreditsUpdate() {

	int result = check_Preset(4);
	if (result == -3 && lastInput != -3) // B button pressed on a controller
	{
		GameStateSetLastInput(result);
		GameStateManagerSetNextState(GsMainMenu);
	}
	lastInput = result;

	scroll = 0.f;

	if (!menuSelectionUpdate(allButtons)) {
		if (ButtonReleased(bback)) {
			GameStateManagerSetNextState(GsMainMenu);
		}
		if (ButtonCurr(arrowUp)) {
			scroll += scrollSpeed * deltaDT;
		}
		if (ButtonCurr(arrowDown)) {
			scroll -= scrollSpeed * deltaDT;
		}
	}
	deltaDT = 0;

	// Sets scroll so you cant scroll off screen
	if (scroll > 0) {
		Vector2D pos1 = TextManagerGetPosition(text[0]);
		if (pos1.y + scroll > 110.f) {
			scroll = 110.f - pos1.y;
		}
	}
	if (scroll < 0) {
		int windowHeight;
		WindowGetSize(NULL, &windowHeight);
		Vector2D pos1 = TextManagerGetPosition(text[maxText - 1]);
		if (pos1.y + scroll < windowHeight - 110.f) {
			scroll = (windowHeight - 110.f) - pos1.y;
		}
	}

	// Updates text drawing
	for (int i = 0; i < maxText; ++i) {
		Vector2D pos = TextManagerGetPosition(text[i]);
		TextManagerSetPosition(text[i], pos.x, pos.y + scroll);

		pos = TextManagerGetPosition(stext[i]);
		TextManagerSetPosition(stext[i], pos.x, pos.y + scroll);
	}


	updateInput();

	// Draw
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	DrawMenuBackGround();
	TextManagerDraw(1);
	TextManagerDraw(0);
	ButtonDraw(bback);
	ButtonDraw(sarrowUp);
	ButtonDraw(sarrowDown);
	ButtonDraw(arrowUp);
	ButtonDraw(arrowDown);
}

// Free any memory associated with the Stub game state.
void GameStateCreditsShutdown()
{
	// Free all objects.
	ButtonFree(bback);
	TextManagerShutdown();
}

// Unload the resources associated with the Stub game state.
void GameStateCreditsUnload() {
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

