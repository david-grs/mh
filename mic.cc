#include "mic.h"

#include <iostream>

int main()
{
    hash_array<int, double, empty_key<int, 0>> mha;

    mha.insert(std::make_pair(1, 2.0));
    mha.insert(std::make_pair(2, 2.0));
    mha.insert(std::make_pair(3, 2.0));
    mha.insert(std::make_pair(4, 2.0));

    for (auto& a : mha)
        std::cout << a.first << " " << a.second << std::endl;
}
