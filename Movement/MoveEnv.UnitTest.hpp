#include "MoveEnv.h"

namespace Movement
{
    TEST(MoveEnv, basic)
    {
        MovingEntity_Revolvable entity;
        EXPECT_TRUE( entity.YawAngle() == 0.f );
        EXPECT_TRUE( entity.RollAngle() == 0.f );
        EXPECT_TRUE( entity.PitchAngle() == 0.f );
        EXPECT_TRUE( entity.Position().isZero() );
    }

    TEST(MoveEnv, rotation)
    {
        int testCount = 10;
        while(testCount-- > 0)
        {
            Vector3 platf_offset = Vector3::random();
            Vector3 entity_offset = Vector3::random();
            float yaw = G3D::uniformRandom(0.f, G3D::twoPi());
            float pitch = G3D::uniformRandom(0.f, G3D::twoPi());
            float roll = G3D::uniformRandom(0.f, G3D::twoPi());

            MovingEntity_Revolvable platform;
            platform.Position(platf_offset);
            platform.YawAngle(yaw);
            platform.PitchAngle(pitch);
            platform.RollAngle(roll);

            MovingEntity_Revolvable entity;
            entity.Position(entity_offset);
            entity.SetEnvironment(&platform);

            EXPECT_TRUE( entity.GlobalPosition().fuzzyEq
                (G3D::Matrix3::fromEulerAnglesZXY(yaw,pitch,roll)*entity_offset + platf_offset)
            );
        }
    }
}
