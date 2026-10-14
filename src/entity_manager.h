#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <cstddef>  // size_t

#include "pack.h"  // Pack from prep
#include "registry.h"
#include <utility>
#include <vector>
#include <limits>
#include <type_traits>
#include <tuple>
#include <array>
#include <cassert>

using EntityID = size_t;

template <typename... ComponentTypes>
class EntityManager
{
    public:
        EntityID createEntity()
        {
            //used this link as reference: https://en.cppreference.com/w/cpp/container/array/fill
            std::array<size_t, num_components> original_slots;
            original_slots.fill(empty_slot);
            EntityID id;

            if (!free_entities.empty())
            {
                id = free_entities.back();
                free_entities.pop_back();
                entity_is_live[id] = true;
                entity_to_tickets[id] = original_slots;
            }
            else
            {
                id = entity_is_live.size();
                entity_is_live.push_back(true);
                entity_to_tickets.push_back(original_slots);
            }
            return id;
        }

        bool validEntity(EntityID id) const
        {
            if (id < entity_is_live.size() && entity_is_live[id])
            {
                return true;
            }
            else
            {
                return false;
            }
        }


        template <typename ComponentType>
        bool hasComponent(EntityID id) const
        {
            //used link as reference: https://en.cppreference.com/w/cpp/language/dependent_name
            static_assert(Pack<ComponentTypes...>::template contains<ComponentType>, "ComponentType not valid");
            assert(validEntity(id) && "EntityID is not valid");
            static constexpr auto component_type_index = Pack<ComponentTypes...>::template index<ComponentType>;
            return entity_to_tickets[id][component_type_index]!=empty_slot;
        }

        template <typename ComponentType>
        ComponentType& getComponent(EntityID id)
        {
            static_assert(Pack<ComponentTypes...>::template contains<ComponentType>, "ComponentType not valid");
            assert(validEntity(id) && "EntityID is not valid");
            assert(hasComponent<ComponentType>(id) && "The entity doesn't contain this component!");

            static constexpr auto COMPONENT_TYPE_INDEX =Pack<ComponentTypes...>::template index<ComponentType>;
            size_t ticket_id = entity_to_tickets[id][COMPONENT_TYPE_INDEX];

            //used as reference: https://en.cppreference.com/w/cpp/utility/tuple/get
            return std::get<COMPONENT_TYPE_INDEX>(component_storage)[ticket_id];
        }

        template <typename ComponentType>
        const ComponentType& getComponent(EntityID id) const
        {
            static_assert(Pack<ComponentTypes...>::template contains<ComponentType>, "ComponentType not valid");
            assert(validEntity(id) && "EntityID is not valid");
            assert(hasComponent<ComponentType>(id) && "The entity doesn't contain this component!");
            static constexpr auto COMPONENT_TYPE_INDEX = Pack<ComponentTypes...>::template index<ComponentType>;
            size_t ticket_id = entity_to_tickets[id][COMPONENT_TYPE_INDEX];
            return std::get<COMPONENT_TYPE_INDEX>(component_storage)[ticket_id];
        }

        template <typename ComponentType>
        void addComponent(EntityID id, ComponentType&& component)
        {
            using BaseComponent = std::remove_reference_t<ComponentType>;
            static_assert(Pack<ComponentTypes...>::template contains<BaseComponent>,"ComponentType not valid");
            
            assert(validEntity(id) && "EntityID is not valid");
            assert(!hasComponent<BaseComponent>(id) && "Entity contains component already");

            static constexpr auto COMPONENT_TYPE_INDEX =
                Pack<ComponentTypes...>::template index<BaseComponent>;
            size_t ticket_id = std::get<COMPONENT_TYPE_INDEX>(component_storage).store(std::forward<ComponentType>(component));

            entity_to_tickets[id][COMPONENT_TYPE_INDEX] = ticket_id;
        }

    private:
        static constexpr size_t num_components = Pack<ComponentTypes...>::size;
        static constexpr size_t empty_slot = std::numeric_limits<size_t>::max();

        std::vector<std::array<size_t, num_components>> entity_to_tickets;
        std::tuple<Registry<ComponentTypes>...> component_storage;

        std::vector<bool> entity_is_live;
        std::vector<EntityID> free_entities;

};

#endif  // ENTITY_MANAGER_H
