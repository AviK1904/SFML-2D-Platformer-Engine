#include"assets.h"
#include<iostream>
#include<fstream>

State stringToState(const std::string& str)
{
    if (str == "Spawn") return Spawn;
    if (str == "Idle")  return Idle;
    if (str == "Run")   return Run;
    if (str == "Jump")  return Jump;
    if (str == "BusterShot")return BusterShot;
    if (str == "Shoot")return Shoot;
    if (str == "RunShoot")return RunShoot;
    if (str == "JumpShoot")return JumpShoot;

    return Count; // Default fallback if you misspell something in assets.txt!
}

Assets::Assets()
{

}

void Assets::addTexture(const std::string& name, const std::string& path)
{
	sf::Texture tex;
	if (!tex.loadFromFile(path))
	{
		std::cerr << "Textures not Loaded" << std::endl;
		return;
	}
    tex.setSmooth(true);

	m_textures[name] = std::move(tex);
}

void Assets::addAnimation(const State name, const CAnimation &anim)
{
	m_animations[name] = anim;
}

void Assets::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    std::string tag;

    while (file >> tag) {
        if (tag == "Texture") {
            std::string name, path;
            file >> name >> path;
            addTexture(name, path);
        }
        else if (tag == "Animation") {
            std::string name;
            float speed;
            int frameCount;
            bool loop;
            file >> name >> speed >> frameCount >> loop;
            State state = stringToState(name);

            if (state == Count)
            {
                std::cerr << "State Not Defined error in asset.cpp" << std::endl;
            }

            CAnimation anim(state, speed);
            anim.loop = loop; // Store it in the component
            anim.frameCount = frameCount;

            // Loop for the specific number of frames defined
            for (int i = 0; i < frameCount; i++) {
                int x, y, w, h;
                file >> x >> y >> w >> h;
                anim.frames.push_back(sf::IntRect({ x, y }, { w, h }));
            }

            addAnimation(state, anim);
        }
        else if (tag == "Tile")
        {
            std::string name; 
            int x, y, w, h;

            file >> name >> x >> y >> w >> h;

            // Store the cutout rectangle in your map!
            m_tileRects[name] = sf::IntRect({ x, y }, { w, h });
        }
    }

}

const sf::Texture& Assets::getTexture(const std::string& name) const
{
	return m_textures.at(name);
}

const CAnimation& Assets::getAnimation(const State name) const
{
	return m_animations.at(name);
}

const sf::IntRect& Assets::getTileRect(const std::string& name) const
{
    return m_tileRects.at(name);
}






