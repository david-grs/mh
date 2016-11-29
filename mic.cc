#include "mic.h"

int main()
{
    hash_array<int, double, empty_key<int, 0>> mha;

    mha.insert(std::make_pair(1, 2.0));
    mha.insert(std::make_pair(2, 2.0));
    mha.insert(std::make_pair(3, 2.0));
    mha.insert(std::make_pair(4, 2.0));
}
