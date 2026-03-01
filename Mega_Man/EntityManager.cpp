#include "EntityManager.h"
#include <algorithm> // Required for std::remove_if

EntityManager::EntityManager()
	:m_totalEntities(0)
{
}

EntityID EntityManager::addEntity(const std::string& tag)
{
	EntityID id = m_totalEntities++;

	m_entities.resize(m_totalEntities);
	m_transforms.resize(m_totalEntities);
	m_sprites.resize(m_totalEntities);
	m_inputs.resize(m_totalEntities);
	m_boundingboxes.resize(m_totalEntities);
	m_animations.resize(m_totalEntities);
	m_states.resize(m_totalEntities);

	m_entities[id].id = id;
	m_entities[id].tag = tag;
	m_entities[id].active = true;
	m_entities[id].mask.reset();

	//Adding index to the search map
	m_entityMap[tag].push_back(id);

	return id;
}

const std::vector<EntityID>& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}

const std::vector<EntityManager::EntityDesc>& EntityManager::getAllEntities() const
{
	return m_entities;
}

void EntityManager::update()
{
	for (auto &pair : m_entityMap)
	{
		removeDeadEntities(pair.second);
	}
}

void EntityManager::removeEntity(EntityID id)
{
	m_entities[id].active = false;
}

void EntityManager::removeDeadEntities(std::vector<EntityID>& vec)
{
	auto end = std::remove_if(vec.begin(), vec.end(), [this](EntityID id) {
		return !m_entities[id].active;
		});

	vec.erase(end, vec.end());
}