#pragma once
#include<SFML/Graphics.hpp>
#include"components.h"

class Assets
{
	std::unordered_map<std::string, sf::Texture> m_textures;
	std::unordered_map<State, CAnimation> m_animations;

	//For Tiles
	std::unordered_map<std::string, sf::IntRect> m_tileRects;

public:
	Assets();

	void addTexture(const std::string& name, const std::string& path);
	void addAnimation(const State name, const CAnimation &anim);

	void loadFromFile(const std::string& path);

	const sf::Texture& getTexture(const std::string& name) const;
	const CAnimation& getAnimation(const State name) const;

	const sf::IntRect& getTileRect(const std::string& name) const;

};