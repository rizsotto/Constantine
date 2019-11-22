// RUN: %verify_const %s
// expected-no-diagnostics

#include <cstdlib>

void * operator new (size_t, void * p) throw();
void * operator new[] (size_t, void * p) throw();
void operator delete (void *, void *) throw();
void operator delete[] (void *, void *) throw();

struct TestType {
    int const i;
    int const j;
    int const k;

    TestType()
        : i(1)
        , j(2)
        , k(3)
    { }

   ~TestType()
    { }
};


class PlacementNewTest {
private:
    unsigned char buffer[sizeof(TestType)];

public:
    PlacementNewTest()
        : buffer()
    { }

   ~PlacementNewTest()
    { }

    TestType * reset() {
        return new(buffer) TestType();
    }
};

class PlacementNewArrayTest {
private:
    unsigned char buffer[2 * sizeof(TestType)];

public:
    PlacementNewArrayTest()
        : buffer()
    { }

   ~PlacementNewArrayTest()
    { }

    TestType * reset() {
        return new(buffer) TestType[2];
    }
};

class PlacementDeleteTest {
private:
    unsigned char buffer[sizeof(TestType)];

public:
    PlacementDeleteTest()
        : buffer()
    { }

   ~PlacementDeleteTest() {
        TestType * const ptr = (TestType*)&buffer;
        ptr->~TestType();
    }
};

