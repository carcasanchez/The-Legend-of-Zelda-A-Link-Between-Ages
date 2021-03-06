#ifndef _GAME_LAYER_H_
#define _GAME_LAYER_H_

#include "j1Module.h"
#include <list>
#include "j1Timer.h"
#include <map>

using namespace std;

class Entity;
class j1EntityManager;
class Hud;
class Collider;
class Object;


class j1GameLayer : public j1Module
{
public:
	j1GameLayer();
	~j1GameLayer();

	bool Awake(pugi::xml_node& conf);
	bool Start();
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	void PickObject(Object* object);
	void BuyObject(Object* object);

	//Deactives hud loading
	void StopLoading();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	bool DebugFunc();

	bool On_Collision_Callback(Collider*, Collider*, float dt);



public:

	j1EntityManager*	em = NULL;
	Hud*				hud = NULL;

	bool        pause = false;
	bool		quit_game = false;

	int playerX = 9;
	int playerY = 5;

	j1Timer buy_timer; 

	std::vector<int> openChests;

	bool debug = false;


};



#endif // !_GAME_LAYER_H_