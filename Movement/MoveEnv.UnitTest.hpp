#include "MoveEnv.h"
#include "framework/gtest.h"

namespace Movement
{
    template<class T> void MoveEnv_basic()
    {
        {
            T entity;
            EXPECT_TRUE( entity.YawAngle() == 0.f );
            EXPECT_TRUE( entity.RollAngle() == 0.f );
            EXPECT_TRUE( entity.PitchAngle() == 0.f );
            EXPECT_TRUE( entity.RelativePosition().isZero() );
            EXPECT_TRUE( entity.Environment() == nullptr );
        }
        {
            T entity, environment;
            float yaw = G3D::uniformRandom(0.f, G3D::twoPi());
            float pitch = G3D::uniformRandom(0.f, G3D::twoPi());
            float roll = G3D::uniformRandom(0.f, G3D::twoPi());
            Vector3 offset = Vector3::random();

            entity.YawAngle(yaw);
            entity.PitchAngle(pitch);
            entity.RollAngle(roll);
            entity.RelativePosition(offset);
            entity.SetEnvironment(&environment);

            EXPECT_TRUE( entity.YawAngle() == yaw );
            EXPECT_TRUE( entity.PitchAngle() == pitch );
            EXPECT_TRUE( entity.RollAngle() == roll );
            EXPECT_TRUE( entity.RelativePosition() == offset );
            EXPECT_TRUE( entity.Environment() == &environment );
            
            entity.SetEnvironment(nullptr);
            EXPECT_TRUE( entity.Environment() == nullptr );
        }
    }

    template<class T> struct MovingEntity_RevolvableRandom : T
    {
        explicit MovingEntity_RevolvableRandom()
        {
            Vector3 platf_offset = Vector3::random()*10.f;
            float yaw = G3D::uniformRandom(0.f, G3D::twoPi());
            float pitch = G3D::uniformRandom(0.f, G3D::twoPi());
            float roll = G3D::uniformRandom(0.f, G3D::twoPi());

            RelativePosition(platf_offset);
            YawAngle(yaw);
            PitchAngle(pitch);
            RollAngle(roll);
        }
    };

    template<class T> void MoveEnv_rotation()
    {
        int testCount = 2;
        while(testCount-- > 0)
        {
            MovingEntity_RevolvableRandom<T> platform;
            MovingEntity_RevolvableRandom<T> entity;
            entity.SetEnvironment(&platform);

            Vector3 computedGlobal = G3D::Matrix3::fromEulerAnglesZXY(platform.YawAngle(),
                platform.PitchAngle(),platform.RollAngle())*entity.RelativePosition() + platform.RelativePosition();
            Vector3 resultGlobal = entity.GlobalPosition();
            EXPECT_TRUE( resultGlobal.fuzzyEq(computedGlobal) );

            entity.SetEnvironment(nullptr);
        }
    }

    template<class T> void MoveEnv_global_returns_same()
    {
        int testCount = 2;
        while(testCount-- > 0)
        {
            MovingEntity_RevolvableRandom<T> entity0;
            MovingEntity_RevolvableRandom<T> entity1;
            MovingEntity_RevolvableRandom<T> entity2;
            MovingEntity_RevolvableRandom<T> obj;

            entity1.SetEnvironment(&entity0);
            entity2.SetEnvironment(&entity1);
            obj.SetEnvironment(&entity2);

            const Vector3 global = obj.GlobalPosition();
            Vector3 globalNow;

            obj.SetEnvironment(&entity1);
            globalNow = obj.GlobalPosition();
            EXPECT_TRUE( global.fuzzyEq(globalNow) );

            obj.SetEnvironment(&entity0);
            globalNow = obj.GlobalPosition();
            EXPECT_TRUE( global.fuzzyEq(globalNow) );

            obj.SetEnvironment(nullptr);
            globalNow = obj.GlobalPosition();
            EXPECT_TRUE( global.fuzzyEq(globalNow) );

            entity0.SetEnvironment(nullptr);
            entity1.SetEnvironment(nullptr);
            entity2.SetEnvironment(nullptr);
            obj.SetEnvironment(nullptr);
        }
    }

    template<class T> void MoveEnv_LaunchTests()
    {
        MoveEnv_basic<T>();
        MoveEnv_rotation<T>();
        MoveEnv_global_returns_same<T>();
    }

    TEST(MoveEnv, MovingEntity_Revolvable) {
        MoveEnv_LaunchTests<MovingEntity_Revolvable>();
    }

    TEST(MoveEnv, MovingEntity_Revolvable2) {
        MoveEnv_LaunchTests<MovingEntity_Revolvable2>();
    }
}
