
#pragma once

#include "typedefs.h"
#include <vector>

using namespace Movement;


class TestArea;
class World
{
    std::vector<TestArea*>  m_tests;
    bool                    state_run;

    void InitWorld();
    void Update(const uint32 diff);

    World();
    ~World();

public:
    static World& instance();

    void Run();
    void stop() { state_run = false; }

    void register_test(TestArea &t);


    uint32 SLEEP_TIME;
    enum{
        def_sleep_time = 100,
    };
};

#define sWorld World::instance()


class TestArea
{
    bool state_run;
public:
    virtual ~TestArea() {}

    void stop() { state_run = false; }
    void run() { state_run = true; }
    bool running() const{ return state_run; }

    virtual void Update(const uint32 diff) = 0;
    virtual void InitTest() {}

    TestArea( );
};


