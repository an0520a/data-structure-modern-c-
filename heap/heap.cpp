#pragma once

#include <utility>
#include <algorithm>
#include <new>
#include "heap.hpp"

template <typename T, typename Compare>
Heap<T, Compare>::Heap(const Compare& __comp) : heap_arr_(nullptr), size_(0), capacity_(0)
{
    comp_ = __comp;
}

template <typename T, typename Compare>
Heap<T, Compare>::Heap(const T* __begin, const T* __end, const Compare& __comp) : 
heap_arr_(nullptr), size_(0), capacity_(0)
{
    if (__end >= __begin)
    {
        size_ = __end - __begin;
        capacity_ = guessCapacityBySize(size_);
        reserve(capacity_);

        for (size_t i = 0; i < size_; i++)
        {
            new (&heap_arr_[i]) T(__begin[i]);
        }

        for (size_t i = std::move(getParentIndex(size)); i > 0; i--)
        {
            heapIfy(i);
        }
        heapIfy(0);
    }

    comp_ = __comp;
}

template <typename T, typename Compare>
template <typename Iter>
Heap<T, Compare>::Heap(const Iter& __begin, const Iter& __end, const Compare& __comp) : heap_arr_(nullptr), size_(0), capacity_(0)
{
    if (__end > __begin)
    {
        size_ = std::distance(__begin, __end);
        capacity_ = guessCapacityBySize(size_);
        heap_arr_ = reserve(capacity_);

        for (size_t i = 0; i < size_; i++)
        {
            new (&heap_arr_[i]) T(*__begin);
            __begin++;
        }

        for (size_t i = std::move(getParentIndex(size)); i > 0; i--)
        {
            heapIfy(i);
        }
        heapIfy(0);
    }

    comp_ = __comp;
}

template <typename T, typename Compare>
Heap<T, Compare>::Heap(const Heap<T, Compare>& __heap)
: heap_arr_(nullptr), size_(__heap.size_), capacity_(__heap.capacity_)
{
    heap_arr_ = static_cast<T *>(operator new(capacity_ * sizeof(T)));

    for (size_t i = 0; i < size_; i++)
    {
        new (&heap_arr_[i]) T(__heap.heap_arr_[i]);
    }
}

template <typename T, typename Compare>
Heap<T, Compare>::Heap(Heap<T, Compare>&& __heap) noexcept
: heap_arr_(__heap.heap_arr_), size_(__heap.size_), capacity_(__heap.capacity_)
{
    __heap.heap_arr_ = nullptr;
    __heap.capacity_ = 0;
    __heap.size_ = 0;
}

template <typename T, typename Compare>
Heap<T, Compare>::~Heap()
{
    if (heap_arr_ != nullptr)
    {
        for (size_t i = 0; i < size_; i++)
        {
            heap_arr_[i].~T();
        }
        
        operator delete(heap_arr_);
    }
}

template <typename T, typename Compare>
void Heap<T, Compare>::reserve(const size_t __capacity)
{
    if (__capacity > capacity_)
    {
        T* new_heap_arr = nullptr;

        if (heap_arr_ == nullptr)
        {
            new_heap_arr = static_cast<T *>(operator new(__capacity * sizeof(T), std::align_val_t(alignof(T))));
        }
        else
        {
            new_heap_arr = static_cast<T *>(operator new(__capacity * sizeof(T), std::align_val_t(alignof(T))));
            
            for (size_t i = 0; i < size_; i++)
            {
                new (&new_heap_arr[i]) T(std::move(heap_arr_[i]));
                heap_arr_[i].~T();
            }

            operator delete(heap_arr_);
        }

        capacity_ = __capacity;
        heap_arr_ = new_heap_arr;
    }
}

template <typename T, typename Compare>
constexpr size_t Heap<T, Compare>::size() const noexcept
{
    return size_;
}

template <typename T, typename Compare>
constexpr size_t Heap<T, Compare>::capacity() const noexcept
{
    return capacity_;
}

template <typename T, typename Compare>
constexpr bool Heap<T, Compare>::empty() const noexcept
{
    return size_ == 0;
}

template <typename T, typename Compare>
constexpr bool Heap<T, Compare>::full() const noexcept
{
    return size_ == capacity_;
}

template <typename T, typename Compare>
const T& Heap<T, Compare>::top() const
{
    return *heap_arr_;
}

template <typename T, typename Compare>
void Heap<T, Compare>::pop()
{
    heap_arr_[0].~T();
    heap_arr_[0] = std::move(heap_arr_[size_ - 1]);
    heap_arr_[size_ - 1].~T();

    size_--;

    heapIfy(0);
}

template <typename T, typename Compare>
void Heap<T, Compare>::push(const T& __value)
{
    if (full())
    {
        reserve((capacity_ << 1) | 1);
    }

    new (&heap_arr_[size_]) T(__value);
    reverseHeapIfy(size_);

    size_++;
}

template <typename T, typename Compare>
void Heap<T, Compare>::push(T&& __value)
{
    if (full())
    {
        reserve((capacity_ << 1) | 1);
    }

    new (&heap_arr_[size_]) T(std::move(__value));
    reverseHeapIfy(size_);

    size_++;
}

template <typename T, typename Compare>
void Heap<T, Compare>::heapIfy(const size_t __i)
{
    size_t left_index = getLeftIndex(__i);
    size_t right_index = getRightIndex(__i);
    size_t extremum_index = __i;

    if (left_index < size_ && comp_(heap_arr_[left_index], heap_arr_[extremum_index]))
    {
        extremum_index = left_index;
    }
    if (right_index < size_ && comp_(heap_arr_[right_index], heap_arr_[extremum_index]))
    {
        extremum_index = right_index;
    }
    
    if (extremum_index != __i)
    {
        std::swap(heap_arr_[extremum_index], heap_arr_[__i]);
        heapIfy(extremum_index);
    }
}

template <typename T, typename Compare>
void Heap<T, Compare>::reverseHeapIfy(size_t __i)
{
    while(__i > 0 && comp_(heap_arr_[__i], heap_arr_[getParentIndex(__i)]))
    {
        std::swap(heap_arr_[__i], heap_arr_[getParentIndex(__i)]);
        __i = getParentIndex(__i);
    }
}

template <typename T, typename Compare>
constexpr size_t Heap<T, Compare>::getParentIndex(const size_t __i) const noexcept
{
    if (__i == 0)   return static_cast<size_t>(0);
    else            return (__i - 1) / 2;
}

template <typename T, typename Compare>
constexpr size_t Heap<T, Compare>::getLeftIndex(const size_t __i) const noexcept
{
    return 2 * __i + 1;
}

template <typename T, typename Compare>
constexpr size_t Heap<T, Compare>::getRightIndex(const size_t __i) const noexcept
{
    return 2 * __i + 2;
}


template <typename T, typename Compare>
constexpr size_t Heap<T, Compare>::guessCapacityBySize(size_t __size) const noexcept
{
    size_t guess_value = 0;

    if (__size != 0)
    {
        do
        {
            guess_value <<= 1;
            guess_value |= 1;
        } while(__size >>= 1);
    }

    return guess_value;
}