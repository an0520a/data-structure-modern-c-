#pragma once

#include <cstddef>

template <typename T, typename Compare = std::greater<T>>
class Heap
{
public:
    Heap(const Compare& __comp = Compare());
    Heap(const T* __begin, const T* __end, const Compare& __comp = Compare());
    template <typename Iter> Heap(const Iter& begin, const Iter& end, const Compare& __comp = Compare());
    ~Heap();
    Heap(const Heap<T, Compare>& __heap);
    Heap(Heap<T, Compare>&& __heap) noexcept;
    constexpr size_t size() const noexcept;
    constexpr size_t capacity() const noexcept;
    constexpr bool empty() const noexcept;
    constexpr bool full() const noexcept;
    const T& top() const;
    void pop();
    void push(const T& __value);
    void push(T&& __value);
    void reserve(const size_t __capacity);
private:
    constexpr size_t getParentIndex(const size_t __i) const noexcept;
    constexpr size_t getLeftIndex(const size_t __i) const noexcept;
    constexpr size_t getRightIndex(const size_t __i) const noexcept;
    void heapSort();
    void heapIfy(const size_t __i);
    void reverseHeapIfy(size_t __i);
    constexpr size_t guessCapacityBySize(const size_t __size) const noexcept;

    T* heap_arr_;
    size_t size_;
    size_t capacity_;
    Compare comp_;
};

#include "heap.cpp"