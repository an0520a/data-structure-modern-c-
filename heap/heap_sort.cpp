#include <iostream>
#include <algorithm>

template <typename T, typename Compare>
void HeapIfy(T* __arr, const size_t& __arr_size, const size_t& __index, const Compare& __comp)
{
    constexpr auto getLeftIndex = [](const size_t& __index) { return 2 * __index + 1; };
    constexpr auto getRightIndex = [](const size_t& __index) { return 2 * __index + 2; };

    size_t left_index = getLeftIndex(__index);
    size_t right_index = getRightIndex(__index);
    size_t extremum_index = __index;

    

    if (left_index < __arr_size && __comp(__arr[left_index], __arr[extremum_index]))
    {
        extremum_index = left_index;
    }
    if (right_index < __arr_size && __comp(__arr[right_index], __arr[extremum_index]))
    {
        extremum_index = right_index;
    }

    if (extremum_index != __index)
    {
        std::swap(__arr[__index], __arr[extremum_index]);
        HeapIfy(__arr, __arr_size, extremum_index, __comp);
    }
}

template <typename T, typename Compare = std::greater<T>>
void HeapSort(T* __begin, T* __end, Compare __comp = Compare())
{
    constexpr auto make_heap = [__comp](T* __arr, const size_t& __arr_size)->void
    {
        if (__arr_size != 0)
        {
            size_t last_parent_index = (__arr_size - 1) / 2;

            for (size_t i = std::move(last_parent_index); i > 0; i--)
            {
                HeapIfy(__arr, __arr_size, i, __comp);
            }
            HeapIfy(__arr, __arr_size, 0, __comp);
        }
    };

    size_t arr_size = __end - __begin;
    make_heap(__begin, arr_size);

    for (size_t i = arr_size - 1, j = 1; i > 0; i--, j++)
    {
        std::swap(__begin[0], __begin[i]);
        HeapIfy(__begin, arr_size - j, 0, __comp);
    }

    std::reverse(__begin, __end);
}

int main()
{
    int arr[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

    HeapSort(arr, arr + sizeof(arr) / sizeof(arr[0]));

    for (const int& val : arr)
    {
        std::cout << val << ' ';
    }
    std::cout << std::endl;
}