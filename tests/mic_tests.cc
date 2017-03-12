#include "mic.h"
#include "fixture.h"

#include <gtest/gtest.h>

struct Person
{
    std::string first_name;
    std::string last_name;
    int age;
};

struct TestMultiIndexContainer : public ::testing::Test
{
    TestMultiIndexContainer()
    {
    }

protected:
    mic<Person, unordered<Person, std::string, &Person::first_name>> _mic;
};

TEST_F(TestMultiIndexContainer, bla)
{
}

