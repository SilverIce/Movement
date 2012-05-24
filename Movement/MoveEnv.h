#pragma once

#include "framework/Component.h"
#include "framework/typedefs_p.h"
#include "framework/RdtscTimer.h"
#include "G3D/Matrix3.h"
#include "LinkedListSimple.h"

namespace Movement
{
    /** Represents abstract environment for objects that inside */
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
        Q_DISABLE_COPY(MovingEntity);
    private:
        IMoveEnvironment* m_Environment;
    public:

        /** Relative position */
        Vector3 Position;

        explicit MovingEntity() : m_Environment(nullptr) {}

        /** Attaches entity to environment.
            Null environment can be passed also. In this case entity will be attached to global coordinate system.
            Note: environment switch does not changes global position. */
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

        /** Current environment entity attached.
            In case environment is null - it means that entity attached to global coordinate system. */
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

    /** Holds rotation in Euler and 3x3 matrix format. */
    struct LazyRotation
    {
    private:
        float m_Yaw;
        float m_Pitch;
        float m_Roll;

        mutable G3D::Matrix3 m_rotation;
        mutable bool m_matrixUpdated;

    public:

        explicit LazyRotation() :
            m_Yaw(0.f),
            m_Pitch(0.f),
            m_Roll(0.f),
            m_rotation(G3D::Matrix3::identity()),
            m_matrixUpdated(false)
        {
        }

        const G3D::Matrix3& relativeRotation() const {
            if (!m_matrixUpdated) {
                m_rotation = G3D::Matrix3::fromEulerAnglesZXY(m_Yaw, m_Pitch, m_Roll);
                m_matrixUpdated = true;
            }
            return m_rotation;
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

        void EulerAngles(const Vector3& value) {
            m_Yaw = value.x;
            m_Pitch = value.y;
            m_Roll = value.z;
            m_matrixUpdated = false;
        }

        Vector3 EulerAngles() const {
            return Vector3(m_Yaw, m_Pitch, m_Roll);
        }

        void SetRotationFromTangentLine(const Vector3& tangent) {
            YawAngle(atan2f(tangent.y, tangent.x));
            PitchAngle( atan2f(tangent.z,tangent.xy().length()) );
            RollAngle(0.f);
        }
    };

    struct MovingEntity_Revolvable : IMoveEnvironment, LazyRotation
    {
    private:
        MovingEntity m_entity;

        using LazyRotation::relativeRotation;

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

        void RelativePosition(const Vector3& position) {
            m_entity.Position = position;
        }

        const Vector3& RelativePosition() const {
            return m_entity.Position;
        }

        Vector3 GlobalPosition() const {
            return m_entity.GlobalPosition();
        }

        void SetEnvironment(MovingEntity_Revolvable* Env) {
            assert_state(Env != this);
            m_entity.Environment(Env);
        }

        MovingEntity_Revolvable* Environment() const {
            return (MovingEntity_Revolvable*)m_entity.Environment();
        }

        explicit MovingEntity_Revolvable()
        {
        }

        ~MovingEntity_Revolvable() {
            assert_state(!Environment());
        }
    };

    struct MovingEntity_Revolvable2 : Component
    {
    private:
        MovingEntity_Revolvable m_entity;

        MovingEntity_Revolvable2 * m_Environment;
        mutable Vector3 m_globalPosition;
        mutable bool m_globalPositionOutdated;

        LinkedListSimple<MovingEntity_Revolvable2*> m_binded;
        LinkedListElementSimple<MovingEntity_Revolvable2*> m_me;

        void OnPositionChanged() {
            m_globalPositionOutdated = true;
            struct {
                void operator()(MovingEntity_Revolvable2* entity) { entity->OnPositionChanged();}
            } notifier;
            m_binded.Visit(notifier);
        }

        void OnRotationChanged() {
            struct {
                void operator()(MovingEntity_Revolvable2* entity) { entity->OnPositionChanged();}
            } notifier;
            m_binded.Visit(notifier);
        }

    public:

        void RelativePosition(const Vector3& position) {
            m_entity.RelativePosition(position);
            OnPositionChanged();
        }

        const Vector3& RelativePosition() const {
            return m_entity.RelativePosition();
        }

        const Vector3& GlobalPosition() const {
            if (m_globalPositionOutdated) {
                m_globalPosition = m_entity.GlobalPosition();
                m_globalPositionOutdated = false;
            }
            return m_globalPosition;
        }

        MovingEntity_Revolvable2* Environment() const {
            return m_Environment;
        }

        void SetEnvironment(MovingEntity_Revolvable2* env)
        {
            assert_state(env != this);
            if (m_Environment == env)
                return;
            if (m_Environment)
                m_Environment->m_binded.delink(m_me);
            if (env) {
                env->m_binded.link_first(m_me);
                m_entity.SetEnvironment(&env->m_entity);
            }
            else
                m_entity.SetEnvironment(nullptr);
            m_Environment = env;
        }

        typedef LinkedListSimple<MovingEntity_Revolvable2*> Entities;
        const Entities& BindedEntities() const {
            return m_binded;
        }

    public:

        explicit MovingEntity_Revolvable2() :
            m_Environment(nullptr),
            m_globalPositionOutdated(true)
        {
            m_me.Value = this;
        }

        ~MovingEntity_Revolvable2() {
            assert_state(!Environment());
            assert_state(BindedEntities().empty());
        }

        void YawAngle(float value) {
            m_entity.YawAngle(value);
            OnRotationChanged();
        }

        void PitchAngle(float value) {
            m_entity.PitchAngle(value);
            OnRotationChanged();
        }

        void RollAngle(float value) {
            m_entity.RollAngle(value);
            OnRotationChanged();
        }

        float YawAngle() const {
            return m_entity.YawAngle();
        }

        float PitchAngle() const {
            return m_entity.PitchAngle();
        }

        float RollAngle() const {
            return m_entity.RollAngle();
        }

        void SetRotationFromTangentLine(const Vector3& tangent) {
            m_entity.SetRotationFromTangentLine(tangent);
            OnRotationChanged();
        }
    };

    struct MovingEntity_Revolvable3 : Component
    {
    private:
        MovingEntity_Revolvable3 * m_Environment;
        Vector3 m_RelativePosition;
        LazyRotation m_rotation;

        LinkedListSimple<MovingEntity_Revolvable3*> m_binded;
        LinkedListElementSimple<MovingEntity_Revolvable3*> m_me;
        mutable bool m_globalRotationOutdated;
        mutable bool m_globalPositionOutdated;
        mutable Vector3 m_globalPosition;
        mutable G3D::Matrix3 m_globalRotation;

        /** Current entity position change outdates global position of all attached entities and
            current global position. */
        void OnPositionChanged() {
            if (Environment())
                m_globalPositionOutdated = true;
            struct {
                void operator()(MovingEntity_Revolvable3* entity) const {
                    entity->m_globalPositionOutdated = true;
                    entity->m_binded.Visit(*this);
                }
            } notifier;
            m_binded.Visit(notifier);
        }

        /** Current entity rotation change outdates global positions and rotations of all attached entities,
            but does not outdates current global position. */
        void OnRotationChanged() {
            if (Environment())
                m_globalRotationOutdated = true;
            struct {
                void operator()(MovingEntity_Revolvable3* entity) const {
                    entity->m_globalRotationOutdated = true;
                    entity->m_globalPositionOutdated = true;
                    entity->m_binded.Visit(*this);
                }
            } notifier;
            m_binded.Visit(notifier);
        }

        const G3D::Matrix3& GlobalRotation() const
        {
            if (!Environment())
                return m_rotation.relativeRotation();

            if (m_globalRotationOutdated) {
                m_globalRotationOutdated = false;
                m_globalRotation = Environment()->GlobalRotation() * m_rotation.relativeRotation();
            }
            return m_globalRotation;
        }

    public:

        void RelativePosition(const Vector3& position) {
            m_RelativePosition = position;
            OnPositionChanged();
        }

        const Vector3& RelativePosition() const {
            return m_RelativePosition;
        }

        const Vector3& GlobalPosition() const {
            if (!Environment())
                return RelativePosition();
            if (m_globalPositionOutdated) {
                m_globalPositionOutdated = false;
                m_globalPosition = Environment()->GlobalRotation() * RelativePosition() + Environment()->GlobalPosition();
            }
            return m_globalPosition;
        }

        MovingEntity_Revolvable3* Environment() const {
            return m_Environment;
        }

        void SetEnvironment(MovingEntity_Revolvable3* env)
        {
            assert_state(env != this);
            if (m_Environment == env)
                return;
            if (m_Environment)
                m_Environment->m_binded.delink(m_me);
            if (env) {
                env->m_binded.link_first(m_me);
                m_RelativePosition = (GlobalPosition() - env->GlobalPosition()) * env->GlobalRotation();
            }
            else
                m_RelativePosition = GlobalPosition();
            m_Environment = env;
        }

        typedef LinkedListSimple<MovingEntity_Revolvable3*> Entities;
        const Entities& BindedEntities() const {
            return m_binded;
        }

    public:

        explicit MovingEntity_Revolvable3() :
            m_Environment(nullptr),
            m_globalPositionOutdated(true),
            m_globalRotationOutdated(true)
        {
            m_me.Value = this;
        }

        ~MovingEntity_Revolvable3() {
            assert_state(!Environment());
            assert_state(BindedEntities().empty());
        }

        void YawAngle(float value) {
            m_rotation.YawAngle(value);
            OnRotationChanged();
        }

        void PitchAngle(float value) {
            m_rotation.PitchAngle(value);
            OnRotationChanged();
        }

        void RollAngle(float value) {
            m_rotation.RollAngle(value);
            OnRotationChanged();
        }

        void SetRotationFromTangentLine(const Vector3& tangent) {
            m_rotation.SetRotationFromTangentLine(tangent);
            OnRotationChanged();
        }

        float YawAngle() const {
            return m_rotation.YawAngle();
        }

        float PitchAngle() const {
            return m_rotation.PitchAngle();
        }

        float RollAngle() const {
            return m_rotation.RollAngle();
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
