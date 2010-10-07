
#include "PathMover.h"
#include "movement.h"
#include "OutLog.h"
#include <assert.h>

namespace Movement
{

    bool LinearMover::Update_short( const int32 &diff )
    {
        assert(current_node != path.end() && "You have got a crash, gz!"); // никогда не д указывать в конец пути
        assert(current_node+1 != path.end()); // should be never updated when current node is last

        elapsed_local -= diff;

        move_time_passed += diff;

        bool arrived = false;
        if( elapsed_local > 0 )
        {
            double perc = double(elapsed_local) / double(current_node->time);

            const Vector3& dest = (current_node+1)->vec;
            position = dest - direction * perc;
        }
        else
        {
            ++current_node;
            if(current_node+1 == path.end())
                arrived = true;

            InitNode();
        }

        return arrived;
    }

    uint32 getMSTimeDiff(uint32 old_time, uint32 new_time)
    {
        if (new_time > old_time)
            return new_time - old_time;
        else
            return 0xFFFFFFFF - old_time + new_time;
    }

    bool LinearMover::UpdatePosition(const uint32 & curr_ms_time)
    {
        uint32 time_passed = getMSTimeDiff(time_stamp, curr_ms_time);

//         if( time_passed < MIN_POSITION_UPDATE_INTERVAL )     // should we care about this?
//             return false;

        bool path_passed = true;
        if( time_passed < elapsed_local )   // elapsed_local works like 'current_node->time'
        {
            path_passed = LinearMover::Update_short(time_passed);
        }
        else
        {
            time_passed -= elapsed_local;
            ++current_node;

            while(true)         // (current_node) == path.end()-1 - "current_node is last node"
            {
                if( current_node == (path.end()-1) )
                {
                    LinearMover::InitNode();
                    path_passed = true;
                    break;
                }

                if( time_passed < current_node->time )
                {
                    LinearMover::InitNode();
                    path_passed = LinearMover::Update_short(time_passed);
                    break;
                }

                time_passed -= current_node->time;
                ++current_node;
            }
        }

        time_stamp = curr_ms_time;

        return path_passed;
    }

    void LinearMover::InitNode()
    {
        sLog.write("Init new node");

        assert(current_node != path.end());

        const Vector3& start = current_node->vec;
        position = start;

        if(current_node+1 != path.end())
        {
            const Vector3& dest  = (current_node+1)->vec;
            direction = dest - start;

            elapsed_local += current_node->time; 
        }
        else    //конец пути, обнуляем все.. хотя надо ли?
        {
            elapsed_local = 0;
            direction = Vector3::zero();

            move_time_passed = move_time_full;
        }
    }

    LinearMover::LinearMover(MovementState *m) :
        position(m->position), path(m->spline.spline_path), time_stamp(m->spline.last_ms_time),
        move_time_full(m->spline.duration), move_time_passed(m->spline.time_passed)
    {
        elapsed_local = 0;
        current_node = path.end();
    }

    void LinearMover::Initialize()
    {
        assert(path.size() > 1);

        elapsed_local = 0;
        move_time_passed = 0;
        current_node = path.begin();
        InitNode();
    }

/*
    void PathInitializer::PreparePath()
    {
        assert(path.size() > 1);

        float speed = mov.GetCurrentSpeed();

        mov.duration = 0;
        for(NodeList::iterator node = path.begin(); node+1 != path.end(); ++node) 
        {
            const Vector3& begin = node->vec;
            const Vector3& dest = (node+1)->vec;

            float dist = (dest-begin).length();
            uint32 move_time = uint32((dist * 1000.0f)/speed);

            node->time = move_time;
            mov.duration += move_time;
        }
    }

    PathInitializer::PathInitializer(MoveData &m) : mov(m), path(m.spline_path)
    {
        // both actions may be wrong
        path.clear();
        PushPoint(mov.position);
    }*/


    //void LinearMover::RecalculatePath()
    //{
    //    assert(path.size() > 1);

    //    float perc = local_speed / mov.GetCurrentSpeed();

    //    elapsed_local = float(elapsed_local) / perc;

    //    mov.duration = 0;
    //    for(NodeList::iterator node = path.begin(); (node+1) != path.end(); ++node) 
    //    {
    //        node->time = float(node->time) / perc;
    //        mov.duration += node->time;
    //    }
    //}














}
