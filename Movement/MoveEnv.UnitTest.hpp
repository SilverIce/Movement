#include "MoveEnv.h"

namespace Movement
{
    TEST(MoveEnv, basic)
    {
        {
            MovingEntity_Revolvable entity;
            EXPECT_TRUE( entity.YawAngle() == 0.f );
            EXPECT_TRUE( entity.RollAngle() == 0.f );
            EXPECT_TRUE( entity.PitchAngle() == 0.f );
            EXPECT_TRUE( entity.Position().isZero() );
        }
        {
            MovingEntity_Revolvable entity;
            float yaw = G3D::uniformRandom(0.f, G3D::twoPi());
            float pitch = G3D::uniformRandom(0.f, G3D::twoPi());
            float roll = G3D::uniformRandom(0.f, G3D::twoPi());
            Vector3 offset = Vector3::random();

            entity.YawAngle(yaw);
            entity.PitchAngle(pitch);
            entity.RollAngle(roll);
            entity.Position(offset);

            EXPECT_TRUE( entity.YawAngle() == yaw );
            EXPECT_TRUE( entity.PitchAngle() == pitch );
            EXPECT_TRUE( entity.RollAngle() == roll );
            EXPECT_TRUE( entity.Position() == offset );
        }
    }

    struct MovingEntity_RevolvableRandom : MovingEntity_Revolvable
    {
        explicit MovingEntity_RevolvableRandom()
        {
            Vector3 platf_offset = Vector3::random()*10.f;
            float yaw = G3D::uniformRandom(0.f, G3D::twoPi());
            float pitch = G3D::uniformRandom(0.f, G3D::twoPi());
            float roll = G3D::uniformRandom(0.f, G3D::twoPi());

            Position(platf_offset);
            YawAngle(yaw);
            PitchAngle(pitch);
            RollAngle(roll);
        }
    };

    TEST(MoveEnv, rotation)
    {
        int testCount = 2;
        while(testCount-- > 0)
        {
            MovingEntity_RevolvableRandom platform;
            MovingEntity_RevolvableRandom entity;
            entity.SetEnvironment(&platform);

            Vector3 computedGlobal = G3D::Matrix3::fromEulerAnglesZXY(platform.YawAngle(),
                platform.PitchAngle(),platform.RollAngle())*entity.Position() + platform.Position();
            Vector3 resultGlobal = entity.GlobalPosition();
            EXPECT_TRUE( resultGlobal.fuzzyEq(computedGlobal) );
        }
    }

    TEST(MoveEnv, global_returns_same)
    {
        int testCount = 2;
        while(testCount-- > 0)
        {
            MovingEntity_RevolvableRandom entity0;
            MovingEntity_RevolvableRandom entity1;
            MovingEntity_RevolvableRandom entity2;
            MovingEntity_RevolvableRandom obj;

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
        }
    }
}
