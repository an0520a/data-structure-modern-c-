#pragma once

#include <cstddef>

// Don't support random access
template <typename T>
class Deque
{
public:
    Deque();
    virtual ~Deque();
    Deque(const Deque& __deque);
    Deque(Deque&& __deque) noexcept;
    Deque& operator=(const Deque& __deque);
    Deque& operator=(Deque&& __deque) noexcept;
    void push_front(const T& value__);
    void push_front(T&& __value);
    void push_back(const T& __value);
    void push_back(T&& __value);
    void pop_front();
    void pop_back();
    T& front();
    const T& front() const;
    T& back();
    const T& back() const;
    bool empty() const noexcept;
    size_t size() const noexcept;
    void resize(size_t __count, const T& __value = T());
    void shrink_to_fit() noexcept;
protected:
    struct alignas(alignof(T)) DequeBlock
    {
        T* block_begin_;
        T* block_end_;
        DequeBlock* prev_block_;
        DequeBlock* next_block_;
    };

    static constexpr size_t determine_block_size();
    static constexpr size_t determine_block_element_capacity();
    static constexpr size_t kDequeBlockMemoryUnitSize_ = 512; // must be power of 2;
    static constexpr size_t kDequeBlockMemorySize_ = determine_block_size();
    static constexpr size_t kDequeBlockElementCapacity_ = (kDequeBlockMemorySize_ - sizeof(DequeBlock)) / sizeof(T);
    [[nodiscard]] DequeBlock* make_new_block();
    void delete_all_element() noexcept;
    void delete_all_block() noexcept;
    void constexpr link_block(DequeBlock* __prev, DequeBlock* __next) noexcept;
    void prepare_next_block_of_back();
    void prepare_prev_block_of_front();
    DequeBlock* front_block_;
    DequeBlock* back_block_;
    T* front_element_;
    T* back_element_;
    size_t size_;
};
// - Implementation Requirements -
// When there are no elements, front_element_ and back_element_ must be nullptr.
// front_block_ and back_block_ must point to blocks containing elements pointed to by
// front_element_ and back_element_, respectively. 
// However, when the size is 0, front_block_ and back_block_ can 
// point to nullptr (when there is no created block) 
// or a block with empty elements (when a created block already exists).
// You must always be able to access back_block_ from front_block_ through member variable next_block_. 
// Conversely, back_block_ must be able to access front_block_ via the prev_block_ member variable. 
// However, access may not be possible if front_element_ and back_element are in the same block. 
// Instead, in this case, front_block_ and back_block_ should always point to the same block.
// Also, it should not be possible to access back_block_ from front_block_ via the member variable prev_block_. 
// Conversely, front_block_ should not be accessible from back_block_ via member variable next_block_.
// size_ should always indicate the total number of elements present in all blocks.
// All functions should have strong exception guarantees.

#include "deque.cpp"