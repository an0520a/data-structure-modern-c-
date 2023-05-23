#include <iostream>
#include <deque>
#include "deque.hpp"

class TestElement
{
public:
    TestElement(int __value, bool* __ptr) 
    {
        ptr_ = __ptr, value_ = __value;
        *ptr_ = true;
        std::cout << "생성자 호출" << std::endl;
    }
    ~TestElement()
    {
        if (ptr_ != nullptr && flag_ != false)
        {
            *ptr_ = false;
            std::cout << "소멸자 호출" << std::endl;

        }
    }
    TestElement(const TestElement& __test_element)
    {
        ptr_ = __test_element.ptr_;
        value_ = __test_element.value_;
        flag_ = false;

        std::cout << "복사 생성자 호출" << std::endl;
    }
    TestElement(TestElement&& __test_element)
    {
        ptr_ = __test_element.ptr_;
        value_ = __test_element.value_;

        __test_element.ptr_ = nullptr;
        __test_element.ptr_ = nullptr;

        std::cout << "이동 생성자 호출" << std::endl;
    }
    bool operator==(const TestElement& __test_element) { return value_ == __test_element.value_; }
    bool operator!=(const TestElement& __test_element) { return !operator==(__test_element); }
    int value_;
    bool* ptr_;
    bool flag_ = true;
};

int main()
{
    std::pair<bool, bool> deallocated_check[10000];
    std::deque<TestElement> deq1;
    Deque<TestElement> deq2;

    for (int i = 0; i < 10000; i++)
    {
        if ((i & 0b11) ^ 0b11 != 0)
        {
            deq1.push_back(TestElement(i, &deallocated_check[i].first));
            deq2.push_back(TestElement(i, &deallocated_check[i].second));
        }
        else
        {
            deq1.push_front(TestElement(i, &deallocated_check[i].first));
            deq2.push_front(TestElement(i, &deallocated_check[i].second));
        }
    }

    for (int i = 0; i < 5000; i++)
    {
        if ((i & 0b110) ^ 0b110)
        {
            if (deq1.back() != deq2.back())
            {
                std::cout << "diffrent!" << std::endl;
                throw 1;
            }

            deq1.pop_back();
            deq2.pop_back();
        }
        else
        {
            if (deq1.front() != deq2.front())
            {
                std::cout << "diffrent!" << std::endl;
                throw 1;
            }

            deq1.pop_front();
            deq2.pop_front();
        }
    }

    // for (int i = 5000; i < 10000; i++)
    // {
    //     if ((i & 0b110) ^ 0b110)
    //     {
    //         std::cout << "deq1 : " << deq1.back().value_ << " | deq2 : " << deq2.back().value_ << std::endl;
    //         if (deq1.back() != deq2.back())
    //         {
    //             std::cout << "diffrent!" << std::endl;
    //             throw 1;
    //         }

    //         deq1.pop_back();
    //         deq2.pop_back();
    //     }
    //     else
    //     {
    //         std::cout << "deq1 : " << deq1.front().value_ << " | deq2 : " << deq2.front().value_ << std::endl;

    //         if (deq1.front() != deq2.front())
    //         {
    //             std::cout << "diffrent!" << std::endl;
    //             throw 1;
    //         }

    //         deq1.pop_front();
    //         deq2.pop_front();
    //     }
    // }

    std::cout << "next test" << std::endl;

    Deque<TestElement> deq3;
    bool test_arr[10000];
    for (int i = 0; i < 10000; i++)
    {
        printf("%p\n", test_arr);
        if ((i & 0b11) ^ 0b11 != 0)
        {
            deq3.push_back(TestElement(i, &test_arr[i]));
        }
        else
        {
            deq3.push_front(TestElement(i, &test_arr[i]));
            if (test_arr[i] != true) std::cout << "error" << std::endl;
        }
    }

    for (size_t i = 0; i < 10000; i++)
    {
        if (test_arr[i] != true)
        {
            std::cout << "allocated_error" << std::endl;
        }
    }

    deq3 = deq2;

    for (size_t i = 0; i < 10000; i++)
    {
        if (test_arr[i] != false)
        {
            std::cout << "dellocated_error" << std::endl;
        }
    }

    std::cout << deq1.size() << " " << deq3.size() << std::endl;

    for (int i = 5000; i < 10000; i++)
    {
        if ((i & 0b11) ^ 0b11 != 0)
        {
            std::cout << "deq1 : " << deq1.back().value_ << " | deq3 : " << deq3.back().value_ << std::endl;
            if (deq1.back() != deq3.back())
            {
                std::cout << "diffrent!" << std::endl;
                throw 1;
            }

            deq1.pop_back();
            deq3.pop_back();
        }
        else
        {
            std::cout << "deq1 : " << deq1.front().value_ << " | deq3 : " << deq3.front().value_ << std::endl;

            if (deq1.front() != deq3.front())
            {
                std::cout << "diffrent!" << std::endl;
                throw 1;
            }

            deq1.pop_front();
            deq3.pop_front();
        }
    }

    for (int i = 0 ; i < 10000; i++)
    {
        if (deallocated_check[i].first != false || deallocated_check[i].second != false)
        {
            throw 1;
        }
    }
}