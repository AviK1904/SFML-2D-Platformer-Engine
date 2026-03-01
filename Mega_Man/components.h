#pragma once
#include<SFML/Graphics.hpp>
#include<optional>


enum State
{
    Spawn,
    Idle,
    Run,
    Jump,
    BusterShot,
    Shoot,
    RunShoot,
    JumpShoot,




    Count
};


class CTransform
{

public:
	sf::Vector2f pos = { 0.0f,0.0f };
    sf::Vector2f prevPos = { 0.0, 0.0 };
	sf::Vector2f vel = { 0.0f,0.0f };
    sf::Vector2f scale = { 1.0f, 1.0f };
    float angle = 0.f;

    float speed = 0.0f;
    float jumpSpeed = 0.0f;


    float gravity = 1200.0f;

    CTransform(){}
	CTransform(const sf::Vector2f p, const sf::Vector2f v, const float a,const float s,const float js,const float g)
		: pos(p),vel(v),angle(a),speed(s),jumpSpeed(js),gravity(g)
	{	}
};

class CSprite
{

public:
    std::optional<sf::Sprite> sprite;

    CSprite(){}

    CSprite(const sf::Texture& texture)
    {
        sprite.emplace(texture);
    }

    CSprite(const sf::Texture& texture, const sf::IntRect& rect)
    {
        sprite.emplace(texture);
        sprite->setTextureRect(rect);
    }

};

class CInput
{

public:
    bool up = 0;
    bool down = 0;
    bool left = 0;
    bool right = 0;
    bool shoot = 0;
};

class CBoundingBox
{

public:
    sf::Vector2f size;
    sf::Vector2f halfSize;

    CBoundingBox(){}
    CBoundingBox(sf::Vector2f s)
        :size(s),halfSize(s.x/2.f,s.y/2.f)
    { }
};


class CAnimation {
public:
    State name=Idle;       // "MegaMan_Run"
    std::vector<sf::IntRect> frames;
    size_t frameCount = 0;      // How many frames in total
    size_t currentFrame = 0;;   // Which one are we on?
    float speed = 0.0f;            // Time per frame (e.g., 0.1s)
    float timer = 0.0f;            // Accumulator: timer += dt
    bool loop = true;              // Does it repeat?

    CAnimation() : frameCount(1), currentFrame(0), speed(0), timer(0), loop(true) {}

    CAnimation(const State n, float s) : name(n), speed(s) {}

    // Helper to get the current rectangle for drawing
    const sf::IntRect& getRect() const
    {
        return frames[currentFrame];
    }
};

class CState
{
public:
    State state=Idle;
    float shootTimer = 0.0f;
    bool isGrounded = false;
};