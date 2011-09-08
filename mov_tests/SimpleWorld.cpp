


#include "SimpleWorld.h"
#include <Windows.h>

extern void test();

int main()
{
    test();
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

/*
//#include "MoveTables.h"
#include "PathMover.h"
#include "MoveData.h"

using namespace Movement;


struct MoveTest : public TestArea
{

   MoveTest() : mdata(0), mover(mdata) {}

   MoveData mdata;
   NodeMover mover;

   void InitTest()
   {
       mdata.SetSpeed(MOVE_WALK,2.5);
       mdata.SetMoveType(MOVE_WALK);

       PathInitializer init(mdata);

       float step = 2.0f;

       for(float x = 0.0f; x < 16; x += step)
       {
           float y = x*2;
           init.PushPoint( Vector3(x,y,0) );
           t_log.Out("X %f Y %f", x, y);
       }

       init.PreparePath();

       mover.Initialize();

       t_log.Out("\n Log positions each 100 ms, full movetime %u", mdata.move_time_full);
   }

   void Update(const uint32 & diff)
   {
       if(mover.Update_short(diff))
           stop();

       Vector3& pos = mdata.curr_pos;
       t_log.Out("    x %f y %f z %f time %u", pos.x, pos.y, pos.z, mdata.move_time_passed);
   }
};



struct Corrd : public TestArea
{
        //double x=-nan(0x7fffff), y=2.74654499e-42, z=1.32813939e-18;
    void InitTest(){}
    void Update(const uint32 & diff)
    {
        uint32 crazy_x = ~0x7fffff;
        float x = *(float*)&crazy_x;
        if(MaNGOS::IsValidMapCoord(x))
            t_log.Out("is valid %f",x);
        else
            t_log.Out("invalid %f",x);

        stop();
    }

};*/


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