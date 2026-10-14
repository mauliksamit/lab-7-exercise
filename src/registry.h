#ifndef REGISTRY_H
#define REGISTRY_H


#include <utility>
#include <vector>
#include <cassert>
#include <cstddef>

template <typename T>
class Registry
{
    public:

    size_t store(const T& item)
    {
        size_t ticket_id;
        if (!free_slots.empty())
        {
            ticket_id = free_slots.back();
            free_slots.pop_back();
            components[ticket_id] = item;
            is_live[ticket_id] = true;
        }
        else
        {
            ticket_id = components.size();
            components.push_back(item);
            is_live.push_back(true);
        }

        live_count++;
        return ticket_id;
    }


    size_t store(T&& item)
    {
        size_t ticket_id;
        if (!free_slots.empty())
        {
            ticket_id = free_slots.back();
            free_slots.pop_back();
            components[ticket_id] = std::move(item); //for rvalues
            is_live[ticket_id] = true;
        }
        else
        {
            ticket_id = components.size();
            components.push_back(std::move(item));
            is_live.push_back(true);
        }

        live_count++;
        return ticket_id;
    }

    void remove(size_t ticket_id)
    {
        assert(valid(ticket_id) && "Ticket ID is invalid");
        components[ticket_id] = T{};
        is_live[ticket_id] = false;
        free_slots.push_back(ticket_id);
        live_count--;
    }

    T& operator[](size_t ticket_id)
    {
        assert(valid(ticket_id) && "Ticket ID is invalid");
        return components[ticket_id];
    }

    const T& operator[](size_t ticket_id) const
    {
        assert(valid(ticket_id) && "Ticket ID is invalid");
        return components[ticket_id];
        
    }

    bool valid(size_t ticket_id) const
    {
        if (ticket_id < is_live.size() && is_live[ticket_id])
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    size_t size() const
    {
        return live_count;
    }

    private:
    std::vector<T> components;
    std::vector<size_t> free_slots;
    std::vector<bool> is_live;
    size_t live_count =0;
};

#endif  // REGISTRY_H
