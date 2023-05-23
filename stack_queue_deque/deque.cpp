#pragma once

#include <new>
#include "deque.hpp"


template <size_t N>
struct MsbPosStruct
{
    static constexpr size_t value = 1 + MsbPosStruct<(N >> 1)>::value;
};

template <>
struct MsbPosStruct<0>
{
    static constexpr size_t value = 0;
};

template <size_t N>
using MsbPos = struct MsbPosStruct<N>;

template <typename T>
constexpr size_t Deque<T>::determine_block_size()
{
    constexpr size_t required_minimum_memory = sizeof(Deque<T>::DequeBlock) + (alignof(T) - 1) + sizeof(T);

    if (required_minimum_memory <= kDequeBlockMemoryUnitSize_)
    {
        return kDequeBlockMemoryUnitSize_;
    }
    else
    {
        // Multiples of 512
        constexpr size_t calculate_size = 
        (required_minimum_memory + (kDequeBlockMemoryUnitSize_ - 1)) & ~(kDequeBlockMemoryUnitSize_ - 1);

        // Returns raised to a power of 2
        if (calculate_size == (1 << (MsbPos<calculate_size>::value - 1)))
        {
            return calculate_size;
        }
        else
        {
            return 1 << MsbPos<calculate_size>::value;
        }
    }
}

template <typename T>
constexpr size_t Deque<T>::determine_block_element_capacity()
{
    return (kDequeBlockMemorySize_ - (sizeof(DequeBlock) + alignof(T) - 1)) / sizeof(T);
}

template <typename T>
Deque<T>::Deque()
: front_block_(nullptr), back_block_(nullptr), front_element_(nullptr), back_element_(nullptr), size_(0)
{
}

template <typename T>
Deque<T>::Deque(const Deque& __deque)
{
    if (__deque.size_ == 0)
    {
        front_block_ = nullptr, back_block_ = nullptr;
        front_element_ = nullptr, back_element_ = nullptr;
        size_ = 0;
        return;
    }

    DequeBlock* current_block = __deque.front_block_;
    T* current_element = __deque.front_element_ - 1;
    front_block_ = make_new_block();
    back_block_ = front_block_;
    front_element_ = front_block_->block_begin_ + (__deque.front_element_ - __deque.front_block_->block_begin_);
    back_element_ = front_element_ - 1;

    do
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

        new (back_element_) T(*current_element);
    } while (current_element != __deque.back_element_);

    size_ = __deque.size_;
}

template <typename T>
Deque<T>::Deque(Deque&& __deque)
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
Deque<T>::~Deque()
{
    if (front_block_ == nullptr)
    {
        return;
    }

    while (front_element_ != back_element_)
    {
        back_element_->~T();
        
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

    if (back_element_ != nullptr)
    {
        back_element_->~T();
    }

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
    front_element_ = nullptr;
    back_element_ = nullptr;
}

template <typename T>
void Deque<T>::operator=(const Deque& __deque)
{
    if (back_element_ != nullptr)
    {
        while (front_element_ != back_element_)
        {
            back_element_->~T();
            
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

        back_element_->~T();
        front_element_ = nullptr;
        back_element_ = nullptr;
        size_ = 0;
    }

    if (__deque.size_ == 0)
    {
        return;
    }

    DequeBlock* current_block = __deque.front_block_;
    T* current_element = __deque.front_element_ - 1;

    if (front_block_ == nullptr)
    {
        front_block_ = make_new_block();
        back_block_ = front_block_;
    }
    front_element_ = front_block_->block_begin_ + (__deque.front_element_ - __deque.front_block_->block_begin_);
    back_element_ = front_element_ - 1;

    do
    {
        current_element++;
        back_element_++;

        if (current_element == current_block->block_end_)
        {
            if (back_block_->next_block_ == nullptr)
            {
                if (front_block_->prev_block_ != nullptr)
                {
                    DequeBlock* replace_block = front_block_->prev_block_;
                    link_block(replace_block->prev_block_, front_block_);
                    replace_block->next_block_ = nullptr;
                    link_block(back_block_, replace_block);
                    back_block_ = replace_block;
                }
                else
                {
                    DequeBlock* new_block = make_new_block();
                    link_block(back_block_, new_block);
                    back_block_ = new_block;
                    new_block = nullptr;
                }
            }

            current_block = current_block->next_block_;
            current_element = current_block->block_begin_;
            back_block_ = back_block_->next_block_;
            back_element_ = back_block_->block_begin_;
        }

        new (back_element_) T(*current_element);
    } while (current_element != __deque.back_element_);

    size_ = __deque.size_;
}

template <typename T>
void Deque<T>::operator=(Deque&& __deque)
{
    if (back_element_ != nullptr)
    {
        while (front_element_ != back_element_)
        {
            back_element_->~T();
            
            if (back_element_ == back_block_->block_begin_)
            {
                DequeBlock* current_block = back_block_;

                back_block_ = back_block_->prev_block_;
                back_element_ = back_block_->block_end_ - 1;

                operator delete(back_block_);
            }
            else
            {
                back_element_--;
            }
        }

        back_element_->~T();
        operator delete(back_block_);
    }

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
}

template <typename T>
void Deque<T>::push_front(const T& __value)
{
    if (front_element_ == nullptr)
    {
        if (front_block_ == nullptr)
        {
            front_block_ = make_new_block();
            back_block_ = front_block_;
        }

        front_element_ = front_block_->block_begin_ + (kDequeBlockElementCapacity_ - 1) / 2;
        back_element_ = front_element_;
    }
    else
    {
        if (front_element_ == front_block_->block_begin_)
        {
            if (front_block_->prev_block_ == nullptr)
            {
                if (back_block_->next_block_ == nullptr)
                {
                    DequeBlock* new_block = make_new_block();
                    link_block(new_block, front_block_);
                    front_block_ = new_block;
                    new_block = nullptr;
                }
                else
                {
                    DequeBlock* replace_block = back_block_->next_block_;
                    link_block(back_block_, replace_block->next_block_);
                    replace_block->prev_block_ = nullptr;
                    link_block(replace_block, front_block_);
                    front_block_ = replace_block;
                    replace_block = nullptr;
                }
            }

            front_block_ = front_block_->prev_block_;
            front_element_ = front_block_->block_end_ - 1;
        }
        else
        {
            front_element_--;
        }
    }

    new (front_element_) T(__value);
    size_++;
}

template <typename T>
void Deque<T>::push_front(T&& __value)
{
    if (front_element_ == nullptr)
    {
        if (front_block_ == nullptr)
        {
            front_block_ = make_new_block();
            back_block_ = front_block_;
        }

        front_element_ = front_block_->block_begin_ + (kDequeBlockElementCapacity_ - 1) / 2;
        back_element_ = front_element_;
    }
    else
    {
        if (front_element_ == front_block_->block_begin_)
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
            
            front_block_ = front_block_->prev_block_;
            front_element_ = front_block_->block_end_ - 1;
        }
        else
        {
            front_element_--;
        }
    }

    new (front_element_) T(std::move(__value));
    size_++;
}

template <typename T>
void Deque<T>::push_back(const T& __value)
{
    if (back_element_ == nullptr)
    {
        if (back_block_ == nullptr)
        {
            back_block_ = make_new_block();
            front_block_ = back_block_;
        }

        back_element_ = back_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        front_element_ = back_element_;
    }
    else
    {
        if (back_element_ == back_block_->block_end_ - 1)
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

            back_block_ = back_block_->next_block_;
            back_element_ = back_block_->block_begin_;
        }
        else
        {
            back_element_++;
        }
    }

    new (back_element_) T(__value);
    size_++;
}

template <typename T>
void Deque<T>::push_back(T&& __value)
{
    if (back_element_ == nullptr)
    {
        if (back_block_ == nullptr)
        {
            back_block_ = make_new_block();
            front_block_ = back_block_;
        }

        back_element_ = back_block_->block_begin_ + kDequeBlockElementCapacity_ / 2;
        front_element_ = back_element_;
    }
    else
    {
        if (back_element_ == back_block_->block_end_ - 1)
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

            back_block_ = back_block_->next_block_;
            back_element_ = back_block_->block_begin_;
        }
        else
        {
            back_element_++;
        }
    }

    new (back_element_) T(std::move(__value));
    size_++;
}

template <typename T>
void Deque<T>::pop_front()
{
    front_element_->~T();
    size_--;

    if (size_ == 0)
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
    back_element_->~T();
    size_--;

    if (size_ == 0)
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
size_t Deque<T>::size() const noexcept
{
    return size_;
}

template <typename T>
void Deque<T>::resize(size_t __count, const T& __value)
{
    if (__count > size_)
    {
        if (size_ == 0)
        {
            push_back(__value);
        }

        while (size_ != __count)
        {
            if (back_element_ == back_block_->block_end_ - 1)
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

                back_block_ = back_block_->next_block_;
                back_element_ = back_block_->block_begin_;
            }
            else
            {
                back_element_++;
            }

            new (back_element_) T(__value);

            size_++;
        }
    }
    else
    {
        while (size_ != __count)
        {
            back_element_->~T();

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
void Deque<T>::shrink_to_fit()
{
    DequeBlock* current_block = front_block_;
    if (current_block != nullptr)
    {
        current_block = current_block->prev_block_;
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
    }

    while (current_block != nullptr)
    {
        DequeBlock* next_current_block = current_block->next_block_;
        operator delete(current_block);
        current_block = next_current_block;
    }
}

template <typename T>
constexpr void Deque<T>::link_block(DequeBlock* __prev, DequeBlock* __next)
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
    block_address = (block_address + (alignof(T) - 1)) & ~(alignof(T) - 1);
    new_block->block_begin_ = reinterpret_cast<T *>(block_address);
    new_block->block_end_ = new_block->block_begin_ + kDequeBlockElementCapacity_;

    return new_block;
}