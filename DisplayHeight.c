//------------------------------------------------------------------------------
//
// File Name:	DisplayHeight.c
// Author(s):	Jasmine Widgery
// Project:		Project 0
// Course:		CS230S21
//
// Copyright ?2021 DigiPen (USA) Corporation.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "DisplayHeight.h"
#include "AEEngine.h"
#include "TextManager.h"
#include "Camera.h"
#include "ColorShift.h"
#include "CS230/SpriteSource.h"
#include "CS230/Vector2D.h"
#include "CS230/Mesh.h"
#include "AEEngine.h"
#include "Window.h"
#include "PlayerCharacters.h"

//------------------------------------------------------------------------------
// Private Consts:
//------------------------------------------------------------------------------

const float frameRate = 1000.f;
#define ItemSize 50

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------

float playerDistance[4];

//------------------------------------------------------------------------------
// Private Variables:
//------------------------------------------------------------------------------

static player myPlayer[4];

static int totalPlayers;

static AEGfxTexturePtr doubleJump;
static AEGfxVertexList* itemMesh = NULL;
static Vector2D player1Pos;
static Vector2D player2Pos;
static Vector2D player3Pos;
static Vector2D player4Pos;

static float highestDis = 0;
static float lowestDis = 0;
static float secHighestDis = 0;
static float secLowestDis = 0;

static textStrPtr player1;
static textStrPtr player2;
static textStrPtr player3;
static textStrPtr player4;

static textStrPtr p1Dist;
static textStrPtr p2Dist;
static textStrPtr p3Dist;
static textStrPtr p4Dist;

static textStrPtr sPlayer[4];
static textStrPtr spDist[4];

static Vector2D sizePreset1;
static Vector2D sizePreset2;
static Vector2D sizePreset3;
static Vector2D sizePreset4;

static colorRGB player1Col;
static colorRGB player2Col;
static colorRGB player3Col;
static colorRGB player4Col;

static float lerpPercent = 0.02f;

static int windowWidth, windowHeight;

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

static void SetDisplaySizeVariables(bool isUpdate, float dt);

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

void DisplayHeightInit()
{
	totalPlayers = PlayerGetCount();
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	itemMesh = MeshCreateQuad(ItemSize / 2.0f, ItemSize / 2.0f, 1.0f, 1.0f, "Item");
	doubleJump = AEGfxTextureLoad("Assets/item_doublejump.png");

	highestDis = playerDistance[0];
	lowestDis = playerDistance[0];
	secLowestDis = playerDistance[0];
	secHighestDis = playerDistance[0];

	player1Col = PlayerGetColor(0);
	player2Col = PlayerGetColor(1);
	player3Col = PlayerGetColor(2);
	player4Col = PlayerGetColor(3);

	sizePreset1.x = 35.f;
	sizePreset1.y = 35.f;

	sizePreset2.x = 30.f;
	sizePreset2.y = 30.f;

	sizePreset3.x = 25.f;
	sizePreset3.y = 25.f;

	sizePreset4.x = 20.f;
	sizePreset4.y = 20.f;

	SetDisplaySizeVariables(0, 0);

	WindowGetSize(&windowWidth, &windowHeight);

	Vector2D posOffset = { 175.f, 55.f };
	float distOffset = 30.f;

	float shadowOff = 3.f;

	// Shadow

	sPlayer[0] = TextManagerCreateText(posOffset.x + shadowOff, posOffset.y + shadowOff, myPlayer[0].heightDisplaySize.x, myPlayer[0].heightDisplaySize.y, 0.f, &player1Col, 7, "Player 1");
	sPlayer[1] = TextManagerCreateText((float)windowWidth - posOffset.x + shadowOff, posOffset.y + shadowOff, myPlayer[1].heightDisplaySize.x, myPlayer[1].heightDisplaySize.y, 0.f, &player2Col, 7, "Player 2");
	sPlayer[2] = TextManagerCreateText(posOffset.x + shadowOff, (float)windowHeight - posOffset.y + shadowOff, myPlayer[2].heightDisplaySize.x, myPlayer[2].heightDisplaySize.y, 0.f, &player3Col, 7, "");
	sPlayer[3] = TextManagerCreateText((float)windowWidth - posOffset.x + shadowOff, (float)windowHeight - posOffset.y + shadowOff, myPlayer[3].heightDisplaySize.x, myPlayer[3].heightDisplaySize.y, 0.f, &player4Col, 7, "");

	spDist[0] = TextManagerCreateText(posOffset.x + shadowOff, posOffset.y + distOffset + shadowOff, myPlayer[0].heightDisplaySize.x, myPlayer[0].heightDisplaySize.y, 0.f, &player1Col, 7, "");
	spDist[1] = TextManagerCreateText((float)windowWidth - posOffset.x + shadowOff, posOffset.y + distOffset + shadowOff, myPlayer[1].heightDisplaySize.x, myPlayer[1].heightDisplaySize.y, 0.f, &player2Col, 7, "");
	spDist[2] = TextManagerCreateText(posOffset.x + shadowOff, (float)windowHeight - posOffset.y + distOffset + shadowOff, myPlayer[2].heightDisplaySize.x, myPlayer[2].heightDisplaySize.y, 0.f, &player3Col, 7, "");
	spDist[3] = TextManagerCreateText((float)windowWidth - posOffset.x + shadowOff, (float)windowHeight - posOffset.y + distOffset + shadowOff, myPlayer[3].heightDisplaySize.x, myPlayer[3].heightDisplaySize.y, 0.f, &player4Col, 7, "");

	// Main text

	player1 = TextManagerCreateText(posOffset.x, posOffset.y, myPlayer[0].heightDisplaySize.x, myPlayer[0].heightDisplaySize.y, 0.f, &player1Col, 7, "Player 1");
	player2 = TextManagerCreateText((float)windowWidth - posOffset.x, posOffset.y, myPlayer[1].heightDisplaySize.x, myPlayer[1].heightDisplaySize.y, 0.f, &player2Col, 7, "Player 2");
	player3 = TextManagerCreateText(posOffset.x, (float)windowHeight - posOffset.y, myPlayer[2].heightDisplaySize.x, myPlayer[2].heightDisplaySize.y, 0.f, &player3Col, 7, "");
	player4 = TextManagerCreateText((float)windowWidth - posOffset.x, (float)windowHeight - posOffset.y, myPlayer[3].heightDisplaySize.x, myPlayer[3].heightDisplaySize.y, 0.f, &player4Col, 7, "");

	player1Pos.x = (posOffset.x) + 170.f;
	player1Pos.y = (posOffset.y) + 20.f;

	player2Pos.x = ((float)windowWidth - posOffset.x) - 170.f;
	player2Pos.y = (posOffset.y) + 20.f;

	player3Pos.x = (posOffset.x) + 170.f;
	player3Pos.y = ((float)windowHeight - posOffset.y) + 20.f;

	player4Pos.x = ((float)windowWidth - posOffset.x) - 170.f;
	player4Pos.y = ((float)windowHeight - posOffset.y)+ 20.f;

	p1Dist = TextManagerCreateText(posOffset.x, posOffset.y + distOffset, myPlayer[0].heightDisplaySize.x, myPlayer[0].heightDisplaySize.y, 0.f, &player1Col, 7, "");
	p2Dist = TextManagerCreateText((float)windowWidth - posOffset.x, posOffset.y + distOffset, myPlayer[1].heightDisplaySize.x, myPlayer[1].heightDisplaySize.y, 0.f, &player2Col, 7, "");
	p3Dist = TextManagerCreateText(posOffset.x, (float)windowHeight - posOffset.y + distOffset, myPlayer[2].heightDisplaySize.x, myPlayer[2].heightDisplaySize.y, 0.f, &player3Col, 7, "");
	p4Dist = TextManagerCreateText((float)windowWidth - posOffset.x, (float)windowHeight - posOffset.y + distOffset, myPlayer[3].heightDisplaySize.x, myPlayer[3].heightDisplaySize.y, 0.f, &player4Col, 7, "");

	if (totalPlayers >= 3)
	{
		TextManagerSetText(player3, "Player 3");
	}
	if (totalPlayers == 4)
	{
		TextManagerSetText(player4, "Player 4");
	}

}

// Update the ...
// Params:
//	 dt = Change in time (in seconds) since the last game loop.
void DisplayHeightUpdate(float dt) {
	SetDisplaySizeVariables(1, dt);

	char p1buff[50] = { 0 };
	sprintf_s(p1buff, _countof(p1buff), "%.2f ft", playerDistance[0] / TileSize);

	char p2buff[50] = { 0 };
	sprintf_s(p2buff, _countof(p2buff), "%.2f ft", playerDistance[1] / TileSize);

	char p3buff[50] = { 0 };
	sprintf_s(p3buff, _countof(p3buff), "%.2f ft", playerDistance[2] / TileSize);

	char p4buff[50] = { 0 };
	sprintf_s(p4buff, _countof(p4buff), "%.2f ft", playerDistance[3] / TileSize);

	TextManagerSetText(p1Dist, p1buff);
	TextManagerSetText(p2Dist, p2buff);

	if (totalPlayers >= 3) {
		TextManagerSetText(p3Dist, p3buff);
	}
	if (totalPlayers == 4) {
		TextManagerSetText(p4Dist, p4buff);
	}

	textStrPtr ptexts[4] = { player1, player2, player3, player4 };
	textStrPtr pdistexts[4] = { p1Dist, p2Dist, p3Dist, p4Dist };

	for (int i = 0; i < totalPlayers; ++i) {
		if (PlayerGetAlive(i) == false) {
			colorRGB color = PlayerGetColor(i);
			colorHSL hslColor = RGB2HSL(color);
			hslColor.sat = 0.4f;
			hslColor.alp = 0.5f;
			color = HSL2RGB(hslColor);
			TextManagerSetColor(ptexts[i], &color);
			TextManagerSetColor(pdistexts[i], &color);
		}

		TextManagerUpdateShadow(sPlayer[i], ptexts[i]);
		TextManagerUpdateShadow(spDist[i], pdistexts[i]);
	}
	
}

void DisplayHeightDraw()
{
	TextManagerDraw(7);

	for (int i = 0; i < totalPlayers; i++)
	{
		if (PlayerCanDoubleJump(i))
		{
			AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
			AEGfxSetBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
			AEGfxTextureSet(doubleJump, 0.0f, 0.0f);
			AEGfxSetTintColor(1.0f, 1.0f, 1.0f, 0.8f);
			switch (i)
			{
			case 0:
				AEGfxSetFullTransform(player1Pos.x, player1Pos.y, 0.f, 0.60f, 0.60f);
				break;
			case 1:
				AEGfxSetFullTransform(player2Pos.x, player2Pos.y, 0.f, 0.60f, 0.60f);
				break;
			case 2:
				AEGfxSetFullTransform(player3Pos.x, player3Pos.y, 0.f, 0.60f, 0.60f);
				break;
			case 3:
				AEGfxSetFullTransform(player4Pos.x, player4Pos.y, 0.f, 0.60f, 0.60f);
				break;
			}
			AEGfxMeshDraw(itemMesh, AE_GFX_MDM_TRIANGLES);
		}
	}
}

void DisplayHeightFree()
{
	AEGfxMeshFree(itemMesh);
	AEGfxTextureUnload(doubleJump);
}


//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

static void SetDisplaySizeVariables(bool isUpdate, float dt) {
	textStrPtr ptexts[4] = { player1, player2, player3, player4 };
	textStrPtr pdistexts[4] = { p1Dist, p2Dist, p3Dist, p4Dist };
	Vector2D sizePresets[4] = { sizePreset1, sizePreset2, sizePreset3, sizePreset4 };
	
	for (int i = 0; i < totalPlayers; i++) {
		int place = 0;

		for (int j = 0; j < totalPlayers; j++) {
			if (playerDistance[i] < playerDistance[j]) {
				place++;
			}
		}

		if (isUpdate) {
			myPlayer[i].heightDisplaySize.x = lerp(myPlayer[i].heightDisplaySize.x, sizePresets[place].x, lerpPercent * dt * frameRate);
			myPlayer[i].heightDisplaySize.y = lerp(myPlayer[i].heightDisplaySize.y, sizePresets[place].y, lerpPercent * dt * frameRate);

			TextManagerSetSize(ptexts[i], myPlayer[i].heightDisplaySize.x, myPlayer[i].heightDisplaySize.y);
			TextManagerSetSize(pdistexts[i], myPlayer[i].heightDisplaySize.x, myPlayer[i].heightDisplaySize.y);

		} else {
			myPlayer[i].heightDisplaySize = sizePresets[place];	
		}
	}
}