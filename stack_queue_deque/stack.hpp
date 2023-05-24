#pragma once

#include "deque.hpp"

template <typename T>
class Stack
{
public:
    Stack();
    Stack(const Stack& __stack);
    Stack(Stack&& __stack);
    ~Stack();
    void operator=(const Stack& __stack);
    void operator=(Stack&& __stack);
    void push(const T& __value);
    void push(T&& __value);
    void pop();
    T& top();
    const T& top() const;
    size_t size() const;
    bool empty() const;
private:
    Deque<T> deque_;
};