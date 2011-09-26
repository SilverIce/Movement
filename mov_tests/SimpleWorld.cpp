


#include "SimpleWorld.h"
#include <Windows.h>
#include <conio.h>

extern void test();

int main()
{
    test();
    getch();
    //Sleep(0xFFFFFF);
    return 0;
}

void World::Run()
{
    uint32 realCurrTime = 0;
    uint32 realPrevTime = GetTickCount();
    uint32 prevSleepTime = 0;                               // used for balanced full tick time length near WORLD_SLEEP_CONST

    InitWorld();
    state_run = true;

    while ( state_run == true )
    {
        realCurrTime = GetTickCount();
        uint32 diff = (MSTime(realCurrTime) - MSTime(realPrevTime)).time;

        Update(diff);         // takes some time
        realPrevTime = realCurrTime;

        if (diff <= SLEEP_TIME + prevSleepTime)
        {
            prevSleepTime = SLEEP_TIME + prevSleepTime-diff;
            Sleep(prevSleepTime);
        }
        else
            prevSleepTime = 0;
    }
}

World::~World()
{
    for(std::vector<TestArea*>::iterator it = m_tests.begin();it!=m_tests.end(); ++it)
        delete (*it);
}

void World::Update(const uint32 diff)
{
    for(std::vector<TestArea*>::iterator it = m_tests.begin();it!=m_tests.end(); ++it)
    {
        if((*it)->running())
           (*it)->Update(diff);
    }
}

World::World()
{
    SLEEP_TIME = def_sleep_time;
}

World& World::instance()
{
    static World si;
    return si;
}

void World::register_test( TestArea &t )
{
    m_tests.push_back(&t);
}

TestArea::TestArea() : state_run(true)
{
    sWorld.register_test(*this);
}