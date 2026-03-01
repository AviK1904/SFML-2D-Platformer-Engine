#include"game.h"
#include<iostream>
#include<fstream>

Game::Game()
	: m_running(true), m_paused(false)
{
	init();
}

void Game::run()
{
	m_clock.restart();
	while (m_running)
	{
		sf::Time dt = m_clock.restart();

		sInput();

		if (!m_paused)
		{
			update(dt);
			sAnimation(dt);
		}

		sRender();
	}
}

void Game::init()
{
	m_window.create(sf::VideoMode({ 1280, 720 }), "Mega Man");
	m_window.setFramerateLimit(60);

	m_window.requestFocus();

	m_assets.loadFromFile("assets.txt");
	loadLevel("level1.txt");

}

void Game::update(sf::Time dt)
{
	m_entities.update();

	//Run systems
	sMovement(dt);
	sCollision();
}

void Game::sInput()
{
	while (std::optional event = m_window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			m_running = false;
		}

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
				m_running = false;

			if (keyPressed->scancode == sf::Keyboard::Scancode::P)
			{
				m_paused = !m_paused;
			}
		}

		if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>())
		{
			auto& input = m_entities.getComponent<CInput>(m_playerID);
			if (mousePress->button == sf::Mouse::Button::Left)
			{
				input.shoot = true;

				// int clickX = mousePress->position.x;
				// int clickY = mousePress->position.y;
			}
		}

	}

	for (auto id : m_entities.getEntities("player"))
	{
		auto& input = m_entities.getComponent<CInput>(id);

		input.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
		input.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
		input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
		input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);


	}
}

void Game::sRender()
{
	m_window.clear(sf::Color(35, 35, 40));

	// --- DRAW THE VISUAL SPRITES ---
	for (const auto& entityDesc : m_entities.getAllEntities())
	{
		if (!entityDesc.active) continue; // Skip dead entities

		EntityID id = entityDesc.id;

		// Does it have a position and a picture?
		if (m_entities.hasComponent<CTransform>(id) && m_entities.hasComponent<CSprite>(id))
		{
			auto& transform = m_entities.getComponent<CTransform>(id);
			auto& cSprite = m_entities.getComponent<CSprite>(id);

			// Open the std::optional box safely
			if (cSprite.sprite.has_value())
			{
				auto& sprite = cSprite.sprite.value();

				// Update the sprite's position on the screen
				sprite.setPosition(transform.pos);
				sprite.setScale(transform.scale);

				// Draw it to the window!
				m_window.draw(sprite);
			}
		}
	}

	// --- DEBUG HITBOX DRAWING ---
	bool drawHitboxes = true; // Change this to false when you want to play normally!

	if (drawHitboxes)
	{
		for (const auto& entityDesc : m_entities.getAllEntities())
		{
			if (!entityDesc.active) continue;
			EntityID id = entityDesc.id;

			// If the entity has a Transform AND a BoundingBox, draw the box!
			if (m_entities.hasComponent<CBoundingBox>(id) && m_entities.hasComponent<CTransform>(id))
			{
				auto& transform = m_entities.getComponent<CTransform>(id);
				auto& box = m_entities.getComponent<CBoundingBox>(id);

				sf::RectangleShape rect;
				rect.setSize(box.size);
				rect.setFillColor(sf::Color::Transparent); // Inside is clear
				rect.setOutlineThickness(2.0f);

				// Because Mega Man's origin is bottom-center, we must shift his debug rectangle
				// up and to the left so it matches the physics math!
				if (entityDesc.tag == "player")
				{
					rect.setPosition({ transform.pos.x - box.halfSize.x, transform.pos.y - box.size.y });
					rect.setOutlineColor(sf::Color::Red); // Make player red to stand out
				}
				// Tiles spawn from the top-left, so their rect position is exactly their transform position
				else
				{
					rect.setPosition({ transform.pos.x, transform.pos.y });
					rect.setOutlineColor(sf::Color::Green); // Make tiles green
				}

				m_window.draw(rect);
			}
		}
	}

	m_window.display();
}

void Game::sMovement(sf::Time dt)
{
	//  Create a flag outside the loop
	bool playerShotThisFrame = false;

	for (auto id : m_entities.getEntities("player"))
	{
		auto& input = m_entities.getComponent<CInput>(id);
		auto& transform = m_entities.getComponent<CTransform>(id);
		auto& state = m_entities.getComponent<CState>(id);
		auto& anim = m_entities.getComponent<CAnimation>(id);

		transform.prevPos = transform.pos;

		// --- 1. PHYSICS & INPUT ---
		//float floorHeight = 300.f;
		transform.vel.x = 0.f;
		float groundBuffer = 2.f;

		bool isGrounded = state.isGrounded;

		state.isGrounded = false;

		if (input.left) { transform.vel.x -= transform.speed; transform.scale.x = -1.7f; }
		if (input.right) { transform.vel.x += transform.speed; transform.scale.x = 1.7f; }
		if (input.up && isGrounded) { transform.vel.y = -transform.jumpSpeed; }

		transform.vel.y += transform.gravity * dt.asSeconds();
		transform.pos += transform.vel * dt.asSeconds();

		/*if (transform.pos.y >= floorHeight)
		{
			transform.pos.y = floorHeight;
			transform.vel.y = 0;
		}*/

		//  CALCULATE NEW STATE ---
		State newState = state.state; // Default to current
		State oldState = state.state; // Save this for the animation bridge later

		if (input.shoot)
		{
			input.shoot = false;
			state.shootTimer = .4f;

			// FLAG IT, BUT DO NOT CALL spawnBullet() YET!
			playerShotThisFrame = true;
		}

		// NO MORE RE-FETCHING HERE! We are still using the safe, original references.

		if (state.shootTimer > 0)
		{
			state.shootTimer -= dt.asSeconds();
		}

		bool isShooting = state.shootTimer > 0;

		if (!isGrounded)
		{
			newState = isShooting ? JumpShoot : Jump; // (Note: you should change one to JumpShoot later)
		}
		else if (std::abs(transform.vel.x) > 0.2f)
		{
			newState = isShooting ? RunShoot : Run;
		}
		else
		{
			newState = isShooting ? Shoot : Idle;
		}

		// THE "BRIDGE": SWAP ANIMATION ONLY ON CHANGE ---
		if (oldState != newState && anim.name != Spawn)
		{
			// Save the frame we are currently on so the legs don't reset!
			size_t prevFrame = anim.currentFrame;
			float prevTimer = anim.timer;

			state.state = newState;
			m_entities.addComponent<CAnimation>(id, m_assets.getAnimation(state.state));

			// Create boolean checks for our paired states
			bool isRunSync = (oldState == Run && newState == RunShoot) ||
				(oldState == RunShoot && newState == Run);

			bool isJumpSync = (oldState == Jump && newState == JumpShoot) ||
				(oldState == JumpShoot && newState == Jump);

			// Sync frames if we are swapping between Run and RunShoot
			if (isRunSync || isJumpSync)
			{
				auto& newAnim = m_entities.getComponent<CAnimation>(id);
				if (!newAnim.frames.empty())
				{
					newAnim.currentFrame = prevFrame	% newAnim.frames.size();
					newAnim.timer = prevTimer; // Keep the sub-frame timing perfect!
				}
			}
		}
	}

	// THE SAFE ZONE (DEFERRED ACTIONS) ---
	// The player loop is over. The references are gone. We can safely resize vectors!
	if (playerShotThisFrame)
	{
		spawnBullet();
	}

	// BULLET MOVEMENT ---
	for (auto id : m_entities.getEntities("bullet"))
	{
		auto& transform = m_entities.getComponent<CTransform>(id);
		transform.pos += transform.vel * dt.asSeconds();

		if (transform.pos.x < -100 || transform.pos.x > 1380)
		{
			m_entities.removeEntity(id);
		}
	}
}

void Game::sCollision()
{
	if (!m_entities.hasComponent<CBoundingBox>(m_playerID)) return;

	auto& pTransform = m_entities.getComponent<CTransform>(m_playerID);
	auto& pBox = m_entities.getComponent<CBoundingBox>(m_playerID);
	auto& pState = m_entities.getComponent<CState>(m_playerID);

	// 1. Save the intended diagonal movement calculated by sMovement
	sf::Vector2f destination = pTransform.pos;

	// 2. Teleport Mega Man back to his safe position from the start of the frame
	pTransform.pos = pTransform.prevPos;

	// ==========================================
	// X-AXIS COLLISION ONLY
	// ==========================================
	pTransform.pos.x = destination.x; // Apply only horizontal movement

	for (auto tile : m_entities.getEntities("tile"))
	{
		if (!m_entities.hasComponent<CBoundingBox>(tile)) continue;
		auto& tTransform = m_entities.getComponent<CTransform>(tile);
		auto& tBox = m_entities.getComponent<CBoundingBox>(tile);

		// Calculate player edges (using the current pos.x and prevPos.y)
		float pLeft = pTransform.pos.x - pBox.halfSize.x;
		float pRight = pTransform.pos.x + pBox.halfSize.x;
		float pBottom = pTransform.pos.y;
		float pTop = pTransform.pos.y - pBox.size.y;

		float tLeft = tTransform.pos.x;
		float tRight = tTransform.pos.x + tBox.size.x;
		float tTop = tTransform.pos.y;
		float tBottom = tTransform.pos.y + tBox.size.y;

		if (pRight > tLeft && pLeft < tRight && pBottom > tTop && pTop < tBottom)
		{
			// We ONLY moved X, so this MUST be a wall hit! No guessing needed!
			if (pTransform.vel.x > 0) // Pushing right
			{
				pTransform.pos.x = tLeft - pBox.halfSize.x;
			}
			else if (pTransform.vel.x < 0) // Pushing left
			{
				pTransform.pos.x = tRight + pBox.halfSize.x;
			}
		}
	}

	// ==========================================
	// Y-AXIS COLLISION ONLY
	// ==========================================
	pTransform.pos.y = destination.y; // Now apply vertical movement

	for (auto tile : m_entities.getEntities("tile"))
	{
		if (!m_entities.hasComponent<CBoundingBox>(tile)) continue;
		auto& tTransform = m_entities.getComponent<CTransform>(tile);
		auto& tBox = m_entities.getComponent<CBoundingBox>(tile);

		// RECALCULATE player edges because Phase 1 might have pushed us on the X axis!
		float pLeft = pTransform.pos.x - pBox.halfSize.x;
		float pRight = pTransform.pos.x + pBox.halfSize.x;
		float pBottom = pTransform.pos.y;
		float pTop = pTransform.pos.y - pBox.size.y;

		float tLeft = tTransform.pos.x;
		float tRight = tTransform.pos.x + tBox.size.x;
		float tTop = tTransform.pos.y;
		float tBottom = tTransform.pos.y + tBox.size.y;

		if (pRight > tLeft && pLeft < tRight && pBottom > tTop && pTop < tBottom)
		{
			// We just moved Y, and X is already safe. This MUST be a floor or ceiling!
			if (pTransform.vel.y > 0) // Falling down
			{
				pTransform.pos.y = tTop;
				pTransform.vel.y = 0;
				pState.isGrounded = true;
			}
			else if (pTransform.vel.y < 0) // Jumping up
			{
				pTransform.pos.y = tBottom + pBox.size.y;
				pTransform.vel.y = 0;
			}
		}
	}

}

void Game::sAnimation(sf::Time dt)
{
	for (const auto& entityDesc : m_entities.getAllEntities())
	{
		// 2. Skip entities that have been destroyed
		if (!entityDesc.active) continue;

		EntityID id = entityDesc.id;

		// 3. Use your mask-checking template!
		if (m_entities.hasComponent<CAnimation>(id) && m_entities.hasComponent<CSprite>(id)
			&& m_entities.hasComponent<CState>(id))
		{
			// 4. Get the actual data components
			auto& anim = m_entities.getComponent<CAnimation>(id);
			auto& sprite = m_entities.getComponent<CSprite>(id).sprite.value(); // Assuming CSprite has a .sprite member
			auto& state = m_entities.getComponent<CState>(id);

			//If the animation has no frames, skip it so we don't crash!
			if (anim.frames.empty()) continue;

			// 5. Update Timer
			anim.timer += dt.asSeconds();

			if (anim.timer >= anim.speed)
			{
				anim.timer = 0; // Reset timer

				// Advance frame logic
				if (anim.loop) 
				{
					anim.currentFrame = (anim.currentFrame + 1) % anim.frames.size();
				}
				else
				{
					if (anim.currentFrame < anim.frames.size() - 1)
					{
						anim.currentFrame++;
					}
					else if (state.state == Spawn)
					{
						state.state = Idle;
						anim = m_assets.getAnimation(Idle);
						//m_entities.addComponent<CAnimation>(id, anim);
					}
				}

			}
			// Apply new frame to the sprite
			auto rect = anim.getRect();
			sprite.setTextureRect(rect);

			// Re-center origin so the beam doesn't shake!
			sprite.setOrigin({ rect.size.x / 2.0f, (float)rect.size.y });
		}
	}
}


void Game::spawnPlayer(float x,float y)
{
	// Create the entity and get its ID
	m_playerID = m_entities.addEntity("player");

	// Add Transform (Put him in the middle of the screen)
	CTransform transform;
	transform.pos = { x,y };
	transform.scale = { 1.5f,1.5f };
	transform.speed = 200.f;
	transform.jumpSpeed = 600.f;
	m_entities.addComponent<CTransform>(m_playerID, transform);

	// Add Sprite Component
	CSprite cSprite;
	cSprite.sprite = sf::Sprite(m_assets.getTexture("MegaManX"));
	

	//Add State Component ("Spawn")
	CState cState;
	cState.state = Spawn;
	m_entities.addComponent<CState>(m_playerID, cState);

	// Add Animation Component ("Spawn")
	CAnimation cAnim = m_assets.getAnimation(m_entities.getComponent<CState>(m_playerID).state);
	m_entities.addComponent<CAnimation>(m_playerID, cAnim);


	auto& anim = m_entities.getComponent<CAnimation>(m_playerID);

	// Grab the very first frame of the Spawn animation (index 0)
	sf::IntRect firstFrame = anim.frames[0];

	// Cut out the first frame immediately so the whole sheet doesn't draw
	cSprite.sprite.value().setTextureRect(firstFrame);
	cSprite.sprite.value().setOrigin({ firstFrame.size.x / 2.0f, (float)firstFrame.size.y });
	m_entities.addComponent<CSprite>(m_playerID, cSprite);

	m_entities.addComponent<CBoundingBox>(m_playerID, sf::Vector2f(35.f*transform.scale.x,46.f*transform.scale.y));
}

void Game::spawnBullet()
{
	EntityID bulletID = m_entities.addEntity("bullet");
	auto& playerTransform = m_entities.getComponent<CTransform>(m_playerID);

	CTransform bulletTransform;
	bulletTransform.pos = { playerTransform.pos.x, playerTransform.pos.y - 45.0f };
	bulletTransform.scale.x = playerTransform.scale.x;
	bulletTransform.scale.y = playerTransform.scale.y;


	float xOffset = (playerTransform.scale.x > 0) ? 35.0f : -35.0f;
	bulletTransform.pos.x += xOffset;


	if (playerTransform.scale.x < 0) bulletTransform.vel.x = -800.0f;
	else bulletTransform.vel.x = 800.0f;

	bulletTransform.gravity = 0.0f;
	m_entities.addComponent<CTransform>(bulletID, bulletTransform);

	CState cState;
	cState.state = BusterShot;
	m_entities.addComponent<CState>(bulletID, cState);

	CAnimation Banim = m_assets.getAnimation(BusterShot);
	if (Banim.frames.empty()) {
		std::cerr << "Error: BusterShot animation is empty!" << std::endl;
		return;
	}
	m_entities.addComponent<CAnimation>(bulletID, Banim);


	CAnimation& anim = m_entities.getComponent<CAnimation>(bulletID);

	CSprite cSprite;
	cSprite.sprite = sf::Sprite(m_assets.getTexture("MegaManX"));
	cSprite.sprite.value().setTextureRect(anim.frames[0]);
	cSprite.sprite.value().setColor(sf::Color(218, 165, 32));

	m_entities.addComponent<CSprite>(bulletID, cSprite);


}

void Game::loadLevel(const std::string& levelPath)
{
	std::ifstream file(levelPath);
	if (!file.is_open()) {
		std::cerr << "Failed to load level file!" << std::endl;
		return;
	}

	std::string line;
	float y = 0.0f;          // Tracks our Row
	float gridSize = 32.0f;  // Size of one tile

	// Read the file line by line
	while (std::getline(file, line))
	{
		float x = 0.0f;      // Tracks our Column

		for (char tileType : line)
		{
			if (tileType == 'P')
			{
				// Spawn Mega Man exactly here!
				spawnPlayer(x, y);
			}
			else if (tileType == 'R')
			{
				// Spawn a SOLID road block
				auto entity = m_entities.addEntity("tile");
				CTransform transform;
				transform.pos = { x,y };
				transform.scale = { .5f,.5f };
				m_entities.addComponent<CTransform>(entity, transform);


				CSprite cSprite(m_assets.getTexture("Floor"),m_assets.getTileRect("R"));
				m_entities.addComponent<CSprite>(entity,cSprite);

				// Give it a bounding box so Mega Man can stand on it
				m_entities.addComponent<CBoundingBox>(entity, sf::Vector2f(gridSize, gridSize));
			}
			x += gridSize; // Move to the next column
		}
		y += gridSize; // Move to the next row
	}
}
