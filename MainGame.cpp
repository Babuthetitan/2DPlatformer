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
};

enum CutieState
{
	IDLE,
	WALKING,
	JUMPING,
};

//Width, height, vertcial velocity and speed for cutie
const float CUTIE_WIDTH = 12.0f;
const float CUTIE_HEIGHT = 12.0f;
const float CUTIE_SPEED = 3.0f;
float cutieVerticalVelocity = 3.0f;
const float FALL_THRESHOLD = DISPLAY_HEIGHT + CUTIE_HEIGHT;

//Jumping variables for cutie
bool isJumping = false;
float jumpVelocity = 1.0f;
const float jumpStrength = 20.0f; //Jump height
const float gravity = 1.0f;  //Gravity strength

//Width and height for floors
const float FLOOR_WIDTH = 50.0f;
const float FLOOR_HEIGHT = 9.0f;

//Number of floors to create
const int NUM_FLOORS = 10;
//Floor radius
const float FLOOR_RADIUS = 12.0f;
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
void ApplyGravity();
bool CheckAABBCollision(const GameObject& obj1, float left1, float right1, float top1, float bottom1);
void SetInitialCutiePosition();
void DrawAABB(const GameObject& obj, float width, float height);

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
	Play::CreateGameObject(TYPE_CUTIE, { 55, 150 }, 8, "cutie_idle_3");
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	obj_cutie.animSpeed=0.1;

	//Creating the game floors
	FloorCreation();

	//Setting the initial position of "cutie" on a floor
	SetInitialCutiePosition();
}

//Update (60 times a second!)
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
		ApplyGravity();
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

//Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}



//All my functions
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
		GameObject& obj_floor = Play::GetGameObject(i);
		Play::DrawObject(Play::GetGameObject(i));
		Play::DrawObject(obj_floor);
		DrawAABB(obj_floor, FLOOR_WIDTH, FLOOR_HEIGHT); //Draws an AABB for floors
	}

	//Draw Cutie and its AABB
	GameObject& cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	Play::DrawObject(cutie);
	DrawAABB(cutie, CUTIE_WIDTH, CUTIE_HEIGHT); //Draws an AABB around cutie

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
			Play::CreateGameObject(TYPE_FLOOR, { xPos, yPos }, FLOOR_WIDTH + FLOOR_HEIGHT, "floor");
		}
	}
}

void FloorBehaviour()
{
	//Floor behaviour going into update
	std::vector<int> FloorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);

	for (int i : FloorIDs)
	{
		GameObject& obj_floor = Play::GetGameObject(i);
		obj_floor.pos.y += FLOOR_SPEED;

		float floorLeft = obj_floor.pos.x - (FLOOR_WIDTH / 2);
		float floorRight = obj_floor.pos.x + (FLOOR_WIDTH / 2);
		float floorTop = obj_floor.pos.y - (FLOOR_HEIGHT / 2);
		float floorBottom = obj_floor.pos.y + (FLOOR_HEIGHT / 2);

		//checking if floors are out of the display area and reset their position
		if (obj_floor.pos.y > DISPLAY_HEIGHT + (FLOOR_HEIGHT / 2))
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
	if (Play::KeyDown(VK_SPACE) && !isJumping)
	{
		// Upward velocity to initiate jump
		cutieVerticalVelocity = -jumpStrength;
		isJumping = true;
		cutieState = CutieState::JUMPING;
	}
}

void ApplyGravity()
{
	//Gravity
	if (isJumping)
	{
		//Apply gravity to cutie vertical velocity
		cutieVerticalVelocity += gravity;

		//Update cutie vertical position based on gravity
		GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
		obj_cutie.pos.y += cutieVerticalVelocity;

		// Check if Cutie has landed on a floor
		std::vector<int> FloorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);
		for (int i : FloorIDs)
		{
			GameObject& obj_floor = Play::GetGameObject(i);

			float floorLeft = obj_floor.pos.x - (FLOOR_WIDTH / 2);
			float floorRight = obj_floor.pos.x + (FLOOR_WIDTH / 2);
			float floorTop = obj_floor.pos.y - (FLOOR_HEIGHT / 2);
			float floorBottom = obj_floor.pos.y + (FLOOR_HEIGHT / 2);


			//if cutie has landed
			if (CheckAABBCollision(obj_cutie, floorLeft, floorRight, floorTop, floorBottom))
			{
				// Cutie has landed on a floor
				obj_cutie.pos.y = obj_floor.pos.y - CUTIE_HEIGHT;
				isJumping = false;
				cutieState = CutieState::IDLE;
				cutieVerticalVelocity = 0.0f; // Reset vertical velocity
				break; // Exit the loop since Cutie has landed on a floor
			}
		}
	}

	if (!isJumping)
	{
		GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
		obj_cutie.pos.y += 8 * gravity; // Apply gravity

		// Check collision with floors
		std::vector<int> FloorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);
		for (int i : FloorIDs)
		{
			GameObject& obj_floor = Play::GetGameObject(i);

			float floorLeft = obj_floor.pos.x - (FLOOR_WIDTH / 2);
			float floorRight = obj_floor.pos.x + (FLOOR_WIDTH / 2);
			float floorTop = obj_floor.pos.y - (FLOOR_HEIGHT / 2);
			float floorBottom = obj_floor.pos.y + (FLOOR_HEIGHT / 2);

			// Check if the character is colliding with a floor
			if (CheckAABBCollision(obj_cutie, floorLeft, floorRight, floorTop, floorBottom))
			{
				// The character is on a floor; stop falling
				obj_cutie.pos.y = obj_floor.pos.y - CUTIE_HEIGHT;
				isJumping = false;
				cutieState = CutieState::IDLE;
				cutieVerticalVelocity = 0.0f; // Reset vertical velocity
				break; // Exit the loop since Cutie has landed on a floor
			}
		}
	}

	// Check if "cutie" falls below the display area
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	if (obj_cutie.pos.y > FALL_THRESHOLD)
	{
		// Reset the game to the start screen
		gameState = STATE_START;
	}
}

bool CheckAABBCollision(const GameObject& obj1, float left1, float right1, float top1, float bottom1)
{
	//AABB for cutie
	float cutieLeft = obj1.pos.x - (CUTIE_WIDTH / 2);
	float cutieRight = obj1.pos.x + (CUTIE_WIDTH / 2);
	float cutieTop = obj1.pos.y - (CUTIE_HEIGHT / 2);
	float cutieBottom = obj1.pos.y + (CUTIE_HEIGHT / 2);

	//Check collision

	return(cutieRight >= left1 && cutieLeft <= right1 && cutieBottom >= top1 && cutieTop <= bottom1);
}

void SetInitialCutiePosition()
{
	std::vector<int> FLoorIDs = Play::CollectGameObjectIDsByType(TYPE_FLOOR);

		if (!FLoorIDs.empty())
		{
			//Selects a random floor object
			int randomIndex = rand() % FLoorIDs.size();
			GameObject& randomFloor = Play::GetGameObject(FLoorIDs[randomIndex]);

			//Cutie will be positioned just above the selected floor
			GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
			obj_cutie.pos.x = randomFloor.pos.x;
			obj_cutie.pos.y = randomFloor.pos.y - CUTIE_HEIGHT;

			//Jump state reset
			isJumping = false;
		}
}

void DrawAABB(const GameObject& obj, float width, float height)
{
	float objLeft = obj.pos.x - (width / 2);
	float objRight = obj.pos.x + (width / 2);
	float objTop = obj.pos.y - (height / 2);
	float objBottom = obj.pos.y + (height / 2);

	Play::DrawRect(Point2D(objLeft, objTop), Point2D(objRight, objBottom), Play::cGreen);
}
