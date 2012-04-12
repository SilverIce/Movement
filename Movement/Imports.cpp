#include "Imports.h"
#include "framework/DelayInit.h"

#include "G3D/Vector3.h"
#include "framework/typedefs_p.h"
#include <windows.h>

namespace testing {
    extern bool RunAllTests();
}

namespace Movement
{
    namespace
    {
        void FOnPositionChanged(WorldObject*, const Movement::Location&) {}
        void FBroadcastMessage(WorldObject const* obj, Movement::MovementMessage& msg) {}
        void FBroadcastMessage(WorldObject const* obj, WorldPacket& msg) {}
        void FSendPacket(void * socket, const WorldPacket& data) {}
        uint32 FgetMSTime() { return GetTickCount();}

        void FGeneratePath(WorldObject const* obj, const G3D::Vector3& fromVec,
            const G3D::Vector3& toVec, bool flightPath, std::vector<G3D::Vector3>& path)
        {
            path.push_back(fromVec);
            path.push_back(toVec);
        }

        UnitMovement* FGetUnit(void* context, uint64 guid)
        {
            return NULL;
        }

        UnitMovement* FGetUnit2(WorldObject const* obj, uint64 guid)
        {
            return NULL;
        }

        void FSpawnMark(WorldObject* obj, const Vector3& ) {}
        void FSetUIntValue(WorldObject* obj, uint16 fieldIdx, uint32) {}
        uint32 FGetUIntValue(WorldObject* obj, uint16 fieldIdx) { return 0;}
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
        SetupImports(ftable);
        ::delayInit::callCtors();
        return testing::RunAllTests();
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
