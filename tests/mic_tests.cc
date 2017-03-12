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
    struct by_first_name {};
    struct by_age {};

    mic<Person,
        unordered<
            by_first_name,
            member<Person, std::string, &Person::first_name>
        >,
        ordered<
            by_age,
            member<Person, int, &Person::age>
        >> _mic;
};

TEST_F(TestMultiIndexContainer, bla)
{
}

