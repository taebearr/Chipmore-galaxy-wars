#include <iostream>
#include <fstream>
#include <cstdlib>
#include <random>
#include <numbers>

#include "Game.h"

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	unsigned int winX = 1280;
	unsigned int winY = 720;
	unsigned int frameLimit = 60;
	unsigned int winMode = 0;
	int fontSize = 20;
	int fontColorR = 255, fontColorG = 255, fontColorB = 255;

	std::string fontPath;
	std::string word;
	std::ifstream fin(path);

	if (fin.is_open()) {
		while (fin >> word) 
		{
			if (word == "Window")
			{
				fin >> winX >> winY >> frameLimit >> winMode;
			}
			else if (word == "Font")
			{
				fin >> fontPath >> fontSize >> fontColorR >> fontColorG >> fontColorB;
			}
			else if (word == "Player")
			{
				fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR
					>> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG
					>> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
			}
			else if (word == "Enemy")
			{
				fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX
					>> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN
					>> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
			}
			else if (word == "Bullet")
			{
				fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR
					>> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG
					>> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
			}
		}
	}

	fin.close();

	// load background
	if (!m_backgroundTexture.loadFromFile("galaxy2.jpg")) {
		std::cout << "Error!! Failed to load background texture.\n";
	}

	m_backgroundSprite.setTexture(m_backgroundTexture);

	// Scale the background sprite to fit the window
	float scaleX = static_cast<float>(winX) / m_backgroundSprite.getLocalBounds().width;
	float scaleY = static_cast<float>(winY) / m_backgroundSprite.getLocalBounds().height;
	m_backgroundSprite.setScale(scaleX, scaleY);

	// Set the background sprite's position to (0, 0)
	m_backgroundSprite.setPosition(sf::Vector2f(0, 0));

	// load ttf font and set font
	if (!m_font.loadFromFile(fontPath)) {
		std::cout << "Error!! Failed to load font.\n";
	}
	m_text.setFont(m_font);
	m_text.setCharacterSize(fontSize);
	m_text.setFillColor(sf::Color(fontColorR, fontColorG, fontColorB));
	m_text.setPosition(0, 0);
	m_text.setString("Score: " + std::to_string(m_score));

	// set up window parameters ( 0=user defined window size, 1=full window size )
	if (winMode == 0)
	{
		m_window.create(sf::VideoMode(winX, winY), "Chipmore Galaxy Wars");
		m_window.setFramerateLimit(frameLimit);
	}
	else if(winMode == 1)
	{
		auto fullscreenMode{ sf::VideoMode::getFullscreenModes() };
		m_window.create(fullscreenMode[0], "Chipmore Galaxy Wars", sf::Style::Fullscreen);
		m_window.setFramerateLimit(frameLimit);
	}

	spawnPlayer();
}

void Game::run()
{
	while (m_running)
	{
		m_entities.update();

		if (!m_paused)
		{
			sLifespan();
			sEnemySpawner();
			sMovement();
			sCollision();
		}

		sUserInput();
		sRender();

		// increment the current frame
		// may need to be moved when pause implement
		m_currentFrame++;
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
	// We create every entity by calling EntityManager.addEntity(tag)
	// This returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
	auto entity = m_entities.addEntity("player");

	// Give this entity a Transform so it spawns at (200, 200) with velocity (1, 1) and angle 0
	// Spawn at the middle of window
	float mx = m_window.getSize().x / 2.0f;
	float my = m_window.getSize().y / 2.0f;

	entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(m_playerConfig.S, m_playerConfig.S), 0.0f);
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V,
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	// Add an input component to the player so that we can use inputs
	entity->cInput = std::make_shared<CInput>();

	// Since we want this Entity to be our player, set our Game's player variable to be this Entity
	// This goes slightly against the EntityManager paradigm, but we use the player so much it's worth it
	m_player = entity;

}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	auto entity = m_entities.addEntity("enemy");

	// Give this entity a Transform so it spawns at (200, 200) with velocity (1, 1) and angle 0
	// spawn at random position
	float ex = rand() % m_window.getSize().x;
	float ey = rand() % m_window.getSize().y;

	// Randomize enemy shape vertices
	int eV = m_enemyConfig.VMIN + (std::rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN + 1));

	// Randomize enemy speed between SMIN & SMAX
	float r = (float)rand() / (float)RAND_MAX;
	float eS = m_enemyConfig.SMIN + r * (m_enemyConfig.SMAX - m_enemyConfig.SMIN);

	// Randomize enemy shape color
	int eShapeColR = 0 + (std::rand() % (255 - 0 + 1));
	int eShapeColG = 0 + (std::rand() % (255 - 0 + 1));
	int eShapeColB = 0 + (std::rand() % (255 - 0 + 1));

	entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(eS, eS), 0.0f);
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, eV, 
						sf::Color(eShapeColR, eShapeColG, eShapeColB), 
						sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
	entity->cScore = std::make_shared<CScore>(100);
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

// spawn the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> parent)
{
	// when we create the smaller enemy, we have to read the values of the original enemy
	// - spawn a number of small enemies equal to the vertices of the original enemy
	// - set each small enemy to the same color as the original, half the size
	// - small enemies are worth double points of the original enemy

	// Get the number of vertices of the original enemy
	size_t vertices = parent->cShape->circle.getPointCount();

	// Get the position of the parent enemy
	Vec2 parentPos = parent->cTransform->pos;

	// Get the velocity of the parent enemy
    Vec2 parentVelocity = parent->cTransform->velocity;

	//Set each enemy to the same color as the original, half the size
	sf::Color parentFill = parent->cShape->circle.getFillColor();
	sf::Color parentOutline = parent->cShape->circle.getOutlineColor();
	float parentThickness = parent->cShape->circle.getOutlineThickness();

	float smallEnemyRadius = parent->cShape->circle.getRadius() * 0.5f;
	float smallEnemyCollisionRadius = parent->cCollision->radius * 0.5f;

	float angle = 0;

	for (size_t i = 0; i < vertices; ++i)
	{
		auto smallEnemy = m_entities.addEntity("smallEnemy");

		// Set the score of the small enemy to double the score of the original enemy
		smallEnemy->cScore = std::make_shared<CScore>(parent->cScore->score * 2);

		// Set the shape of the small enemy
		smallEnemy->cShape = std::make_shared<CShape>(smallEnemyRadius, vertices, parentFill, parentOutline, parentThickness);

		// Set the collision radius of the small enemy
		smallEnemy->cCollision = std::make_shared<CCollision>(smallEnemyCollisionRadius);

		// Set the lifespan of the small enemy
		int smallEnemyLifeSpan = m_enemyConfig.L - 50;
		smallEnemy->cLifespan = std::make_shared<CLifespan>(smallEnemyLifeSpan);

		//Calculate the velocity
		double radians{ angle * std::numbers::pi / 180.0 };

		// Calculate x and y components of velocity
		float velX = std::cos(radians);
		float velY = std::sin(radians);

		// Set the velocity of the small enemy
		Vec2 newVelocity{ velX * parent->cTransform->velocity.x, velY * parent->cTransform->velocity.y };

		// Spawn the small enemy with calculated properties
		smallEnemy->cTransform = std::make_shared<CTransform>(parentPos, newVelocity, 0);

		// Update the angle for the next small enemy
		angle += 360.0f / vertices;
	}
}

// spawn a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	// Implement the spawning of a bullet which travels towards target
	//		 - bullet speed is given as a scalar speed
	//		 - you must set the velocity by using formula in notes

	auto bullet = m_entities.addEntity("bullet");
	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, 
						sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 
						sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);

	// Calculate velocity vector for the bullet
	Vec2 difference{ target.x - entity->cTransform->pos.x, target.y - entity->cTransform->pos.y };
	difference.normalize();
	Vec2 velocity{ m_bulletConfig.S * difference.x, m_bulletConfig.S * difference.y };

	// Add transform component to the bullet
	bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, velocity, 0);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	float angle{ 0 };

	for (int j{ 0 }; j < 15; ++j)
	{
		auto ulti = m_entities.addEntity("bullet");

		// my ulti is pink color square shape pillow
		ulti->cShape = std::make_shared<CShape>(20, 4, sf::Color(255, 160, 122),
			sf::Color(205, 92, 92), m_bulletConfig.OT);

		// TODO: implement collision and lifespan
		ulti->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
		ulti->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);

		Vec2 normalizedPos{ Vec2::normalize(m_player->cTransform->pos) };

		//Calculate the velocity
		double radians{ angle * std::numbers::pi / 180.0 };

		// Calculate x and y components of velocity
		float velX = std::cos(radians);
		float velY = std::sin(radians);

		//Scales the normalized vertor by the parents velocity
		Vec2 newVelocity{ velX * m_bulletConfig.S, velY * m_bulletConfig.S };

		ulti->cTransform = std::make_shared<CTransform>(m_player->cTransform->pos, newVelocity, 0);

		angle += 360.0f / 15.0f;
	}
}

void Game::sMovement()
{
	m_player->cTransform->velocity = { 0,0 };

	// implement player movement
	if (m_player->cInput->up) // W key
	{
		m_player->cTransform->velocity.y -= m_playerConfig.S;
	}
	
	if (m_player->cInput->down) // S key
	{
		m_player->cTransform->velocity.y += m_playerConfig.S;
	}

	if (m_player->cInput->left) // A key
	{
		m_player->cTransform->velocity.x -= m_playerConfig.S;
	}

	if (m_player->cInput->right) // D key
	{
		m_player->cTransform->velocity.x += m_playerConfig.S;
	}

	for (auto e : m_entities.getEntities())
	{
		if (e->tag() == "player")
		{
			// update position of the player
			m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
			m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

			// rotates the player
			e->cTransform->angle += 2.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
		}
		else if (e->cTransform)
		{
			// update the position of entities
			e->cTransform->pos += e->cTransform->velocity;

			// rotates the entity
			e->cTransform->angle += 2.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
		}
	}
}

void Game::sRender()
{
	m_window.clear();

	m_window.draw(m_backgroundSprite);

	for (auto e : m_entities.getEntities())
	{
		// set the position of the shape based on the entity's transform->pos
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

		// set the rotation of the shape based on the entity's transform->angle
		e->cTransform->angle += 2.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);

		m_window.draw(e->cShape->circle);
	}

	m_window.draw(m_text);
	m_window.display();
}

void Game::sLifespan()
{
	// for all entities
	//		if entity has no lifespan component, skip it
	//		if entity has > 0 remaining lifespan, subtract 1
	//		if it has lifespan and is alive
	//			scale its alpha channel properly
	//		if it has lifespan and its time is up
	//			destroy the entity

	for (auto e : m_entities.getEntities())
	{
		if (!e->cLifespan)
		{
			continue;
		}

		if (e->cLifespan->remaining > 0)
		{
			e->cLifespan->remaining--;
		}

		if (e->isActive() && e->cLifespan->remaining > 0)
		{
			float alphaMultiplier{ static_cast<float>(e->cLifespan->remaining) / static_cast<float>(e->cLifespan->total) };

			auto fillColor{ e->cShape->circle.getFillColor() };
			sf::Color newFillColor{ fillColor.r,fillColor.g,fillColor.b, static_cast<sf::Uint8>(255 * alphaMultiplier) };
			e->cShape->circle.setFillColor(newFillColor);

			auto outlineColor{ e->cShape->circle.getOutlineColor() };
			sf::Color newOutlineColor{ outlineColor.r,outlineColor.g,outlineColor.b, static_cast<sf::Uint8>(255 * alphaMultiplier) };
			e->cShape->circle.setOutlineColor(newOutlineColor);

		}
		else if (e->cLifespan->remaining <= 0)
		{
			e->destroy();
		}
	}
}

void Game::sCollision()
{
	//		(use m_currentFrame - m_lastEnemySpawnTime) to determine
	//		how long it has been since the last enemy spawned

	// Case 1: collision between player and enemy
	// destroy player, destroy enemy, respawn player
	for (auto player : m_entities.getEntities("player"))
	{
		// Skip if player is not active
		if (!player->isActive())
			continue;

		for (auto enemy : m_entities.getEntities("enemy"))
		{
			Vec2 diff{ enemy->cTransform->pos.x - player->cTransform->pos.x , enemy->cTransform->pos.y - player->cTransform->pos.y };
			double collisionRadiusSQ{ (player->cCollision->radius + enemy->cCollision->radius) * (player->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				//makes sure the player is alive and doesnt spawn 2 players
				if (player->isActive())
				{
					m_score = 0;
					m_text.setString("Score: " + std::to_string(m_score));
					std::cout << "m_score = " << m_score << "\n";

					enemy->destroy();
					player->destroy();
					spawnPlayer();
				}
			}
		}

		// Case 2: collision between player and small enemy
		// destroy player, destroy enemy, respawn player
		for (auto enemy : m_entities.getEntities("smallEnemy"))
		{
			Vec2 diff{ enemy->cTransform->pos.x - player->cTransform->pos.x , enemy->cTransform->pos.y - player->cTransform->pos.y };
			double collisionRadiusSQ{ (player->cCollision->radius + enemy->cCollision->radius) * (player->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				//makes sure the player is alive so it doesnt spawn 2 players
				if (player->isActive())
				{
					m_score /= 2;
					m_text.setString("Score: " + std::to_string(m_score));
					std::cout << "m_score = " << m_score << "\n";

					player->destroy();
					enemy->destroy();
					spawnPlayer();
				}
			}
		}
	}

	// Case 3: collision between bullet and enemy
	// destroy the bullet, destroy the enemy, spawn small enemy
	for (auto bullet : m_entities.getEntities("bullet"))
	{
		for (auto enemy : m_entities.getEntities("enemy"))
		{
			Vec2 diff{ enemy->cTransform->pos.x - bullet->cTransform->pos.x , enemy->cTransform->pos.y - bullet->cTransform->pos.y };
			double collisionRadiusSQ{ (bullet->cCollision->radius + enemy->cCollision->radius) * (bullet->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				//Updates the score
				m_score += enemy->cScore->score;
				m_text.setString("Score: " + std::to_string(m_score));
				std::cout << "m_score = " << m_score << "\n";

				spawnSmallEnemies(enemy);
				bullet->destroy();
				enemy->destroy();

				break;
			}
		}

		// Case 4: collision between bullet and small enemy
		// destroy the bullet, destroy the small enemy
		for (auto enemy : m_entities.getEntities("smallEnemy"))
		{
			Vec2 diff{ enemy->cTransform->pos.x - bullet->cTransform->pos.x , enemy->cTransform->pos.y - bullet->cTransform->pos.y };
			double collisionRadiusSQ{ (bullet->cCollision->radius + enemy->cCollision->radius) * (bullet->cCollision->radius + enemy->cCollision->radius) };
			double distSQ{ (diff.x * diff.x) + (diff.y * diff.y) };

			if (distSQ < collisionRadiusSQ)
			{
				m_score += enemy->cScore->score;
				m_text.setString("Score: " + std::to_string(m_score));
				std::cout << "m_score = " << m_score << "\n";

				bullet->destroy();
				enemy->destroy();

				break;
			}
		}
	}

	//General Collision ie walls && ground && ceiling for player
	for (auto e : m_entities.getEntities("player"))
	{
		//Checks to see if player collided with walls
		if (e->cTransform->pos.x + m_playerConfig.CR > m_window.getSize().x)
		{
			e->cTransform->pos.x -= m_playerConfig.S;
		}
		else if (e->cTransform->pos.x - m_playerConfig.CR < 0)
		{
			e->cTransform->pos.x += m_playerConfig.S;
		}

		if (e->cTransform->pos.y + m_playerConfig.CR > m_window.getSize().y)
		{
			e->cTransform->pos.y -= m_playerConfig.S;
		}
		else if (e->cTransform->pos.y - m_playerConfig.CR < 0)
		{
			e->cTransform->pos.y += m_playerConfig.S;
		}
	}

	//General Collision ie walls && ground && ceiling for entities
	for (auto e : m_entities.getEntities())
	{
		if (e->tag() == "enemy")
		{
			if (e->cTransform->pos.x + e->cCollision->radius > m_window.getSize().x)
			{
				e->cTransform->velocity.x *= -1;
			}
			else if (e->cTransform->pos.x - e->cCollision->radius < 0)
			{
				e->cTransform->velocity.x *= -1;
			}
			if (e->cTransform->pos.y + e->cCollision->radius > m_window.getSize().y)
			{
				e->cTransform->velocity.y *= -1;
			}
			else if (e->cTransform->pos.y - e->cCollision->radius < 0)
			{
				e->cTransform->velocity.y *= -1;
			}
		}
	}
}

void Game::sEnemySpawner()
{
	if ((m_currentFrame - m_lastEnemySpawnTime) >= m_enemyConfig.SI)
	{
		spawnEnemy();
	}
}

void Game::sUserInput()
{
	//		 note that you should only be setting the player's input component variables here
	//		 you should not implement the player's movement logic here
	//		 the movement system will read the variables you set in this function

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		// this event triggers when the window is closed
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W: // Up key
				std::cout << "W Key Pressed\n";
				m_player->cInput->up = true;
				break;
			case sf::Keyboard::A: // Left key
				std::cout << "A Key Pressed\n";
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::S: // Down key
				std::cout << "S Key Pressed\n";
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::D: // Right key
				std::cout << "D Key Pressed\n";
				m_player->cInput->right = true;
				break;
			case sf::Keyboard::P:
				std::cout << "P Key Pressed\n";
				m_paused ? setPaused(false) : setPaused(true);
				break;
			default:break;
			}
		}

		// this event is triggered when a key is released
		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				std::cout << "W Key Released\n";
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::A:
				std::cout << "A Key Released\n";
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::S:
				std::cout << "S Key Released\n";
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::D:
				std::cout << "D Key Released\n";
				m_player->cInput->right = false;
				break;
			default:break;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
				//call spawnBullet here
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right)
			{
				std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
				//call spawnSpecialWeapon here
				spawnSpecialWeapon(m_player);
			}
		}
	}

}
