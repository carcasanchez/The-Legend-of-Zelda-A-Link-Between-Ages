#ifndef _ENEMY_H_
#define _ENEMY_H_

#include "Entity.h"
#include "j1PerfTimer.h"

enum ENEMY_TYPE
{
	NONE = 0,
	GREEN_SOLDIER,
	BLUE_SOLDIER,
	RED_SOLDIER
};

enum ENEMY_STATE
{
	PATROLING, 
	CHASING
};

class Enemy : public Entity
{
public:
	Enemy() :Entity() {};
	
	virtual bool Spawn(std::string file, iPoint pos) { return true; };
	virtual bool Update(float dt);
	virtual void OnDeath();

	void Repeal();

	//State Machine
	virtual bool Patroling(float dt);
	virtual bool Chasing(float dt);


public:
	ENEMY_STATE enemyState;
	int hostileRange;

protected:
	iPoint currentDest = { 0, 0 };

private:
	iPoint movement;
};

#endif
