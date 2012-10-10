#include "Imports.h"
#include "framework/DelayInit.h"
#include "framework/gtest.h"

//#include "G3D/Vector3.h"
#include "framework/typedefs_p.h"
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

namespace Movement
{
    namespace
    {
        void FOnPositionChanged(WorldObject* obj, const Movement::Location&) {}
        void FBroadcastMessage(WorldObject* obj, Movement::MovementMessage& msg) {}
        void FBroadcastMessage(WorldObject* obj, const PacketData& msg) {}
        void FSendPacket(void * socket, const PacketData& msg) {}
        int32 FgetMSTime() { return (int32)GetTickCount();}

        void FGeneratePath(WorldObject *obj, const G3D::Vector3& fromVec,
            const G3D::Vector3& toVec, bool flightPath, QVector<G3D::Vector3>& path)
        {
            //path.push_back(fromVec);
            //path.push_back(toVec);
        }

        UnitMovement* FGetUnit(void* context, uint64 guid)
        {
            return NULL;
        }

        UnitMovement* FGetUnit2(WorldObject *obj, uint64 guid)
        {
            return NULL;
        }

        void FSpawnMark(WorldObject* obj, const Vector3& pos) {}
        void FSetUIntValue(WorldObject* obj, uint16 fieldIdx, uint32 value) {}
        uint32 FGetUIntValue(WorldObject* obj, uint16 fieldIdx) { return 0;}
        void FSetFloatValue(WorldObject* obj, uint16 fieldIdx, float value) {}
    }

    _Imports Imports = {
        &FOnPositionChanged,
        &FBroadcastMessage,
        &FBroadcastMessage,
        &FSendPacket,
        &FgetMSTime,
        &FGeneratePath,
        &FGetUnit,
        &FGetUnit2,
        &FSpawnMark,
        &FSetUIntValue,
        &FGetUIntValue,
    };

    void SetupImports(const _Imports& ftable) {
        Imports = ftable;
    }

    bool InitModule(const _Imports& ftable)
    {
        srand((unsigned int)time(NULL));
        ::delayInit::callCtors();
        bool testRes = testing::runTests(meta<testing::TestInfo>::getListConst());
        SetupImports(ftable);
        return testRes;
    }
}

extern "C" int EXPORT InitAndRunTests()
{
    return Movement::InitModule(Movement::Imports);
}

#if defined(_CONSOLE)
int main(int argc, char **argv)
{
    InitAndRunTests();
    return 0;
}
#endif
