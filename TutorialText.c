//------------------------------------------------------------------------------
//
// File Name:	TutorialText.c
// Author(s):	Jasmine Widgery
// Project:		Game
//
// Copyright ?2021 DigiPen (USA) Corporation.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "AEEngine.h"
#include "CS230/Vector2D.h"
#include "TutorialText.h"
#include "CS230/Sprite.h"
#include "CS230/Mesh.h"
#include "LevelMaps.h"
#include "CS230/SpriteSource.h"
#include "CS230/Transform.h"
#include "CS230/GameObject.h"

//------------------------------------------------------------------------------
// Private Consts:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Structures:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Variables:
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Private Variables:
static GameObjectPtr boostObject;
static SpritePtr boostSprite;
static AEGfxVertexList* boostMesh;

static AEGfxTexture* boost1Tex;
static AEGfxTexture* boost2Tex;
static AEGfxTexture* boost3Tex;
static AEGfxTexture* boost4Tex;

static AEGfxTexture* wallJumpTex;
static GameObjectPtr wallJumpObject;
static SpritePtr wallJumpSprite;
static SpriteSourcePtr wallJumpSS;

static SpriteSourcePtr boost1SS;
static SpriteSourcePtr boost2SS;
static SpriteSourcePtr boost3SS;
static SpriteSourcePtr boost4SS;

static float timer = 0.5f;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Private Function Declarations:
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public Functions:
//------------------------------------------------------------------------------

//Load the..
void TutorialTextLoad() {
	boost1Tex = AEGfxTextureLoad("Assets/Tutorial Boost Jump1.png");
	AE_WARNING_MESG(boost1Tex, "Failed to load boost1 texture!!");
	boost2Tex = AEGfxTextureLoad("Assets/Tutorial Boost Jump2.png");
	AE_WARNING_MESG(boost2Tex, "Failed to load boost2 texture!!");
	boost3Tex = AEGfxTextureLoad("Assets/Tutorial Boost Jump3.png");
	AE_WARNING_MESG(boost3Tex, "Failed to load boost3 texture!!");
	boost4Tex = AEGfxTextureLoad("Assets/Tutorial Boost Jump4.png");
	AE_WARNING_MESG(boost4Tex, "Failed to load boost4 texture!!");

	wallJumpTex = AEGfxTextureLoad("Assets/Tutorial Wall Jump.png");
	AE_WARNING_MESG(wallJumpTex, "Failed to load wallJumpTex texture!!");
}

// Initialize the ...
void TutorialTextInit() {
	boostSprite = SpriteCreate();
	boostObject = GameObjectCreate();

	SpriteSetUseCamera(boostSprite, true);

	boost1SS = SpriteSourceCreate(1, 1, boost1Tex);
	boost2SS = SpriteSourceCreate(1, 1, boost2Tex);
	boost3SS = SpriteSourceCreate(1, 1, boost3Tex);
	boost4SS = SpriteSourceCreate(1, 1, boost4Tex);

	GameObjectAddSprite(boostObject, boostSprite);
	SpriteSetAlpha(boostSprite, 1.0f);
	SpriteSetSpriteSource(boostSprite, boost1SS);
	boostMesh = MeshCreateQuad(0.5, 0.5, 1.f, 1.f, "boostMesh");
	SpriteSetMesh(boostSprite, boostMesh);

	TransformPtr boostTransform = TransformCreate();
	Vector2D boostTranslation;
	loadedSectionContents* sections = get_loaded_sections();
	boostTranslation.x = (SectionWidth * TileSize) / 2;
	boostTranslation.y = sections[0].y - (20.5f * TileSize) + (sections[0].SC.sectionHeight * TileSize);
	TransformSetTranslation(boostTransform, &boostTranslation);
	GameObjectAddTransform(boostObject, boostTransform);
	Vector2D boostScale;
	boostScale.x = 325.f;
	boostScale.y = 325.f;
	TransformSetScale(boostTransform, &boostScale);


	// Wall jump tutorial

	wallJumpSprite = SpriteCreate();
	wallJumpObject = GameObjectCreate();

	SpriteSetUseCamera(wallJumpSprite, true);

	wallJumpSS = SpriteSourceCreate(1, 1, wallJumpTex);

	GameObjectAddSprite(wallJumpObject, wallJumpSprite);
	SpriteSetAlpha(wallJumpSprite, 1.0f);
	SpriteSetSpriteSource(wallJumpSprite, wallJumpSS);
	SpriteSetMesh(wallJumpSprite, boostMesh);

	Vector2D wallJumpScale;
	wallJumpScale.x = 282.f;
	wallJumpScale.y = 315.f;

	Vector2D wallJumpTranslation;
	wallJumpTranslation.x = (6.f * TileSize) + (wallJumpScale.x / 2.f);
	wallJumpTranslation.y = sections[0].y + (36.f * TileSize);

	TransformPtr wallJumpTransform = TransformCreate();
	TransformSetTranslation(wallJumpTransform, &wallJumpTranslation);
	GameObjectAddTransform(wallJumpObject, wallJumpTransform);
	TransformSetScale(wallJumpTransform, &wallJumpScale);
}

// Update the ...
// Params:
//	 dt = Change in time (in seconds) since the last game loop.
void TutorialTextUpdate(float dt)
{
	/* Tell the compiler that the 'dt' variable is unused. */

	timer -= dt;
	if (timer <= 0)
	{
		if (SpriteGetSpriteSource(boostSprite) == boost1SS)
		{
			SpriteSetSpriteSource(boostSprite, boost2SS);
		}
		else if (SpriteGetSpriteSource(boostSprite) == boost2SS)
		{
			SpriteSetSpriteSource(boostSprite, boost3SS);
		}
		else if (SpriteGetSpriteSource(boostSprite) == boost3SS)
		{
			SpriteSetSpriteSource(boostSprite, boost4SS);
		}
		else if (SpriteGetSpriteSource(boostSprite) == boost4SS)
		{
			SpriteSetSpriteSource(boostSprite, boost1SS);
		}
		timer = 0.5f;
	}
		
}

void TutorialTextDraw()
{
	if (getSectionsPassed() == 0)
	{
		GameObjectDraw(boostObject);
		GameObjectDraw(wallJumpObject);
	}
}

// Shutdown the ...
void TutorialTextShutdown()
{
	GameObjectFree(&boostObject);
	GameObjectFree(&wallJumpObject);

	SpriteSourceFree(&boost1SS);
	SpriteSourceFree(&boost2SS);
	SpriteSourceFree(&boost3SS);
	SpriteSourceFree(&boost4SS);
	SpriteSourceFree(&wallJumpSS);

	AEGfxMeshFree(boostMesh);
}

void TutorialTextUnload()
{
	AEGfxTextureUnload(boost1Tex);
	AEGfxTextureUnload(boost2Tex);
	AEGfxTextureUnload(boost3Tex);
	AEGfxTextureUnload(boost4Tex);
	AEGfxTextureUnload(wallJumpTex);
}

//------------------------------------------------------------------------------
// Private Functions:
//------------------------------------------------------------------------------

