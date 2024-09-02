#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

/*! \file conflicts.h
*	\brief Implements the template class Conflicts.
*   \author Christophe COUAILLET
*/

#include <requirements.hpp>

/*! \brief Class conflicts implements a specialized container that lists the bidirectional conflict relationships between objects.
* 
    Create a relationship with an object itself is not allowed.
    Doubles are not allowed, each relationship is unique.
    The class Conflicts supports two distinct modes at instantiation:
    \li without cascading: only direct relationships between objects are considered.
    \li with cascading: conflicts between objects are evaluated by recursing relationships (if an object A is in conflict with an object B that is in conflict with an object C, then A is in conflict with C).

    The cascading mode cannot be changed after instantiation.
*/
template <typename T>
class Conflicts
{
    public:
        /*! \brief Default constructor with no cascading support. */
        Conflicts()
            : m_cascading(false) {};
        /*! \brief Constructor with cascading mode selection. */
        Conflicts(const bool cascading)
            : m_cascading(cascading) {};

        /*! \brief Returns the cascading mode. */
        bool cascading() { return m_cascading; }
        /*! \brief Clears all relationships. */
        void clear() noexcept { m_conflicts.clear(); }

        /*! \brief Returns true if no relationship has been set. */
        bool empty() const noexcept { return m_conflicts.empty(); }
        /*! \brief Returns the number of existing relationships. */
        size_t size() const noexcept { return m_conflicts.size(); }

        void add(const T &object1, const T &object2);
        void remove(const T &object1, const T &object2);
        void remove(const T &object);
        bool in_conflict(const T &object) const noexcept;
        bool in_conflict(const T &object1, const T &object2) const noexcept;
        std::vector<T> conflicts(const T &object) const;                            // lists direct conflicts
        std::vector<T> all_conflicts(const T &object) const;                        // lists all implicit conflicts if cascading is on
        std::unordered_multimap<T, T> get() const;
        void set(const std::unordered_multimap<T, T> &conflicts);
        void merge(const std::unordered_multimap<T, T> &conflicts);

    private:
        Requirements<T> m_conflicts{ false };
        bool m_cascading{ false };
        // if cascading is on, an object in conflict with another object is in conflict with all objects in relation with this object

        bool deep_search(const T& object1, const T& object2, const T* prev = NULL) const noexcept;
        std::vector<T> all_conflicts(const T& object, const T* prev) const;
};

// Implementation of templates functions

/*! \brief Adds a conflict relationship between two objects.
* 
*   An assertion occurs if the objects are same or if a conflict has already been set for these objects.
*   In cascading mode, this existence is evaluated recursively.
*/
template <typename T>
void Conflicts<T>::add(const T &object1, const T &object2)
{
    assert(!(object1 == object2) && "An object can't be in conflict with itself.");
    // we must ensure the conflict does not already exists, directly or, if cascading is on, indirectly
    assert(!in_conflict(object1, object2) && "Conflict already exists.");
    m_conflicts.add(object1, object2);
}

/*! \brief Removes a direct relationship between two objects.
* 
*   An assertion occurs if this conflict does not exist.
*/
template <typename T>
void Conflicts<T>::remove(const T &object1, const T &object2)
{
    // conflict definition is searched for the 2 directions (object, conflict) and (conflict, object)
    bool found {false};
    if (m_conflicts.exists(object1, object2))
    {
        m_conflicts.remove(object1, object2);
        found = true;
    }
    if (m_conflicts.exists(object2, object1))
    {
        m_conflicts.remove(object2, object1);
        found = true;
    }
    assert(found && "Conflict does not exist.");
}

/*! \brief Removes all existing conflicts involving the object.

    An assertion occurs if no conflict exists for this object.
*/
template <typename T>
void Conflicts<T>::remove(const T &object)
{
    assert(in_conflict(object) && "Conflict does not exist.");
    m_conflicts.remove_all(object);
}

/*! \brief Returns true if a conflict has been set involving this object.
*/
template <typename T>
bool Conflicts<T>::in_conflict(const T &object) const noexcept
{
    return  m_conflicts.has_requirements(object) || m_conflicts.has_dependents(object);
}

template <typename T>
bool Conflicts<T>::deep_search(const T &object1, const T &object2, const T* prev) const noexcept
{
    bool result = m_conflicts.exists(object1, object2) || m_conflicts.exists(object2, object1);
    if (!result && m_cascading)
    {
        // Deep search : if 2 objects are in conflict then a third one that is in conflict with one of them is in conflict with the other
        auto requirements = m_conflicts.requirements(object1);
        auto req = requirements.begin();
        while (!result && req != requirements.end())
        {
            if (prev == nullptr || !(*req == *prev || *req == object2))
                result = deep_search(*req, object2, &object1);
            ++req;
        }
        if (!result)
        {
            auto dependents = m_conflicts.dependents(object1);
            auto dep = dependents.begin();
            while (!result && dep != dependents.end())
            {
                if (prev == nullptr || !(*dep == *prev || *dep == object2))
                    result = deep_search(*dep, object2, &object1);
                ++dep;
            }
        }
    }
    return result;
}

/*! \brief Returns true if a conflict has been set between the two objects.
* 
*   In cascading mode, this evaluation is performed recursively.
*/
template <typename T>
bool Conflicts<T>::in_conflict(const T &object1, const T &object2) const noexcept
{
    bool result = deep_search(object1, object2);
    if (!result)
        result = deep_search(object2, object1);
    return result;
}

/*! \brief Returns the list of the objects in direct conflict with the given object.
* 
*   \sa Conflicts< T >::all_conflicts()
*/
template <typename T>
std::vector<T> Conflicts<T>::conflicts(const T &object) const
{
    std::vector<T> result {};
    auto confs = m_conflicts.requirements(object);
    for (auto con : confs)
        result.push_back(con);
    confs = m_conflicts.dependents(object);
    for (auto con : confs)
        result.push_back(con);
    return result;
}

template <typename T>
std::vector<T> Conflicts<T>::all_conflicts(const T &object, const T* prev) const
{
    std::vector<T> result {};
    auto confs = m_conflicts.requirements(object);
    for (auto con : confs)
    {
        if (prev == nullptr || !(con == *prev))
        {
            result.push_back(con);
            if (m_cascading)
            {
                // perform a recursive search if cascading is on
                auto res = all_conflicts(con, &object);
                for (auto itr : res)
                    result.push_back(itr);
            }
        }
    }
    confs = m_conflicts.dependents(object);
    for (auto con : confs)
    {
        if (prev == nullptr || !(con == *prev))
        {
            result.push_back(con);
            if (m_cascading)
            {
                // perform a recursive search if cascading is on
                auto res = all_conflicts(con, &object);
                for (auto itr : res)
                    result.push_back(itr);
            }
        }
    }
    return result;
}

/*! \brief Returns the list of the objects in conflict with the given object.
* 
*   In cascading mode, the objects are searched recursively.
*   \sa Conflicts< T >::conflicts()
*/
template <typename T>
std::vector<T> Conflicts<T>::all_conflicts(const T &object) const
{
    return all_conflicts(object, nullptr);
}

/*! \brief Returns the list of conflict pairs.
*/
template <typename T>
std::unordered_multimap<T, T> Conflicts<T>::get() const
{
    return m_conflicts.get();
}

/*! \brief Creates the conflicts from the given list. Existing conflicts are cleared first.
* 
*   An assertion occurs if rules are broken.
*   \sa Conflicts< T >::add()
*   \sa Conflicts< T >::merge()
*/
template <typename T>
void Conflicts<T>::set(const std::unordered_multimap<T, T> &conflicts)
{
    clear();
    merge(conflicts);
}

/*! \brief Adds conflicts from the given list.
*
*   An assertion occurs if rules are broken.
*   \sa Conflicts< T >::add()
*   \sa Conflicts< T >::set()
*/
template <typename T>
void Conflicts<T>::merge(const std::unordered_multimap<T, T> &conflicts)
{
    auto itr = conflicts.begin();
    while (itr != conflicts.end())
    {
        add((*itr).first, (*itr).second);
        ++itr;
    }
}
