#pragma once

#include "deque.hpp"

template <typename value_type>
class Stack
{
public:
    Stack();
    Stack(const Stack& __stack);
    Stack(Stack&& __stack);
    ~Stack();
    void operator=(const Stack& __stack);
    void operator=(Stack&& __stack);
    void push(const value_type& __value);
    void push(value_type&& __value);
    void pop();
    value_type& top();
    const value_type& top() const;
    size_t size() const;
    bool empty() const;
private:
    Deque<value_type> deque_;
};