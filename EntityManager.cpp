#include <iostream>
#include <vector>

#include "EntityManager.h"

EntityManager::EntityManager()
	: m_totalEntities(0)
{
}

void EntityManager::update()
{
	for (auto e : m_entitiesToAdd)
	{
		m_entities.push_back(e);

		m_entityMap[e->tag()].push_back(e);
	}

	// remove dead entities from the vector of all entities
	removeDeadEntities(m_entities);

	// remove dead entities from each vector in the entity map
	// use C++20 of interating through [key, value] pairs in a map
	for (auto& [tag, entityVec] : m_entityMap)
	{
		removeDeadEntities(entityVec);
	}
	m_entitiesToAdd.clear();
}

void EntityManager::removeDeadEntities(EntityVec& vec)
{
	std::erase_if(vec, [](auto& entity) { return !entity->isActive(); });
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

	m_entitiesToAdd.push_back(entity);

	return entity;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}
