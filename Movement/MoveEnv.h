#pragma once

#include "G3D/Matrix3.h"
#include "typedefs_p.h"

namespace Movement
{
    /** Represents abstract environment */
    struct IMoveEnvironment
    {
        /** Converts local object position that environment contains into global position
            Takes 'outGlobal' parameter as local object position offset and converts it into global position */
        virtual void ComputeGlobalPosition(Vector3& outGlobal) = 0;
    };

    struct MovingEntity
    {
    public:
        IMoveEnvironment* Environment;
        Vector3 Position;

        explicit MovingEntity() : Environment(nullptr) {}

        Vector3 GlobalPosition() const
        {
            Vector3 global(Position);
            if (Environment != nullptr)
                Environment->ComputeGlobalPosition(global);
            return global;
        }
    };

    struct MovingEntity_Revolvable : IMoveEnvironment
    {
    private:
        MovingEntity m_entity;
        float m_Yaw;
        float m_Pitch;
        float m_Roll;

        G3D::Matrix3 m_rotation;
        bool m_matrixUpdated;

        const G3D::Matrix3& getTransform() {
            if (!m_matrixUpdated) {
                m_rotation = G3D::Matrix3::fromEulerAnglesZXY(m_Yaw, m_Pitch, m_Roll);
                m_matrixUpdated = true;
            }
            return m_rotation;
        }

        void ComputeGlobalPosition(Vector3& outGlobal) override {
            outGlobal = getTransform() * outGlobal + m_entity.Position;
            if (m_entity.Environment != nullptr)
                m_entity.Environment->ComputeGlobalPosition(outGlobal);
        }

    public:

        void Position(const Vector3& position) {
            m_entity.Position = position;
        }

        const Vector3& Position() const {
            return m_entity.Position;
        }

        Vector3 GlobalPosition() const {
            return m_entity.GlobalPosition();
        }

        void SetEnvironment(IMoveEnvironment* Env) {
            assert_state(Env != this);
            m_entity.Environment = Env;
        }

    public:

        explicit MovingEntity_Revolvable() :
            m_Yaw(0.f),
            m_Pitch(0.f),
            m_Roll(0.f),
            m_matrixUpdated(false),
            m_rotation(G3D::Matrix3::identity())
        {
        }

        float YawAngle() const {
            return m_Yaw;
        }

        void YawAngle(float value) {
            m_Yaw = value;
            m_matrixUpdated = false;
        }

        float PitchAngle() const {
            return m_Pitch;
        }

        void PitchAngle(float value) {
            m_Pitch = value;
            m_matrixUpdated = false;
        }

        float RollAngle() const {
            return m_Pitch;
        }

        void RollAngle(float value) {
            m_Roll = value;
            m_matrixUpdated = false;
        }

        void SetRotationFromTangentLine(const Vector3& tangent) {
            YawAngle(atan2f(tangent.y, tangent.x));
            PitchAngle( atan2f(tangent.z,tangent.xy().length()) );
            RollAngle(0.f);
        }
    };

    /*struct MovingEntity_Transport : MovingEntity, IMoveEnvironment
    {
    public:
        G3D::Matrix3 Rotation;

        MovingEntity_Transport() : MovingEntity(), Rotation(G3D::Matrix3::identity())
        {
        }

        void SetRotationFromTangentLine(const Vector3& tangent)
        {
            float yaw = atan2f(tangent.x, tangent.y);
            float pitch = atan2f(tangent.z, tangent.xy().length());
            Rotation = G3D::Matrix3::fromEulerAnglesXYZ(yaw, pitch, 0.f);
        }

        void ComputeGlobalPosition(Vector3& outGlobal) override {
            outGlobal = outGlobal * this->Rotation + this->GlobalPosition();
        }
    };*/

    struct MoveEnvironment_Ground : IMoveEnvironment
    {
        protected: void ComputeGlobalPosition(Vector3& outGlobal) override
        {
            // Nothing to do here: ground(continents and etc) has zero offset and has no rotation
        }
    };
}
