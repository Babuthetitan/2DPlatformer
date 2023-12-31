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
	TYPE_GHOST = 2,
};

enum CutieState
{
	IDLE,
	WALKING,
	JUMPING,
};

enum GhostState
{
	IDLE1,
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
const float jumpStrength = 17.0f; //Jump height
const float gravity = 1.0f;  //Gravity strength

//Width and height for floors
const float FLOOR_WIDTH = 50.0f;
const float FLOOR_HEIGHT = 9.0f;

//Number of floors to create
const int NUM_FLOORS = 30;
//Floor radius
const float FLOOR_RADIUS = 12.0f;
//Floor speed
const float FLOOR_SPEED = 0.5f;
//Increasing the floor speed constants
const float FLOOR_SPEED_THRESHOLD = 70.0f; //Score threshold
const float INCREASED_FLOOR_SPEED = 1.0f; //New floor speed
float currentFloorSpeed = FLOOR_SPEED; //Initial floor speed

//Score count tracker variable
int score = 0;

//Last platform cutie has landed on
GameObject* lastLandedPlatform = nullptr;

//Ghost variables
float ghostSpeed = 1.0f; //Adjustable ghostspeed
const int GHOST_FOLLOW_THRESHOLD = 30;

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
void RestartAfterDeath();
void UpdateGhost();
void GhostFollow(GameObject& ghost, const GameObject& cutie, float speed);
void CheckGhostCollisionWithPlayer();
void SetInitialGhostPosition();
void DrawAABB(const GameObject& obj, float width, float height);
void GameOverScreen();

//Flags to control game state and global variables
GameState gameState = STATE_START;
bool enterPressed = false;
CutieState cutieState = IDLE; //cutie starts on an idle state
GhostState ghostState = IDLE1; //ghost starts on an idle state

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

	//ghost enemy object creation
	Play::CreateGameObject(TYPE_GHOST, { -100, -100 }, 8, "ghost_enemy_4");
	GameObject& obj_ghost = Play::GetGameObjectByType(TYPE_GHOST);
	obj_ghost.animSpeed = 0.1;


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
		GameObject& obj_ghost = Play::GetGameObjectByType(TYPE_GHOST);
		GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);

    	CutieControls();
		UpdateCutie();

		//GHOST BEHAVIOR
		if (score >= GHOST_FOLLOW_THRESHOLD)
		{
			GhostFollow(obj_ghost, obj_cutie, 1.0f);
			CheckGhostCollisionWithPlayer();
		}
		
		UpdateGhost();
		ApplyGravity();
		UpdateDraw();
		FloorBehaviour();
		RestartAfterDeath();	
	}
	
	//Gameplay ends over here
	if (gameState == STATE_OVER)
	{
		UpdateDraw();
		
		//Checks if enter key is pressed to restart
		if (Play::KeyDown(VK_RETURN))
		{
			//Reset game state and other variables
			gameState = STATE_START;
			score = 0;
			SetInitialCutiePosition();
			SetInitialGhostPosition();
			Play::StartAudioLoop("music");
		}
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
		//Play::PlayAudio("DITP"); //Plays a scary DITP tone when the game starts, (Diversity Internship Training Programme)
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
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawFontText("32px", "Welcome to Cute Jumper", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 - 50), Play::CENTRE);
	Play::SetDrawingSpace(Play::WORLD);

	// Prompt to press Enter
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawFontText("64px", "Press Enter to Start", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 50), Play::CENTRE);
	Play::SetDrawingSpace(Play::WORLD);

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

	//Draw Ghost
	GameObject& ghost = Play::GetGameObjectByType(TYPE_GHOST);
	Play::DrawObject(ghost);

	//Game controls text
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawFontText("64px", "Use arrow keys to control Cutie & space bar to jump", Point2D(DISPLAY_WIDTH / 2,570), Play::CENTRE);
	Play::SetDrawingSpace(Play::WORLD);

	//score count
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawFontText("64px", "Score: " + std::to_string(score), Point2D(DISPLAY_WIDTH - 100,20), Play::RIGHT);
	Play::SetDrawingSpace(Play::WORLD);
	
	//GameOverScreen
	if (gameState == STATE_OVER)
	{
		GameOverScreen();
	}
	
	Play::PresentDrawingBuffer();
}

void FloorCreation()
{
	//Creating the floors going into entry
	{
		//Creating the floors from the top
		float yPos = 0.0f;

		for (int i = 0; i < NUM_FLOORS; i++)
		{
			float xPos = static_cast<float>(rand() % DISPLAY_WIDTH);
			Play::CreateGameObject(TYPE_FLOOR, { xPos, yPos }, FLOOR_WIDTH + FLOOR_HEIGHT, "floor");

			// Increase the yPos for the next floor
			yPos += FLOOR_HEIGHT + 12.0f; //Adjusting the vertical gap between floors here
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
		obj_floor.pos.y += FLOOR_SPEED; //moves the floors downward
		
		obj_floor.pos.y += currentFloorSpeed; // Update the position of the floors using currentFloorSpeed

		

		float floorLeft = obj_floor.pos.x - (FLOOR_WIDTH / 2);
		float floorRight = obj_floor.pos.x + (FLOOR_WIDTH / 2);
		float floorTop = obj_floor.pos.y - (FLOOR_HEIGHT / 2);
		float floorBottom = obj_floor.pos.y + (FLOOR_HEIGHT / 2);

		//Checking if the floors have gone below the display area
		if (obj_floor.pos.y > DISPLAY_HEIGHT + (FLOOR_HEIGHT / 2))
		{
			//Wrapping the floors back to the top of the screen
			obj_floor.pos.y = -FLOOR_RADIUS;

			//Randomly repositions the floors horizontally
			obj_floor.pos.x = static_cast<float>(rand() % DISPLAY_WIDTH);
		}

		// Check the current score and adjust the floor speed if needed
		if (score >= FLOOR_SPEED_THRESHOLD)
		{
			currentFloorSpeed = INCREASED_FLOOR_SPEED;
		}
		else
		{
		    currentFloorSpeed = FLOOR_SPEED;
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

	Play::UpdateGameObject(obj_cutie);
}

void UpdateGhost()
{
	GameObject& obj_ghost = Play::GetGameObjectByType(TYPE_GHOST);

	switch (ghostState)
	{
	case GhostState::IDLE1:
		obj_ghost.spriteId = Play::GetSpriteId("ghost_enemy_4");
		break;
	}
	
	Play::UpdateGameObject(obj_ghost);
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
	if (Play::KeyPressed(VK_SPACE) && !isJumping)
	{
		Play::PlayAudio("jump"); //Jump sound effect
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


			//if cutie has landed on top of a floor
			if (CheckAABBCollision(obj_cutie, floorLeft, floorRight, floorTop, floorBottom))
			{

				//Checking if cutie has landed on a different platform
				if (&obj_cutie != lastLandedPlatform)
				{
					//Cutie has landed on a new floor, increase the score
					score++;
					lastLandedPlatform = &obj_floor;
				}

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

void RestartAfterDeath()
{
	// Check if "cutie" falls below the display area
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	if (obj_cutie.pos.y > FALL_THRESHOLD)
	{
		// Reset the game to the start screen
		gameState = STATE_OVER;

		//Game music
		Play::StopAudioLoop("music");
		
		//Game over sound effect
		Play::PlayAudio("death");
	}
}

void GhostFollow(GameObject& ghost, const GameObject& cutie, float speed)
{
	GameObject& obj_ghost = Play::GetGameObjectByType(TYPE_GHOST);
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);

	float dx = cutie.pos.x - ghost.pos.x;
	float dy = cutie.pos.y - ghost.pos.y;

	//Normalize the direction
	float length = sqrt(dx * dx + dy * dy);
	if (length != 0.0f)
	{
		dx /= length;
		dy /= length;
	}

	//Ghost position update
	ghost.pos.x += speed * dx;
	ghost.pos.y += speed * dy;
}

void CheckGhostCollisionWithPlayer()
{
	GameObject& obj_ghost = Play::GetGameObjectByType(TYPE_GHOST);
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);

	// Check for collision between ghost and player
	if (CheckAABBCollision(obj_ghost, obj_cutie.pos.x - (CUTIE_WIDTH / 2), obj_cutie.pos.x + (CUTIE_WIDTH / 2), obj_cutie.pos.y - (CUTIE_HEIGHT / 2), obj_cutie.pos.y + (CUTIE_HEIGHT / 2)))
	{
		gameState = STATE_OVER; // Transition to the game over state
		Play::StopAudioLoop("music"); // Stop game music
		Play::PlayAudio("death"); // Play game over sound effect
	}
}

void SetInitialGhostPosition()
{
	GameObject& obj_ghost = Play::GetGameObjectByType(TYPE_GHOST);

	//Initial position of the ghost
	obj_ghost.pos.x = -100;
	obj_ghost.pos.y = -100;
}

void DrawAABB(const GameObject& obj, float width, float height)
{
	float objLeft = obj.pos.x - (width / 2);
	float objRight = obj.pos.x + (width / 2);
	float objTop = obj.pos.y - (height / 2);
	float objBottom = obj.pos.y + (height / 2);

	Play::DrawRect(Point2D(objLeft, objTop), Point2D(objRight, objBottom), Play::cGreen);
}

void GameOverScreen()
{
	//Displays the game oevr screen
    Play::DrawBackground;

	//Text to show that the game is over
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawFontText("32px", "Game Over!! :-)", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 - 50), Play::CENTRE);
	Play::DrawFontText("64px", "Press Enter to Play Again", Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 50), Play::CENTRE);
	Play::SetDrawingSpace(Play::WORLD);

	//Draw Cutie
	GameObject& cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	Play::DrawObject(cutie);

	Play::PresentDrawingBuffer;
}