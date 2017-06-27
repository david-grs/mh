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

#ifdef __cpp_template_auto

    mic<Person,
        unordered<
            by_first_name,
            member<&Person::first_name>
        >,
        ordered<
            by_age,
            member<&Person::age>
        >,
        ordered<
            by_age,
            member<&Person::age>
        >> _mic;

#else

    mic<Person,
        unordered<
            by_first_name,
            member<Person, std::string, &Person::first_name>
        >,
        ordered<
            by_age,
            member<Person, int, &Person::age>
        >,
        ordered<
            by_age,
            member<Person, int, &Person::age>
        >> _mic;

#endif
};

TEST_F(TestMultiIndexContainer, emplace_hash)
{
    auto v = _mic.index<by_first_name>();
    v.emplace("lalal", new Person{"dada", "dd", 1});
    EXPECT_EQ(1, int(_mic.size()));
}

TEST_F(TestMultiIndexContainer, emplace_map)
{
    auto v = _mic.index<1>();
    v.emplace(324, new Person{"dada", "dd", 1});
    EXPECT_EQ(1, int(_mic.size()));
}

TEST_F(TestMultiIndexContainer, find)
{
    auto v = _mic.index<0>();
    auto p = v.find(std::string("lalal"));
    EXPECT_EQ(0, int(_mic.size()));
}

TEST_F(TestMultiIndexContainer, find_const)
{
    const auto& m = _mic;
    auto v = m.index<0>();
    auto p = v.find(std::string("lalal"));
    EXPECT_EQ(0, int(_mic.size()));
}

TEST_F(TestMultiIndexContainer, iterator)
{
    EXPECT_TRUE(_mic.cbegin() == _mic.cend());
    EXPECT_TRUE(_mic.begin() == _mic.end());
}

TEST_F(TestMultiIndexContainer, clear)
{
    _mic.index<0>().emplace("lalal", new Person{"dada", "dd", 1});

    _mic.clear();
    EXPECT_EQ(0, int(_mic.size()));
}

TEST_F(TestMultiIndexContainer, empty)
{
    EXPECT_TRUE(_mic.empty());
}

TEST_F(TestMultiIndexContainer, erase)
{
    _mic.index<0>().emplace("lalal", new Person{"dada", "dd", 1});

    _mic.erase(std::string("lalal"));
    EXPECT_TRUE(_mic.empty());
}

