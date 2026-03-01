#pragma once
#include"components.h"
#include<vector>
#include<map>
#include<bitset>

typedef size_t EntityID;
typedef std::bitset<6> ComponentMask;

class EntityManager
{
    // 1. Storage for all entities (The "Rows")
    struct EntityDesc {
        EntityID id = 0;
        std::string tag = "Default";
        bool active = true;
        ComponentMask mask;
    };

    std::vector<EntityDesc> m_entities;
    std::vector<CTransform> m_transforms;
    std::vector<CSprite> m_sprites;
    std::vector<CInput> m_inputs;
    std::vector<CBoundingBox> m_boundingboxes;
    std::vector<CAnimation> m_animations;
    std::vector<CState> m_states;

    // 3. Helper to find entities by Tag (e.g., "Give me all Bullets")
    std::map<std::string, std::vector<EntityID>> m_entityMap;
    size_t m_totalEntities = 0;

    // Helper to remove dead IDs from a vector
    void removeDeadEntities(std::vector<EntityID>& vec);

public:

    EntityManager();

    // The main function you will use:
    EntityID addEntity(const std::string& tag);

    // Getting the Entities
    const std::vector<EntityID>& getEntities(const std::string& tag);
    const std::vector<EntityDesc>& getAllEntities() const;

    void update();
    void removeEntity(EntityID id);


    // Component Management 
    // 1. Add Component: "Here is data for this ID"
    template <typename T>
    void addComponent(EntityID id, T component)
    {
        // Get the specific vector and set the data
        getComponent<T>(id) = component;

        // Turn on the bit in the mask so we know it exists
        if constexpr (std::is_same_v<T, CTransform>)
            m_entities[id].mask.set(0); // Bit 0 for Transform
        else if constexpr (std::is_same_v<T, CSprite>)
            m_entities[id].mask.set(1); // Bit 1 for Sprite
        else if constexpr (std::is_same_v<T, CInput>)
            m_entities[id].mask.set(2); // Bit 2 for Input
        else if constexpr (std::is_same_v<T, CBoundingBox>)
            m_entities[id].mask.set(3); // Bit 3 for Box
        else if constexpr (std::is_same_v<T, CAnimation>)
            m_entities[id].mask.set(4); // Bit 4 for Animation
        else if constexpr (std::is_same_v<T, CState>)
            m_entities[id].mask.set(5); // Bit 5 for State
    }

    // 2. Get Component: "Give me the data for this ID"
    template <typename T>
    T& getComponent(EntityID id)
    {
        if constexpr (std::is_same_v<T, CTransform>) return m_transforms[id];
        else if constexpr (std::is_same_v<T, CSprite>) return m_sprites[id];
        else if constexpr (std::is_same_v<T, CInput>) return m_inputs[id];
        else if constexpr (std::is_same_v<T, CBoundingBox>) return m_boundingboxes[id];
        else if constexpr (std::is_same_v<T, CAnimation>) return m_animations[id];
        else if constexpr (std::is_same_v<T, CState>) return m_states[id];
    }

    // 3. Has Component: "Does this ID have this data?"
    template <typename T>
    bool hasComponent(EntityID id)
    {
        if constexpr (std::is_same_v<T, CTransform>) return m_entities[id].mask.test(0);
        else if constexpr (std::is_same_v<T, CSprite>) return m_entities[id].mask.test(1);
        else if constexpr (std::is_same_v<T, CInput>) return m_entities[id].mask.test(2);
        else if constexpr (std::is_same_v<T, CBoundingBox>) return m_entities[id].mask.test(3);
        else if constexpr (std::is_same_v<T, CAnimation>) return m_entities[id].mask.test(4);
        else if constexpr (std::is_same_v<T, CState>) return m_entities[id].mask.test(5);

        return false;
    }
    
};