#pragma once

#include <initializer_list.hpp>
#include <capi/early_heap.hpp>

namespace capi
{
    /*
        A linked list implementation using early heap
        A linked list was chosen because it allows inserting an element in the middle without
        copying large memory blocks, which is something done often in an ordered list
    */
    template <typename t>
    class early_ordered_list
    {
    public:
        using type      = t     ;
        using index     = size_t;
        using size_type = size_t;
    public:
        struct list_item
        {
            type       item;
            list_item* next;
        };
    private:
        constexpr static size_type ALLOC_SIZE = utils::type_meta<list_item>::size();
    public:
        early_ordered_list()
            : m_heap(nullptr),
                m_first(nullptr),
                m_size(0)
        { }
        early_ordered_list(std::initializer_list<type> list, capi::std_early_heap* heap) 
            : m_heap(heap),
                m_first(nullptr),
                m_size(0)
        {
            for(type const& i : list)
                add_item(i);
        }
        early_ordered_list(capi::std_early_heap* heap)
            : m_heap(heap),
                m_first(nullptr),
                m_size(0)
            { }
        ~early_ordered_list()
        {
            if(m_heap == nullptr)
                return;
            for(list_item* current = m_first; current != nullptr; current = current->next)
                m_heap->free(current, ALLOC_SIZE);
        }
    public:
        void init_heap(capi::std_early_heap* heap)
        {
            m_heap = heap;
        }
    public:
        // TODO: Display proper error when m_heap is uninitialized
        void add_item(type const& item)
        {
            if(m_heap == nullptr)
                return;
            list_item* current  = m_first;
            list_item* previous = nullptr;
            for(; 
                current != nullptr && current->item < item;
                previous = current, current = current->next
            );
            list_item*& previous_next = previous == nullptr ? m_first : previous->next;
            list_item* new_struct = reinterpret_cast<list_item*>(m_heap->alloc(ALLOC_SIZE));
            new_struct->item = item;
            new_struct->next = previous_next;
            previous_next = new_struct;
            ++m_size;
        }
        void remove_item(type const& item)
        {
            if(m_heap == nullptr)
                return;
            list_item* current  = m_first;
            list_item* previous = nullptr;   
            for(; 
                current != nullptr && current->item != item;
                previous = current, current = current->next
            );
            if(current == nullptr)
                return;
            list_item*& previous_next = previous == nullptr ? m_first : previous->next;
            previous_next = current->next;
            current->item.~type();
            m_heap->free(current, ALLOC_SIZE);
            --m_size;
        }
        void remove_from_head(list_item* to_remove, list_item* parent)
        {
            list_item*& parent_next = parent == nullptr ? m_first : parent->next;
            parent_next = to_remove->next;
            to_remove->item.~type();
            m_heap->free(to_remove, ALLOC_SIZE);
            --m_size;
        }
        type* top()
        {
            if(m_first == nullptr)
                return nullptr;
            list_item* current = m_first;
            for(
                ;
                current != nullptr && current->next != nullptr;
                current = current->next
            )
            {
            }
            return &(current->item);
        }
        type const* top() const
        {
            if(m_first == nullptr)
                return nullptr;
            list_item* current = m_first;
            for(
                ;
                current != nullptr && current->next != nullptr;
                current = current->next
            )
            {
            }
            return &(current->item);
        }
        constexpr list_item* head() const
        {
            return m_first;
        }
        constexpr size_type size() const
        {
            return m_size;
        }
    private:
        capi::std_early_heap* m_heap ;
        list_item*           m_first;
        size_type            m_size ;
    };
}
