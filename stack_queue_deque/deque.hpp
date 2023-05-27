#pragma once

#include <cstddef>
#include <iterator>

template <typename value_type>
struct DequeIterator;

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

    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = DequeIterator<value_type>;
    using const_iterator = DequeIterator<const value_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    void push_front(const value_type& value__);
    void push_front(value_type&& __value);
    void push_back(const value_type& __value);
    void push_back(value_type&& __value);
    iterator insert(const_iterator __pos, const value_type& __value);
    void pop_front();
    void pop_back();
    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;
    bool empty() const noexcept;
    size_type size() const noexcept;
    void resize(size_type __count, const value_type& __value = value_type());
    void shrink_to_fit() noexcept;
    iterator begin() const noexcept;
    iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    reverse_iterator rbegin() const noexcept;
    reverse_iterator rend() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;
protected:
    struct alignas(alignof(T)) DequeBlock
    {
        T* block_begin_;
        T* block_end_;
        DequeBlock* prev_block_;
        DequeBlock* next_block_;
    };

    static constexpr size_type determine_block_size();
    static constexpr size_type determine_block_element_capacity();
    static constexpr size_type kDequeBlockMemoryUnitSize_ = 512; // must be 512 * 2^n (where n is non-negative integer)
    static constexpr size_type kDequeBlockMemorySize_ = determine_block_size();
    static constexpr size_type kDequeBlockElementCapacity_ = determine_block_element_capacity();
    [[nodiscard]] DequeBlock* make_new_block();
    void delete_all_element() noexcept;
    void delete_all_block() noexcept;
    constexpr void link_block(DequeBlock* __prev, DequeBlock* __next) noexcept;
    void prepare_next_block_of_back();
    void prepare_prev_block_of_front();

    friend struct DequeIterator<value_type>;
    friend struct DequeIterator<const value_type>;

    DequeBlock* front_block_;
    DequeBlock* back_block_;
    value_type* front_element_;
    value_type* back_element_;
    size_type size_;
};
// - Implementation Requirements -
// When there are no elements, front_element_ and back_element_ must be nullptr.
// front_block_ and back_block_ must point to blocks containing elements pointed to by
// front_element_ and back_element_, respectively. 
// However, when the size is 0, front_block_ and back_block_ can  point to nullptr 
// (when there is no created block) 
// or a block with empty elements (when a created block already exists).
// You must always be able to access back_block_ from front_block_ through member variable next_block_. 
// Conversely, back_block_ must be able to access front_block_ via the prev_block_ member variable. 
// However, access may not be possible if front_element_ and back_element are in the same block. 
// Instead, in this case, front_block_ and back_block_ should always point to the same block.
// Also, it should not be possible to access back_block_ from front_block_ via the member variable prev_block_. 
// Conversely, front_block_ should not be accessible from back_block_ via member variable next_block_.
// size_ should always indicate the total number of elements present in all blocks.
// All functions should have strong exception guarantees.
// However, calling the pop_front and pop_back functions when the deque is empty is undefined behavior. 
// That is, it is the responsibility of the programmer to check whether 
// the deque is empty before calling the pop_front and pop_back functions. 
// Therefore, there is no strong exception guarantee for undefined behavior caused by 
// calling the pop_front and pop_back functions when the deque is empty.

template <typename T>
struct DequeIterator
{
public:
    // Legacy iterator requirment
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::bidirectional_iterator_tag;

    DequeIterator() noexcept;
    DequeIterator(const DequeIterator& __deque_iterator) noexcept;
    ~DequeIterator();
    DequeIterator& operator=(const DequeIterator&  __deque_iterator) noexcept;
    DequeIterator& operator++() noexcept;
    reference operator*();

    // Legacy input iterator requirement
    bool operator==(const DequeIterator& __deque_iterator) const noexcept;
    bool operator!=(const DequeIterator& __deque_iterator) const noexcept;
    pointer operator->() noexcept;

    // Legacy output iterator requirement
    DequeIterator operator++(int) noexcept;

    // Legacy bidirectional iterator requirement
    DequeIterator& operator--() noexcept;
    DequeIterator operator--(int) noexcept;
private:
    DequeIterator(typename Deque<std::remove_const_t<value_type>>::DequeBlock* __block_pos, value_type* __element_pos);
    typename Deque<std::remove_const_t<value_type>>::DequeBlock* block_pos_;
    pointer element_pos_;

    friend class Deque<std::remove_const_t<value_type>>;
};

#include "deque.cpp"