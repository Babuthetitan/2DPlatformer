#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr int DISPLAY_WIDTH { 1280 };
constexpr int DISPLAY_HEIGHT { 720 };
constexpr int DISPLAY_SCALE{ 1 };

enum GameObjectType
{
	TYPE_CUTIE = 0,
	TYPE_FLOOR = 1,
	TYPE_ALIEN= 2,
	TYPE_GHOST= 3,
};

enum CutieState
{
	
};

//Width and height for cutie
const float CUTIE_WIDTH = 100.0f;
const float CUTIE_HEIGHT = 100.0f;

//Width and height for floors
const float FLOOR_WIDTH = 5.0f;
const float FLOOR_HEIGHT = 5.0f;

//Number of floors to create
const int NUM_FLOORS = 10;
//Floor radius
const float FLOOR_RADIUS = 20.0f;
//Floor speed
const float FLOOR_SPEED = 2.0f;


//Function declarartion
void Draw();
void FloorCreation();
void FloorBehaviour();
void UpdateCutie();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins(); //obj.pos = center of a sprite
	Play::LoadBackground("Data\\Backgrounds\\dusk.jpg");
	//Play::StartAudioLoop("music");
	

	//Object Creation
	Play::CreateGameObject(TYPE_CUTIE, { 85, 75 }, 50, "cutie_idle_3");
	GameObject& obj_cutie = Play::GetGameObjectByType(TYPE_CUTIE);
	obj_cutie.animSpeed=0.1;
	

	FloorCreation();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Draw();
	FloorBehaviour();
	UpdateCutie();

	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
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
	Play::DrawFontText("64px", "Use arrow keys to control Cutie & space bar to jump", Point2D(DISPLAY_WIDTH / 2, 650), Play::CENTRE);



	Play::PresentDrawingBuffer();
}

void FloorCreation()
{
	//Creating the floors going into entry
	for (int i = 0; i < NUM_FLOORS; i++)
	{
		float xPos = static_cast<float>(rand() % DISPLAY_WIDTH);
		float yPos = static_cast<float>(rand() % DISPLAY_HEIGHT);
		Play::CreateGameObject(TYPE_FLOOR, { xPos,yPos }, FLOOR_RADIUS, "floor");
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

		//Check if the floors are out of the display area and rest their position
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
	Play::UpdateGameObject(obj_cutie) ;
}