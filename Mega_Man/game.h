#pragma once
#include<SFML/Graphics.hpp>
#include"Entitymanager.h"
#include"assets.h"


class Game
{

	// Window Variables
	sf::RenderWindow m_window;
	sf::Clock m_clock;
	bool m_running = true;
	bool m_paused = false;

	EntityManager m_entities;
	Assets m_assets;

	EntityID m_playerID;

	void init();    //Setting up intial states
	void update(sf::Time dt);  // updating states every frame

	//System Handling
	void sInput();
	void sRender();
	void sMovement(sf::Time dt);
	void sCollision();

	//Animation handling
	void sAnimation(sf::Time dt);


	void spawnPlayer(float x,float y);
	void spawnBullet();
	void loadLevel(const std::string &levelPath);

public:

	Game();
	void run();
};