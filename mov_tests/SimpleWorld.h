
#ifndef _SI_WORLD
#define _SI_WORLD

#include "typedefs.h"
#include <vector>

static const uint32 SLEEP_TIME = 100;

class TestArea;
class World
{
    void InitWorld();
public:
    ~World();
    World();

    bool run;
    void Run();
    void stop() { run = false; }
    void Update(const uint32 & diff);

    static World& instance();
};

#define sWorld World::instance()

static std::vector<TestArea*> m_tests_tt;

class TestArea
{
    bool runn;
public:
    ~TestArea() {}

    void stop() { runn = false; }
    void run() { runn = true; }
    bool running() const{ return runn; }

    virtual void Update(const uint32 & diff) = 0;
    virtual void InitTest() = 0;

    TestArea( )
    {
        InitTest();
        run();

        m_tests_tt.push_back(this);
    }
};


#endif
