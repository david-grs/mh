#include "mic.h"

#include <iostream>

int main()
{
    hash_array<int, double> mha(empty_key<int>(0));

    mha.insert(std::make_pair(1, 2.0));
    mha.insert(std::make_pair(2, 2.0));
    mha.insert(std::make_pair(3, 2.0));
    mha.insert(std::make_pair(4, 2.0));
    mha.insert(std::make_pair(1, 2.0));
    mha.insert(std::make_pair(2, 2.0));

    for (auto& a : mha)
        std::cout << a.first << " " << a.second << std::endl;

    for (auto it = mha.begin(); it != mha.end(); ++it)
        std::cout << "foo: " << (*it).first << std::endl;

    auto it = mha.begin();

    for (int i = 0; i < 10; ++i)
    {
        it = (it == mha.end() ? mha.begin() : it);
        std::cout << (*it).first << std::endl;
        ++it;
    }

}
