
#pragma once

#include "typedefs.h"
#include "Node.h"

namespace Movement
{
    class Movement;



    class SplineState;

    /// Position management
    class LinearMover
    {
    protected:

        Vector3& position;
        uint32& time_stamp;
        //float local_speed;

        uint32& move_time_passed;
        uint32& move_time_full;

        int32 elapsed_local;
        Vector3 direction;

        NodeList& path;
        NodeList::iterator current_node;

        void InitNode();

    public:

        LinearMover(class MovementState *s);

        // all these methtods should be called only while in running state

        // расчитано только на короткие промежутки времени, 
        // когда время diff намного меньше времени пути по одному node
        bool Update_short(const int32 &diff);

        // расчитано на любые промежутки времени
        bool UpdatePosition(const uint32 & curr_ms_time);

        void Initialize();
    };


    // object with short life-time
    class PathInitializer
    {
        MovementState &mov;
        NodeList& path;

    public:

        PathInitializer(MovementState &m);

        void PushPoint(Vector3 const &pos) { path.push_back(MoveNode(pos)); }
        void PreparePath();
    };




    #define MIN_POSITION_UPDATE_INTERVAL 100 // miliseconds, minimum time between position update calls
  

    /*
    1 .get the path (path from point A to B point) with some movemaps..

    2. select move type by current mode (flying, walking.. ect), select speed by movetype..

    3. calculate move time for each node, total move time,

    4. send following info to client.. move unit at server side too


    */


}
