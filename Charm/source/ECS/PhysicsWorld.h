#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace ECS
    {
        typedef void TaskCallback(int startIndex, int endIndex, uint32_t workerIndex, void* taskContext);
        typedef float FrictionCallback(float frictionA, int userMaterialIdA, float frictionB, int userMaterialIdB);
        typedef float RestitutionCallback(float restitutionA, int userMaterialIdA, float restitutionB, int userMaterialIdB);
        typedef void* EnqueueTaskCallback(TaskCallback* task, int itemCount, int minRange, void* taskContext, void* userContext);
        typedef void FinishTaskCallback(void* userTask, void* userContext);

        enum class ForceMode : u8
        {
            Force = 0,
            Impulse
        };

        enum class PhysicsBodyType : u8
        {
            Static = 0,
            Dynamic,
            Kinematic,
        };

        struct PhysicsWorldID
        {
            uint16_t index1;
            uint16_t generation;
        };

        struct PhysicsBodyID
        {
            int32_t index1;
            uint16_t world0;
            uint16_t generation;
        };

        struct PhysicsShapeID
        {
            int32_t index1;
            uint16_t world0;
            uint16_t generation;

            inline bool operator==(const PhysicsShapeID& other) { return index1 == other.index1 && world0 == other.world0 && generation == other.generation; }
            inline bool operator!=(const PhysicsShapeID& other) { return index1 != other.index1 || world0 != other.world0 || generation != other.generation; }
        };

        struct PhysicsWorld
        {
            /// Gravity vector. Box2D has no up-vector defined.
            struct Vec2
            {
                float x;
                float y;
            };
            Vec2 gravity;

            /// Restitution speed threshold, usually in m/s. Collisions above this
            /// speed have restitution applied (will bounce).
            float restitutionThreshold;

            /// Threshold speed for hit events. Usually meters per second.
            float hitEventThreshold;

            /// Contact stiffness. Cycles per second. Increasing this increases the speed of overlap recovery, but can introduce jitter.
            float contactHertz;

            /// Contact bounciness. Non-dimensional. You can speed up overlap recovery by decreasing this with
            /// the trade-off that overlap resolution becomes more energetic.
            float contactDampingRatio;

            /// This parameter controls how fast overlap is resolved and usually has units of meters per second. This only
            /// puts a cap on the resolution speed. The resolution speed is increased by increasing the hertz and/or
            /// decreasing the damping ratio.
            float contactSpeed;

            /// Maximum linear speed. Usually meters per second.
            float maximumLinearSpeed;

            /// Optional mixing callback for friction. The default uses sqrt(frictionA * frictionB).
            FrictionCallback* frictionCallback;

            /// Optional mixing callback for restitution. The default uses max(restitutionA, restitutionB).
            RestitutionCallback* restitutionCallback;

            /// Can bodies go to sleep to improve performance
            bool enableSleep;

            /// Enable continuous collision
            bool enableContinuous;

            /// Number of workers to use with the provided task system. Box2D performs best when using only
            /// performance cores and accessing a single L2 cache. Efficiency cores and hyper-threading provide
            /// little benefit and may even harm performance.
            /// @note Box2D does not create threads. This is the number of threads your applications has created
            /// that you are allocating to b2World_Step.
            /// @warning Do not modify the default value unless you are also providing a task system and providing
            /// task callbacks (enqueueTask and finishTask).
            int workerCount;

            /// Function to spawn tasks
            EnqueueTaskCallback* enqueueTask;

            /// Function to finish a task
            FinishTaskCallback* finishTask;

            /// User context that is provided to enqueueTask and finishTask
            void* userTaskContext;

            /// User data
            void* userData;

            /// Used internally to detect a valid definition. DO NOT SET.
            int internalValue;
        };

        inline const PhysicsWorldID Physics_NullWorldID = {};
    }
}
