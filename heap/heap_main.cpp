#include <iostream>
#include <algorithm>
#include <vector>
#include "heap.hpp"

int main()
{
    Heap<int, std::less<int>> heap;

    heap.push(4);
    heap.push(1);
    heap.push(7);
    heap.push(2);
    heap.push(8);
    heap.push(3);
    heap.push(6);
    heap.push(5);
    heap.push(9);
    heap.push(0);

    while(!heap.empty())
    {
        std::cout << heap.top() << std::endl;
        heap.pop();
    }
}