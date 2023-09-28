#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr int DISPLAY_WIDTH { 800 };
constexpr int DISPLAY_HEIGHT { 600 };
constexpr int DISPLAY_SCALE{ 1 };

enum GameState
{
	STATE_START,
	STATE_PLAY,
	STATE_OVER,
};

enum GameObjectType
{
	TYPE_CUTIE = 0,
	TYPE_FLOOR = 1,
	TYPE_ALIEN= 2,
	TYPE_GHOST= 3,
};

enum CutieState
{
	IDLE,
	WALKING,
	JUMPING,
};


//Width, height and speed for cutie
const float CUTIE_WIDTH = 100.0f;
const float CUTIE_HEIGHT = 100.0f;
const float CUTIE_SPEED = 3.0f;

//Jumping variables for cutie
bool isJumping = false;
float jumpVelocity = 0.0f;
const float jumpStrength = 15.0f; //Jump height
const float gravity = 1.0f;  //Gravity strength

//Width and height for floors
const float FLOOR_WIDTH = 5.0f;
const float FLOOR_HEIGHT = 5.0f;

//Number of floors to create
const int NUM_FLOORS = 10;
//Floor radius
const float FLOOR_RADIUS = 20.0f;
//Floor speed
const float FLOOR_SPEED = 0.0f;


//Function declarartion
void StartGameLogic();
void MainMenuDraw();
void UpdateDraw();
void FloorCreation();
void FloorBehaviour();
void UpdateCutie();
void CutieControls();

//Flags to control game state and global variables
GameState gameState = STATE_START;
bool enterPressed = false;
CutieState cutieState = IDLE; //starts on an idle state

//Entry
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins(); //obj.pos = center of a sprite
	Play::LoadBackground("Data\\Backgrounds\\newlife.jpg");
	Play::StartAudioLoop("music");

	//Cutie Object Creation
	Play::CreateGameObject(TYPE_CUTIE, { 85, 75 }, 50, "cutie_idle_3");
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	obj_cutie.animSpeed=0.1;

	FloorCreation();
}

// Update (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	//Main Menu Screen
	if (gameState == STATE_START)
	{
		StartGameLogic();
		MainMenuDraw();
		UpdateCutie();

		return false; //holds the start screen until enter is pressed
		
	}

	//Gameplay begins over here
	if (gameState == STATE_PLAY)
	{
		CutieControls();
		UpdateCutie();
		UpdateDraw();
		FloorBehaviour();
	}
	
	//Gameplay ends over here
	if (gameState == STATE_OVER)
	{
		// Add cool game ending stuff
	}

	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void StartGameLogic()
{
	//Check if enter key has been pressed to start the game
	if (Play::KeyDown(VK_RETURN))
	{
		gameState = STATE_PLAY;
		enterPressed = true;
		
	}

	//More cool stuff to add when enter is presed
	if (enterPressed)
	{
		Play::PlayAudio("DITP voice over");
	}

}

void MainMenuDraw()
{
	//This MainMenuDarw function goes into the STATE_START game state

	// Draw background for the start screen
	Play::DrawBackground();

	//Draw Cutie
	GameObject& cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	Play::DrawObject(cutie);

	// Display a welcome message
	Play::DrawFontText("32px", "Welcome to Cute Jumper", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 - 50), Play::CENTRE);

	// Prompt to press Enter
	Play::DrawFontText("64px", "Press Enter to Start", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 50), Play::CENTRE);

	Play::PresentDrawingBuffer();
}

void UpdateDraw()
{
    Play::DrawBackground();

	//Drawing the floors
	Play::CollectGameObjectIDsByType(TYPE_FLOOR);
	std::vector<int> FloorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);
	for (int i: FloorIDs)
	{
		Play::DrawObject(Play::GetGameObject(i));
		Play::GetGameObject(i);
	}

	//Draw Cutie
	GameObject& cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	Play::DrawObject(cutie);

	//Game controls text
	Play::DrawFontText("64px", "Use arrow keys to control Cutie & space bar to jump", Point2D(DISPLAY_WIDTH / 2,570), Play::CENTRE);

	Play::PresentDrawingBuffer();
}

void FloorCreation()
{
	//Creating the floors going into entry
	{
		for (int i = 0; i < NUM_FLOORS; i++)
		{
			float xPos = static_cast<float>(rand() % DISPLAY_WIDTH);
			float yPos = static_cast<float>(rand() % DISPLAY_HEIGHT);
			Play::CreateGameObject(TYPE_FLOOR, { xPos, yPos }, FLOOR_RADIUS, "floor");
		}
	}
}

void FloorBehaviour()
{
	//Floor behaviour going into update
	std::vector<int> FloorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);
	for (int i : FloorIDs)
	{
		GameObject& obj_floor = Play::GetGameObject(i);
		obj_floor.pos.y += FLOOR_SPEED;

		//checking if Gems are out of the display area and reset their position
		if (obj_floor.pos.y > DISPLAY_HEIGHT + FLOOR_RADIUS)
		{
			obj_floor.pos.y = -FLOOR_RADIUS;
			obj_floor.pos.x = static_cast<float>(rand() % DISPLAY_WIDTH);
		}
	}
}

void UpdateCutie()
{
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);

	switch (cutieState)
	{
	case CutieState::IDLE:
		obj_cutie.spriteId = Play::GetSpriteId("cutie_idle_3");
		break;

	case CutieState::WALKING:
		//checking between left or right
		if (Play::KeyDown(VK_LEFT))
		{
			obj_cutie.spriteId = Play::GetSpriteId("cutie_walkleft_3");
		}
		else if (Play::KeyDown(VK_RIGHT))
		{
			obj_cutie.spriteId = Play::GetSpriteId("cutie_walk_3");
		}
		else
		{
			obj_cutie.spriteId = Play::GetSpriteId("cutie_idle_3");
		}
		break;

	case CutieState::JUMPING:
		obj_cutie.spriteId = Play::GetSpriteId("cutie_jump_3");
		break;
	}


	Play::UpdateGameObject(obj_cutie) ;
}

bool IsColliding(const GameObject& obj1, const GameObject& obj2)
{
	// boundaries of obj1
	float left1 = obj1.pos.x;
	float right1 = obj1.pos.x + obj1.radius * 2;
	float top1 = obj1.pos.y;
	float bottom1 = obj1.pos.y + obj1.radius * 2;

	// boundaries of obj2
	float left2 = obj2.pos.x;
	float right2 = obj2.pos.x + obj2.radius * 2;
	float top2 = obj2.pos.y;
	float bottom2 = obj2.pos.y + obj2.radius * 2;

	// Checking for collision
	return (left1 < right2 && right1 > left2 && top1 < bottom2 && bottom1 > top2);
}

void CutieControls()
{
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);

	if (Play::KeyDown(VK_RIGHT))
	{
		obj_cutie.pos.x += CUTIE_SPEED;
		cutieState = CutieState::WALKING; //Sets the state to walking
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		obj_cutie.pos.x -= CUTIE_SPEED;
		cutieState = CutieState::WALKING; //Sets the state to walking

		obj_cutie.spriteId = Play::GetSpriteId("cutie_walkleft_3"); // change to left facing sprite
	}
	else
	{
		cutieState = CutieState::IDLE; //Sets to idle when nothing happens
	}

	//Handling the jumping mechanics
	if (Play::KeyDown(VK_SPACE && !isJumping))
	{
		// Upward velocity to initiate jump
		jumpVelocity = -jumpStrength;
		isJumping = true;
		cutieState = CutieState::JUMPING;
	}

	//Gravity
	if (isJumping)
	{
		//Apply gravity to jump velocity
		jumpVelocity += gravity;

		//Update cutie vertical position based on jump velocity
		obj_cutie.pos.y += jumpVelocity;

		//if cutie has landed
		if (obj_cutie.pos.y >= DISPLAY_HEIGHT - CUTIE_HEIGHT)
		{
			obj_cutie.pos.y = DISPLAY_HEIGHT - CUTIE_HEIGHT;
			isJumping = false;
			CutieState::IDLE; // sets state to idle when landing
		}
	}

	//Checking for floor Collisions
	bool onFloor = false;
	std::vector<int>FloorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);
	for (int i :FloorIDs)
	{
		GameObject& obj_floor = Play::GetGameObject(i);
		if (IsColliding(obj_cutie, obj_floor))
		{
			onFloor = true;
			break; //Cutie is on floor, do not check other floors
		}
	}

	//Applying gravity when not on a floor and reseting jump state when on a floor
	if (!onFloor)
	{
		obj_cutie.pos.y += gravity;
	}
	else
	{
		isJumping = false;
		cutieState = CutieState::IDLE;
	}
}
