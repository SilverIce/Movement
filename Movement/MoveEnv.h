#pragma once

#include "G3D/Matrix3.h"
#include "typedefs_p.h"

namespace Movement
{
    /** Represents abstract environment */
    struct IMoveEnvironment
    {
        /** Converts relative object position that environment contains into global position
            Takes 'outGlobal' parameter as local object position offset and converts it into global position */
        virtual void ComputeGlobalPosition(Vector3& outGlobal) = 0;

        /** Converts global into relative object position */
        virtual void ComputeLocalPosition(Vector3& outLocal) = 0;
    };

    struct MovingEntity
    {
    private:
        IMoveEnvironment* m_Environment;
    public:
        Vector3 Position;

        explicit MovingEntity() : m_Environment(nullptr) {}

        void Environment(IMoveEnvironment* Env)
        {
            if (Env == m_Environment)
                return;

            Position = GlobalPosition();    // "move" our entity into global coord system
            if (Env == nullptr)
                ;   // special case: null environment recognized as global coordinate system, nothing to do
            else
                Env->ComputeLocalPosition(Position);    // convert global into relative position
            m_Environment = Env;
        }

        IMoveEnvironment* Environment() const {
            return m_Environment;
        }

        Vector3 GlobalPosition() const
        {
            Vector3 global(Position);
            if (m_Environment != nullptr)
                m_Environment->ComputeGlobalPosition(global);
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

        const G3D::Matrix3& relativeRotation() {
            if (!m_matrixUpdated) {
                m_rotation = G3D::Matrix3::fromEulerAnglesZXY(m_Yaw, m_Pitch, m_Roll);
                m_matrixUpdated = true;
            }
            return m_rotation;
        }

        void ComputeGlobalPosition(Vector3& outGlobal) override {
            outGlobal = relativeRotation() * outGlobal + m_entity.Position;

            if (m_entity.Environment() != nullptr)
                m_entity.Environment()->ComputeGlobalPosition(outGlobal);
        }

        void ComputeGlobalRotation(G3D::Matrix3& rotation) {
            if (MovingEntity_Revolvable * env = Environment()) {
                env->ComputeGlobalRotation(rotation);
                rotation *= relativeRotation();
            }
            else
                rotation = relativeRotation();
        }

        void ComputeLocalPosition(Vector3& outLocal) override {
            G3D::Matrix3 globalRotation;
            ComputeGlobalRotation(globalRotation);
            outLocal = (outLocal - GlobalPosition()) * globalRotation;
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

        void SetEnvironment(MovingEntity_Revolvable* Env) {
            assert_state(Env != this);
            m_entity.Environment(Env);
        }

    private:
        MovingEntity_Revolvable* Environment() const {
            return (MovingEntity_Revolvable*)m_entity.Environment();
        }

    public:

        explicit MovingEntity_Revolvable() :
            m_Yaw(0.f),
            m_Pitch(0.f),
            m_Roll(0.f),
            m_rotation(G3D::Matrix3::identity()),
            m_matrixUpdated(false)
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
            return m_Roll;
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
