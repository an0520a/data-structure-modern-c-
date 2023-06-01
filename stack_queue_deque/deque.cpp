#pragma once

#include <new>
#include "deque.hpp"

template <std::size_t N>
struct MsbPosStruct
{
    static constexpr std::size_t value = 1 + MsbPosStruct<(N >> 1)>::value;
};

template <>
struct MsbPosStruct<0>
{
    static constexpr std::size_t value = 0;
};

template <std::size_t N>
using MsbPos = struct MsbPosStruct<N>;

template <typename T>
constexpr std::size_t Deque<T>::determine_block_size()
{
    constexpr std::size_t required_minimum_memory = sizeof(Deque<T>::DequeBlock) + (alignof(T) - 1) + sizeof(T);

    if constexpr (required_minimum_memory <= kDequeBlockMemoryUnitSize_)
    {
        return kDequeBlockMemoryUnitSize_;
    }
    else
    {
        // Multiples of 512
        constexpr std::size_t calculated_size = 
        (required_minimum_memory + (kDequeBlockMemoryUnitSize_ - 1)) & ~(kDequeBlockMemoryUnitSize_ - 1);

        // Returns raised to a power of 2
        if constexpr (calculated_size == (1 << (MsbPos<calculated_size>::value - 1)))
        {
            return calculated_size;
        }
        else
        {
            return 1 << MsbPos<calculated_size>::value;
        }
    }
}

template <typename T>
constexpr typename Deque<T>::size_type Deque<T>::determine_block_element_capacity()
{
    return (kDequeBlockMemorySize_ - (sizeof(DequeBlock) + alignof(T) - 1)) / sizeof(T);
}

template <typename T>
Deque<T>::Deque() noexcept
: front_block_(nullptr), back_block_(nullptr), front_element_(nullptr), back_element_(nullptr), size_(0)
{
}

template <typename T>
Deque<T>::Deque(size_type __count, const value_type& __value)
: front_block_(nullptr), back_block_(nullptr), front_element_(nullptr), back_element_(nullptr), size_(0)
{
    if (__count == 0)
    {
        return;
    }

    front_block_ = make_new_block();
    back_block_ = front_block_;
    front_element_ = front_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
    back_element_ = front_element_ - 1;

    try
    {
        for (size_type i = 0; i < __count; i++)
        {
            back_element_++;

            if (back_element_ == back_block_->block_end_)
            {
                prepare_next_block_of_back();
                back_block_ = back_block_->next_block_;
                back_element_ = back_block_->block_begin_;
            }

            new (back_element_) value_type(__value);

            size_++;
        }
    }
    catch(const std::exception& error)
    {
        if (back_element_ == front_element_)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;
        }
        else
        {
            back_element_--;
        }

        delete_all_block();

        throw error;
    }
}

template <typename T>
template <typename InputIt>
Deque<T>::Deque(InputIt __begin, InputIt __end)
: front_block_(nullptr), back_block_(nullptr), front_element_(nullptr), back_element_(nullptr), size_(0)
{
    if (__begin == __end)
    {
        return;
    }

    try
    {
        front_block_ = make_new_block(); // can occured error. but it don't need excetion handling;
        back_block_ = front_block_;
        front_element_ = front_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        back_element_ = front_element_ - 1;

        while(__begin != __end)
        {
            back_element_++;
            
            if (back_element_ == back_block_->block_end_)
            {
                prepare_next_block_of_back(); // can occured error
                back_block_ = back_block_->next_block_;
                back_element_ = back_block_->block_begin_;
            }

            new (back_element_) value_type(*__begin); // can occured error

            __begin++;
            size_++;
        }
    }
    catch(const std::exception& error)
    {
        if (back_element_ == front_element_)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;
        }
        else
        {
            back_element_--;
        }

        delete_all_block();

        throw error;
    }
}

template <typename T>
Deque<T>::Deque(const Deque& __deque)
: front_block_(nullptr), back_block_(nullptr), front_element_(nullptr), back_element_(nullptr), size_(0)
{
    if (__deque.size_ == 0)
    {
        return;
    }

    DequeBlock* current_block = __deque.front_block_;
    T* current_element = __deque.front_element_ - 1;
    front_block_ = make_new_block();
    back_block_ = front_block_;
    front_element_ = front_block_->block_begin_ + (__deque.front_element_ - __deque.front_block_->block_begin_);
    back_element_ = front_element_ - 1;

    try
    {
        while (current_element != __deque.back_element_)
        {
            current_element++;
            back_element_++;

            if (current_element == current_block->block_end_)
            {
                current_block = current_block->next_block_;
                current_element = current_block->block_begin_;

                DequeBlock* new_block = make_new_block();
                link_block(back_block_, new_block);
                back_block_ = new_block;
                back_element_ = back_block_->block_begin_;
                new_block = nullptr;
            }

            new (back_element_) value_type(*current_element);
            size_++;
        }
    }
    catch (const std::exception& bad_alloc_error)
    {
        back_element_--;
        delete_all_block();
        throw bad_alloc_error;
    }
}

template <typename T>
Deque<T>::Deque(Deque&& __deque) noexcept
: front_block_(nullptr), back_block_(nullptr), front_element_(nullptr), back_element_(nullptr), size_(0)
{
    front_block_ = __deque.front_block_;
    back_block_ = __deque.back_block_;
    front_element_ = __deque.front_element_;
    back_element_ = __deque.back_element_;
    size_ = __deque.size_;

    __deque.front_block_ = nullptr;
    __deque.back_block_ = nullptr;
    __deque.front_element_ = nullptr;
    __deque.back_element_ = nullptr;
    __deque.size_ = 0;
}

template <typename T>
Deque<T>::Deque(std::initializer_list<typename Deque<T>::value_type> __init_list)
{
    *this = std::move(Deque(__init_list.begin(), __init_list.end()));
}

template <typename T>
Deque<T>::~Deque()
{
    delete_all_block();
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& __deque)
{
    if (this != &__deque)
    {
        if constexpr (std::is_nothrow_copy_constructible_v<value_type>)
        {
            if (__deque.empty())
            {
                delete_all_element();
                return *this;
            }

            size_type required_block_size = 1;
            size_type prepared_block_size = 1;
            DequeBlock* origin_back_block = back_block_;
            DequeBlock* current_block = nullptr;

            // check required block size
            current_block = __deque.front_block_;
            while (current_block != __deque.back_block_)
            {
                current_block = current_block->next_block_;
                required_block_size++;
            }
            current_block = nullptr;

            // prepare block
            current_block = front_block_;
            while (current_block != back_block_)
            {
                current_block = current_block->next_block_;
                prepared_block_size++;
            }
            current_block = nullptr;

            try
            {
                while (prepared_block_size < required_block_size)
                {
                    prepare_next_block_of_back();
                    back_block_ = back_block_->next_block_;
                    prepared_block_size++;
                }
            }
            catch(const std::exception& bad_alloc_error)
            {
                back_block_ = origin_back_block;
                throw bad_alloc_error;
            }
            origin_back_block = nullptr; // no more used

            //delete all origin element
            delete_all_element();

            // copy all element of __deque
            front_element_ = front_block_->block_begin_ + (__deque.front_element_ - __deque.front_block_->block_begin_);
            back_element_ = front_element_ - 1;
            current_block = __deque.front_block_;
            pointer current_element = __deque.front_element_ - 1;

            while (current_element != __deque.back_element_)
            {
                current_element++;
                back_element_++;

                if (current_element == current_block->block_end_)
                {
                    current_block = current_block->next_block_;
                    current_element = current_block->block_begin_;
                    back_block_ = back_block_->next_block_;
                    back_element_ = back_block_->block_begin_;
                }

                new (back_element_) value_type(*current_element);
            }
            size_ = __deque.size_;
            current_block = nullptr;
            current_element = nullptr;
        }
        else
        {
            Deque tmp_deque(__deque);
            *this = std::move(tmp_deque);
        }
    }

    return *this;
}

template <typename T>
Deque<T>& Deque<T>::operator=(Deque&& __deque) noexcept
{
    delete_all_block();

    front_element_ = __deque.front_element_;
    back_element_ = __deque.back_element_;
    front_block_ = __deque.front_block_;
    back_block_ = __deque.back_block_;
    size_ = __deque.size_;
    
    __deque.front_element_ = nullptr;
    __deque.back_element_ = nullptr;
    __deque.front_block_ = nullptr;
    __deque.back_block_ = nullptr;
    __deque.size_ = 0;

    return *this;
}

template <typename T>
void Deque<T>::push_front(const Deque<T>::value_type& __value)
{
    if (empty())
    {
        if (front_block_ == nullptr)
        {
            front_block_ = make_new_block();
            back_block_ = front_block_;
        }

        front_element_ = front_block_->block_begin_ + (kDequeBlockElementCapacity_ - 1) / 2;
        back_element_ = front_element_;

        try
        {
            new (front_element_) value_type(__value);
        }
        catch (const std::exception& constructor_exception)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;

            throw constructor_exception;
        }
    }
    else
    {
        if (front_element_ == front_block_->block_begin_)
        {
            prepare_prev_block_of_front();
            front_block_ = front_block_->prev_block_;
            front_element_ = front_block_->block_end_ - 1;
        }
        else
        {
            front_element_--;
        }

        try
        {
            new (front_element_) value_type(__value);
        }
        catch(const std::exception& constructor_exception)
        {
            if (front_element_ == front_block_->block_end_ - 1)
            {
                front_block_ = front_block_->next_block_;
                front_element_ = front_block_->block_begin_;
            }
            else
            {
                front_element_++;
            }

            throw constructor_exception;
        }
    }

    size_++;
}

template <typename T>
void Deque<T>::push_front(Deque<T>::value_type&& __value)
{
    if (empty())
    {
        if (front_block_ == nullptr)
        {
            front_block_ = make_new_block();
            back_block_ = front_block_;
        }

        front_element_ = front_block_->block_begin_ + (kDequeBlockElementCapacity_ - 1) / 2;
        back_element_ = front_element_;

        try
        {
            new (front_element_) value_type(std::move(__value));
        }
        catch (const std::exception& constructor_exception)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;

            throw constructor_exception;
        }
    }
    else
    {
        if (front_element_ == front_block_->block_begin_)
        {
            prepare_prev_block_of_front();
            front_block_ = front_block_->prev_block_;
            front_element_ = front_block_->block_end_ - 1;
        }
        else
        {
            front_element_--;
        }

        try
        {
            new (front_element_) value_type(std::move(__value));
        }
        catch (const std::exception& constructor_exception)
        {
            if (front_element_ == front_block_->block_end_ - 1)
            {
                front_block_ = front_block_->next_block_;
                front_element_ = front_block_->block_begin_;
            }
            else
            {
                front_element_++;
            }

            throw constructor_exception;
        }
    }

    size_++;
}

template <typename T>
void Deque<T>::push_back(const Deque<T>::value_type& __value)
{
    if (empty())
    {
        if (back_block_ == nullptr)
        {
            back_block_ = make_new_block();
            front_block_ = back_block_;
        }

        back_element_ = back_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        front_element_ = back_element_;

        try
        {
            new (back_element_) value_type(__value);
        }
        catch (const std::exception& constructor_exception)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;

            throw constructor_exception;
        }
    }
    else
    {
        if (back_element_ == back_block_->block_end_ - 1)
        {
            prepare_next_block_of_back();
            back_block_ = back_block_->next_block_;
            back_element_ = back_block_->block_begin_;
        }
        else
        {
            back_element_++;
        }

        try
        {
            new (back_element_) value_type(__value);
        }
        catch (const std::exception& constructor_exception)
        {
            if (back_element_ == back_block_->block_begin_)
            {
                back_block_ = back_block_->prev_block_;
                back_element_ = back_block_->block_end_ - 1;
            }
            else
            {
                back_element_--;
            }

            throw constructor_exception;
        }
    }

    size_++;
}

template <typename T>
void Deque<T>::push_back(Deque<T>::value_type&& __value)
{
    if (empty())
    {
        if (back_block_ == nullptr)
        {
            back_block_ = make_new_block();
            front_block_ = back_block_;
        }

        back_element_ = back_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        front_element_ = back_element_;

        try
        {
            new (back_element_) value_type(std::move(__value));
        }
        catch(const std::exception& constructor_exception)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;

            throw constructor_exception;
        }
    }
    else
    {
        if (back_element_ == back_block_->block_end_ - 1)
        {
            prepare_next_block_of_back();
            back_block_ = back_block_->next_block_;
            back_element_ = back_block_->block_begin_;
        }
        else
        {
            back_element_++;
        }

        try
        {
            new (back_element_) value_type(std::move(__value));
        }
        catch (const std::exception& constructor_exception)
        {
            if (back_element_ == back_block_->block_begin_)
            {
                back_block_ = back_block_->prev_block_;
                back_element_ = back_block_->block_end_ - 1;
            }
            else
            {
                back_element_--;
            }

            throw constructor_exception;
        }
    }

    size_++;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(const_iterator __pos, const value_type& __value)
{
    if (empty())
    {
        if (back_block_ == nullptr)
        {
            back_block_ = make_new_block();
            front_block_ = back_block_;
        }

        back_element_ = back_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        front_element_ = back_element_;

        try
        {
            new (back_element_) value_type(__value);
        }
        catch(const std::exception& constructor_exception)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;

            throw constructor_exception;
        }

        __pos->block_pos_ = front_block_;
        __pos->element_pos_ = front_element_;

        return __pos;
    }
    else if (__pos == cend())
    {
        push_back(__value);

        return --end();
    }
    else
    {
        if constexpr (std::is_nothrow_move_constructible_v<value_type>)
        {
            push_back(std::move(back()));
        }
        else
        {
            push_back(back());
        }

        // current_iterator be end() - 1
        iterator current_iterator = std::move(--end());
        // current_next_iterator be end() - 1, current_iterator be end() - 2
        iterator current_next_iterator = current_iterator--;
        // current_iterator = end() - 2, current_next_iterator = end() - 1;

        if constexpr (std::is_nothrow_copy_assignable_v<value_type> || std::is_nothrow_move_assignable_v<value_type>)
        {
            while (current_iterator != __pos)
            {
                current_iterator--;
                current_next_iterator--;

                if constexpr (std::is_nothrow_move_assignable_v<value_type>)
                {
                    *current_next_iterator = std::move(*current_iterator);
                }
                else
                {
                    *current_next_iterator = *current_iterator;
                }
            }
            
            try
            {
                *current_iterator = __value;
            }
            catch(const std::exception& copy_assign_error)
            {
                // now, current_iterator == __pos;
                // error occured only specific condition where std::is_nothrow_move_assignable_v == true
                // so, move assign will not occure error.
                // and pop_back() is noexcept if deque.empty() == false;
                const const_iterator end_const_iter = cend();

                while (current_next_iterator != end_const_iter)
                {
                    *current_iterator = std::move(*current_next_iterator);

                    ++current_iterator;
                    ++current_next_iterator;
                }

                pop_back();
                throw copy_assign_error;
            }
        }
        else
        {
            Deque preserve_deque = *this;
            preserve_deque.pop_back();

            try
            {
                while (current_iterator != __pos)
                {
                    current_iterator--;
                    current_next_iterator--;

                    *current_next_iterator = std::move(*current_iterator);
                }

                *current_iterator = __value;
            }
            catch(const std::exception& assign_error)
            {
                *this = std::move(preserve_deque);
                throw assign_error;
            }
        }
        
        return __pos;
    }
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(const_iterator __pos, value_type&& __value)
{
    if (empty())
    {
        if (back_block_ == nullptr)
        {
            back_block_ = make_new_block();
            front_block_ = back_block_;
        }

        back_element_ = back_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        front_element_ = back_element_;

        try
        {
            new (back_element_) value_type(std::move(__value));
        }
        catch(const std::exception& constructor_exception)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;

            throw constructor_exception;
        }

        return { front_block_, front_element_ };
    }
    else if (__pos == cend())
    {
        push_back(std::move(__value));

        return --end();
    }
    else
    {
        if constexpr (std::is_nothrow_move_constructible_v<value_type>)
        {
            push_back(std::move(back()));
        }
        else
        {
            push_back(back());
        }

        // current_iterator be end() - 1
        iterator current_iterator = std::move(--end());
        // current_next_iterator be end() - 1, current_iterator be end() - 2
        iterator current_next_iterator = current_iterator--;
        // current_iterator = end() - 2, current_next_iterator = end() - 1;

        if constexpr (std::is_nothrow_copy_assignable_v<value_type> || std::is_nothrow_move_assignable_v<value_type>)
        {
            while (current_iterator != __pos)
            {
                current_iterator--;
                current_next_iterator--;

                if constexpr (std::is_nothrow_move_assignable_v<value_type>)
                {
                    *current_next_iterator = std::move(*current_iterator);
                }
                else
                {
                    *current_next_iterator = *current_iterator;
                }
            }
            
            try
            {
                *current_iterator = std::move(__value);
            }
            catch(const std::exception& move_assign_error)
            {
                // now, current_iterator == __pos;
                // error occured only specific condition where std::is_nothrow_copy_assignable_v == true
                // so, move assign will not occure error.
                // and pop_back() is noexcept if deque.empty() == false;
                const const_iterator end_const_iter = cend();

                while (current_next_iterator != end_const_iter)
                {
                    *current_iterator = *current_next_iterator;

                    ++current_iterator;
                    ++current_next_iterator;
                }

                pop_back();
                throw move_assign_error;
            }
        }
        else
        {
            Deque preserve_deque = *this;
            preserve_deque.pop_back();

            try
            {
                while (current_iterator != __pos)
                {
                    current_iterator--;
                    current_next_iterator--;

                    *current_next_iterator = std::move(*current_iterator);
                }

                *current_iterator = std::move(__value);
            }
            catch(const std::exception& assign_error)
            {
                *this = std::move(preserve_deque);
                throw assign_error;
            }
        }
        
        return __pos;
    }
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(const_iterator __pos, size_type __count, const value_type& __value)
{
    if (__count == 0)
    {
        return __pos;
    }
    else if (empty())
    {
        Deque<T> tmp_deque(__count, __value);
        *this = tmp_deque;
        return begin();
    }
    else if constexpr 
    ((
        std::is_nothrow_copy_constructible_v<value_type> || std::is_nothrow_move_constructible_v<value_type>) && 
        (std::is_nothrow_copy_assignable_v<value_type> || std::is_nothrow_move_assignable_v<value_type>)
    )
    {
        // 1) Free up space for insert
        DequeBlock* backup_back_block = back_block_;
        size_type need_block_size = (__count - ((back_block_->block_end_ - 1) - back_element_));
        need_block_size = (need_block_size + (kDequeBlockElementCapacity_ - 1)) / kDequeBlockElementCapacity_;

        try
        {
            for (size_t i = 0; i < need_block_size; i++)
            {
                prepare_next_block_of_back();
                back_block_ = back_block_->next_block_;
            }
        }
        catch(const std::exception& bad_alloc_error)
        {
            back_block_ = backup_back_block;
            throw bad_alloc_error;
        }

        // 2) moving value.
        iterator current_iter = end();
        iterator current_count_distance_iter = current_iter;
        const_iterator origin_end_iterator = end();
        const_iterator origin_back_iterator = --end();
        const_iterator new_back_iterator;

        for (size_type i = 0; i < __count; i++)
        {
            current_count_distance_iter++;
        }

        new_back_iterator = current_count_distance_iter;
        new_back_iterator--;
        
        while (current_count_distance_iter != origin_end_iterator && current_iter != __pos)
        {
            current_iter--;
            current_count_distance_iter--;

            if constexpr (std::is_nothrow_move_constructible_v<value_type>)
            {
                new (current_count_distance_iter.element_pos_) value_type(std::move(*current_iter));
            }
            else
            {
                new (current_count_distance_iter.element_pos_) value_type(*current_iter);
            }
        }

        while (current_iter != __pos)
        {
            current_iter--;
            current_count_distance_iter--;

            if constexpr (std::is_nothrow_move_assignable_v<value_type>)
            {
                *current_count_distance_iter = std::move(*current_iter);
            }
            else
            {
                *current_count_distance_iter = *current_iter;
            }
        }

        // 3) insert value
        iterator insert_pos = current_iter;
        size_type insert_count = 0;
        bool need_constructor_flag = false;

        try
        {
            for (; insert_count < __count && insert_pos != origin_end_iterator; insert_count++, insert_pos++)
            {
                *insert_pos = __value;
            }

            if (insert_count < __count)
            {
                need_constructor_flag = true;
            }

            for (; insert_count < __count; insert_count++, insert_pos++)
            {
                new (insert_pos.element_pos_) value_type(__value);
            }
        }
        catch(const std::exception& copy_assign_or_constructor_error)
        {
            if (need_constructor_flag)
            {
                while (insert_pos != origin_end_iterator)
                {
                    insert_pos--;

                    insert_pos->~value_type();
                }
            }

            while (current_iter != origin_end_iterator)
            {
                if constexpr(std::is_nothrow_move_assignable_v<value_type>)
                {
                    *current_iter = std::move(*current_count_distance_iter);
                }
                else
                {
                    *current_iter = *current_count_distance_iter;
                }

                current_iter++, current_count_distance_iter++;
            }

            throw copy_assign_or_constructor_error;
        }

        back_block_ = new_back_iterator.block_pos_;
        back_element_ = new_back_iterator.element_pos_;
        size_ += __count;

        return __pos;
    }
    else
    {
        Deque tmp_deque(__count, __value);
        iterator new_pos = tmp_deque.begin();
        
        for (iterator it = begin(); it != __pos; it++)
        {
            if constexpr (std::is_nothrow_move_assignable_v<value_type>)
            {
                tmp_deque.push_front(std::move(*it));
            }
            else
            {
                tmp_deque.push_front(*it);
            }
        }

        for (iterator it = --end(); it != __pos; it--)
        {
            if constexpr (std::is_nothrow_move_assignable_v<value_type>)
            {
                tmp_deque.push_back(std::move(*it));
            }
            else
            {
                tmp_deque.push_back(*it);
            }
        }

        if constexpr (std::is_nothrow_move_assignable_v<value_type>)
        {
            tmp_deque.push_back(std::move(*__pos.element_pos_));
        }
        else
        {
            tmp_deque.push_back(*__pos.element_pos_);
        }

        return new_pos;
    }
}

template <typename T>
template <typename InputIt>
typename Deque<T>::iterator Deque<T>::insert(const_iterator __pos, InputIt __begin, InputIt __end)
{

    if (__begin == __end)
    {
        return __pos;
    }
    else if (empty())
    {
        Deque<T> tmp_deque(__begin, __end);
        *this = tmp_deque;
        return begin();
    }
    else if constexpr 
    (
        (std::is_nothrow_copy_constructible_v<value_type> || std::is_nothrow_move_constructible_v<value_type>) && 
        (std::is_nothrow_copy_assignable_v<value_type> || std::is_nothrow_move_assignable_v<value_type>)
    )
    {
        // 0) Get size
        size_t input_size = 0;

        for (auto it = __begin; it != __end; it++)
        {
            input_size++;
        }

        // 1) Free up space for insert
        DequeBlock* backup_back_block = back_block_;
        size_type need_block_size = (input_size - ((back_block_->block_end_ - 1) - back_element_));
        need_block_size = (need_block_size + (kDequeBlockElementCapacity_ - 1)) / kDequeBlockElementCapacity_;

        try
        {
            for (size_t i = 0; i < need_block_size; i++)
            {
                prepare_next_block_of_back();
                back_block_ = back_block_->next_block_;
            }
        }
        catch(const std::exception& bad_alloc_error)
        {
            back_block_ = backup_back_block;
            throw bad_alloc_error;
        }

        // 2) moving value.
        iterator current_iter = end();
        iterator current_count_distance_iter = current_iter;
        const_iterator origin_end_iterator = end();
        const_iterator origin_back_iterator = --end();
        const_iterator new_back_iterator;

        for (size_type i = 0; i < input_size; i++)
        {
            current_count_distance_iter++;
        }

        new_back_iterator = current_count_distance_iter;
        new_back_iterator--;

        while (current_count_distance_iter != origin_end_iterator && current_iter != __pos)
        {
            current_iter--;
            current_count_distance_iter--;

            if constexpr (std::is_nothrow_move_constructible_v<value_type>)
            {
                new (current_count_distance_iter.element_pos_) value_type(std::move(*current_iter));
            }
            else
            {
                new (current_count_distance_iter.element_pos_) value_type(*current_iter);
            }
        }

        while (current_iter != __pos)
        {
            current_iter--;
            current_count_distance_iter--;

            if constexpr (std::is_nothrow_move_assignable_v<value_type>)
            {
                *current_count_distance_iter = std::move(*current_iter);
            }
            else
            {
                *current_count_distance_iter = *current_iter;
            }
        }

        // 3) insert value
        iterator insert_pos = current_iter;
        auto input_currernt_iter = __begin;
        size_type insert_count = 0;
        bool need_constructor_flag = false;

        try
        {
            for (; insert_count < input_size && insert_pos != origin_end_iterator; insert_count++, insert_pos++)
            {
                *insert_pos = *input_currernt_iter;
                input_currernt_iter++;
            }

            if (insert_count < input_size)
            {
                need_constructor_flag = true;
            }

            for (; insert_count < input_size; insert_count++, insert_pos++)
            {
                new (insert_pos.element_pos_) value_type(*input_currernt_iter);
                input_currernt_iter++;
            }
        }
        catch(const std::exception& copy_assign_or_constructor_error)
        {
            if (need_constructor_flag)
            {
                while (insert_pos != origin_end_iterator)
                {
                    insert_pos--;

                    insert_pos->~value_type();
                }
            }

            while (current_iter != origin_end_iterator)
            {
                if constexpr(std::is_nothrow_move_assignable_v<value_type>)
                {
                    *current_iter = std::move(*current_count_distance_iter);
                }
                else
                {
                    *current_iter = *current_count_distance_iter;
                }

                current_iter++, current_count_distance_iter++;
            }

            throw copy_assign_or_constructor_error;
        }

        back_block_ = new_back_iterator.block_pos_;
        back_element_ = const_cast<pointer>(new_back_iterator.element_pos_);
        size_ += input_size;

        return __pos;
    }
    else
    {
        Deque tmp_deque(__begin, __end);
        iterator new_pos = tmp_deque.begin();
        
        for (iterator it = begin(); it != __pos; it++)
        {
            if constexpr (std::is_nothrow_move_assignable_v<value_type>)
            {
                tmp_deque.push_front(std::move(*it));
            }
            else
            {
                tmp_deque.push_front(*it);
            }
        }

        for (iterator it = --end(); it != __pos; it--)
        {
            if constexpr (std::is_nothrow_move_assignable_v<value_type>)
            {
                tmp_deque.push_back(std::move(*it));
            }
            else
            {
                tmp_deque.push_back(*it);
            }
        }

        if constexpr (std::is_nothrow_move_assignable_v<value_type>)
        {
            tmp_deque.push_back(std::move(*__pos));
        }
        else
        {
            tmp_deque.push_back(*__pos);
        }

        return new_pos;
    }
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(const_iterator __pos, std::initializer_list<value_type> __init_list)
{
    return insert(__pos, __init_list.begin(), __init_list.end());
}

template <typename T>
void Deque<T>::pop_front()
{
    front_element_->~value_type();
    size_--;

    if (empty())
    {
        front_element_ = nullptr;
        back_element_ = nullptr;
    }
    else
    {
        if (front_element_ == front_block_->block_end_ - 1)
        {
            front_block_ = front_block_->next_block_;
            front_element_ = front_block_->block_begin_;
        }
        else
        {
            front_element_++;
        }
    }
}

template <typename T>
void Deque<T>::pop_back()
{
    back_element_->~value_type();
    size_--;

    if (empty())
    {
        back_element_ = nullptr;
        front_element_ = nullptr;
    }
    else
    {
        if (back_element_ == back_block_->block_begin_)
        {
            back_block_ = back_block_->prev_block_;
            back_element_ = back_block_->block_end_ - 1;
        }
        else
        {
            back_element_--;
        }
    }
}

template <typename T>
void Deque<T>::clear() noexcept
{
    delete_all_element();
}

template <typename T>
T& Deque<T>::front()
{
    return *front_element_;
}

template <typename T>
const T& Deque<T>::front() const
{
    return *front_element_;
}

template <typename T>
T& Deque<T>::back()
{
    return *back_element_;
}

template <typename T>
const T& Deque<T>::back() const
{
    return *back_element_;
}

template <typename T>
bool Deque<T>::empty() const noexcept
{
    return size_ == 0;
}

template <typename T>
typename Deque<T>::size_type Deque<T>::size() const noexcept
{
    return size_;
}

template <typename T>
void Deque<T>::resize(Deque<T>::size_type __count, const Deque<T>::value_type& __value)
{
    if (__count > size_)
    {
        size_type original_size = size_;

        if (empty())
        {
            push_back(__value);
        }

        try
        {
            while (size_ != __count)
            {
                back_element_++;

                if (back_element_ == back_block_->block_end_)
                {
                    prepare_next_block_of_back();
                    back_block_ = back_block_->next_block_;
                    back_element_ = back_block_->block_begin_;
                }

                new (back_element_) value_type(__value);

                size_++;
            }
        }
        catch (const std::exception& constructor_exception)
        {
            back_element_--;

            while (size_ != original_size)
            {
                back_element_->~value_type();

                if (back_element_ == back_block_->block_begin_)
                {
                    back_block_ = back_block_->prev_block_;
                    back_element_ = back_block_->block_end_ - 1;
                }
                else
                {
                    back_element_--;
                }

                size_--;
            }

            if (size_ == 0)
            {
                front_element_ = nullptr;
                back_element_ = nullptr;
            }

            throw constructor_exception;
        }
    }
    else
    {
        while (size_ != __count)
        {
            back_element_->~value_type();

            if (back_element_ == back_block_->block_begin_)
            {
                back_block_ = back_block_->prev_block_;
                back_element_ = back_block_->block_end_ - 1;
            }
            else
            {
                back_element_--;
            }

            size_--;
        }

        if (size_ == 0)
        {
            front_element_ = nullptr;
            back_element_ = nullptr;
        }
    }
}

template <typename T>
void Deque<T>::shrink_to_fit() noexcept
{
    if (size_ == 0)
    {
        delete_all_block();
        return;
    }

    DequeBlock* current_block = nullptr;
    
    current_block = front_block_;
    if (current_block != nullptr)
    {
        current_block = current_block->prev_block_;
        front_block_->prev_block_ = nullptr;
    }

    while (current_block != nullptr)
    {
        DequeBlock* prev_current_block = current_block->prev_block_;
        operator delete(current_block);
        current_block = prev_current_block;
    }

    current_block = back_block_;
    if (current_block != nullptr)
    {
        current_block = current_block->next_block_;
        back_block_->next_block_ = nullptr;
    }

    while (current_block != nullptr)
    {
        DequeBlock* next_current_block = current_block->next_block_;
        operator delete(current_block);
        current_block = next_current_block;
    }
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() const noexcept
{
    return iterator(front_block_, front_element_);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() const noexcept
{
    if (back_element_ + 1 == back_block_->block_end_)
    {
        return iterator(back_block_->next_block_, back_block_->next_block_->block_begin_);
    }
    else
    {
        return iterator(back_block_, back_element_ + 1);
    }
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const noexcept
{
    return const_iterator(front_block_, front_element_);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const noexcept
{
    return const_iterator(back_block_, back_element_ + 1);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() const noexcept
{
    return reverse_iterator(end());
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() const noexcept
{
    return reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const noexcept
{
    return const_reverse_iterator(cbegin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const noexcept
{
    return const_reverse_iterator(cend());
}

template <typename T>
void Deque<T>::prepare_prev_block_of_front()
{
    if (front_block_->prev_block_ == nullptr)
    {
        if (back_block_->next_block_ == nullptr)
        {
            DequeBlock* new_block = make_new_block();
            link_block(new_block, front_block_);
            new_block = nullptr;
        }
        else
        {
            DequeBlock* replace_block = back_block_->next_block_;
            link_block(back_block_, replace_block->next_block_);
            replace_block->prev_block_ = nullptr;
            link_block(replace_block, front_block_);
            replace_block = nullptr;
        }
    }
}

template <typename T>
void Deque<T>::prepare_next_block_of_back()
{
    if (back_block_->next_block_ == nullptr)
    {
        if (front_block_->prev_block_ == nullptr)
        {
            DequeBlock* new_block = make_new_block();
            link_block(back_block_, new_block);
            new_block = nullptr;
        }
        else
        {
            DequeBlock* replace_block = front_block_->prev_block_;
            link_block(replace_block->prev_block_, front_block_);
            replace_block->next_block_ = nullptr;
            link_block(back_block_, replace_block);
            replace_block = nullptr;
        }
    }
}

template <typename T>
void Deque<T>::delete_all_element() noexcept
{
    if (empty() == false)
    {
        while (front_element_ != back_element_)
        {
            back_element_->~value_type();
            
            if (back_element_ == back_block_->block_begin_)
            {
                back_block_ = back_block_->prev_block_;
                back_element_ = back_block_->block_end_ - 1;
            }
            else
            {
                back_element_--;
            }
        }

        back_element_->~value_type();
        front_element_ = nullptr;
        back_element_ = nullptr;
        size_ = 0;
    }
}

template <typename T>
void Deque<T>::delete_all_block() noexcept
{
    if (empty() == false)
    {
        delete_all_element();
    }

    if (front_block_ != nullptr)
    {
        while (front_block_->prev_block_ != nullptr)
        {
            front_block_ = front_block_->prev_block_;
        }

        while (front_block_ != nullptr)
        {
            DequeBlock* next_block = front_block_->next_block_;
            operator delete(front_block_);
            front_block_ = next_block;
        }

        front_block_ = nullptr;
        back_block_ = nullptr;
    }
}

template <typename T>
constexpr void Deque<T>::link_block(DequeBlock* __prev, DequeBlock* __next) noexcept
{
    if (__prev != nullptr) __prev->next_block_ = __next;
    if (__next != nullptr) __next->prev_block_ = __prev;
}

template <typename T>
typename Deque<T>::DequeBlock* Deque<T>::make_new_block()
{
    DequeBlock* new_block = reinterpret_cast<DequeBlock *>(operator new(kDequeBlockMemorySize_, std::align_val_t(alignof(DequeBlock))));

    new (new_block) DequeBlock{ nullptr, nullptr, nullptr, nullptr };
    uintptr_t block_address = reinterpret_cast<uintptr_t>(new_block + 1);
    block_address = (block_address + (alignof(value_type) - 1)) & ~(alignof(value_type) - 1);
    new_block->block_begin_ = reinterpret_cast<value_type *>(block_address);
    new_block->block_end_ = new_block->block_begin_ + kDequeBlockElementCapacity_;

    return new_block;
}

template <typename T>
DequeIterator<T>::DequeIterator() noexcept
: block_pos_(nullptr), element_pos_(nullptr)
{
}

// template <typename T>
// DequeIterator<T>::DequeIterator(const DequeIterator& __deque_iterator) noexcept
// {
//     block_pos_ = __deque_iterator.block_pos_;
//     element_pos_ = __deque_iterator.element_pos_;
// }

template <typename T>
DequeIterator<T>::DequeIterator(const DequeIterator<std::remove_const_t<value_type>>& __deque_iterator) noexcept
{
    block_pos_ = __deque_iterator.block_pos_;
    element_pos_ = reinterpret_cast<pointer>(__deque_iterator.element_pos_);
}

template <typename T>
DequeIterator<T>::DequeIterator(const DequeIterator<std::add_const_t<value_type>>& __deque_iterator) noexcept
{
    block_pos_ = __deque_iterator.block_pos_;
    element_pos_ = const_cast<pointer>(__deque_iterator.element_pos_);
}

template <typename T>
DequeIterator<T>::DequeIterator(typename Deque<std::remove_const_t<value_type>>::DequeBlock* __block_pos, T* __element_pos)
: block_pos_(__block_pos), element_pos_(__element_pos)
{
}

template <typename T>
DequeIterator<T>::~DequeIterator()
{
    element_pos_ = nullptr;
    block_pos_ = nullptr;
}

template <typename T>
DequeIterator<T>& DequeIterator<T>::operator=(const DequeIterator<T>& __deque_iterator) noexcept
{
    element_pos_ = __deque_iterator.element_pos_;
    block_pos_ = __deque_iterator.block_pos_;

    return *this;
}

template <typename T>
DequeIterator<T>& DequeIterator<T>::operator++() noexcept
{
    if (element_pos_ == block_pos_->block_end_ - 1 && block_pos_->next_block_ != nullptr)
    {
        block_pos_ = block_pos_->next_block_;
        element_pos_ = block_pos_->block_begin_;
    }
    else
    {
        element_pos_++;
    }

    return *this;
}

template <typename T>
T& DequeIterator<T>::operator*() noexcept
{
    return *element_pos_;
}

template <typename T>
bool DequeIterator<T>::operator==(const DequeIterator& __deque_iterator) const noexcept
{
    return element_pos_ == __deque_iterator.element_pos_;
}

template <typename T>
bool DequeIterator<T>::operator!=(const DequeIterator& __deque_iterator) const noexcept
{
    return element_pos_ != __deque_iterator.element_pos_;
}

template <typename T>
T* DequeIterator<T>::operator->() noexcept
{
    return element_pos_;
}

template <typename T>
DequeIterator<T> DequeIterator<T>::operator++(int) noexcept
{
    DequeIterator<T> it = *this;
    ++(*this);

    return it;
}

template <typename T>
DequeIterator<T>& DequeIterator<T>::operator--() noexcept
{
    if (element_pos_ == block_pos_->block_begin_ && block_pos_->prev_block_ != nullptr)
    {
        block_pos_ = block_pos_->prev_block_;
        element_pos_ = block_pos_->block_end_ - 1;
    }
    else
    {
        element_pos_--;
    }

    return *this;
}

template <typename T>
DequeIterator<T> DequeIterator<T>::operator--(int) noexcept
{
    DequeIterator<T> it = *this;
    --(*this);

    return it;
}