/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once
#include "MetaMovementSDK_Types.h"

#define OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW (!UE_BUILD_SHIPPING)

#include "Tickable.h"

class OculusXRMovementUtility_DebugDraw
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	: public FTickableGameObject
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
{
public:
	static const float kLineDrawThickness;
	static const float kAxisLineLength;
	static const FString kDefaultDrawQueue;

	struct LineSegment
	{
		FVector start;
		FVector end;
		FColor color;
	};

	struct Axis
	{
		FVector position;
		FQuat rotation;
	};

	struct DrawQueue
	{
		TArray<LineSegment> lines;
		TArray<Axis> axis;
	};

	OculusXRMovementUtility_DebugDraw() {}
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	virtual ~OculusXRMovementUtility_DebugDraw() override {}
#else
	~OculusXRMovementUtility_DebugDraw() = default;
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	void AddSkeleton(
		metaMovementSDK_Handle retargetingHandle,
		metaMovementSDK_SkeletonType skeletonType,
		const TArray<metaMovementSDK_Transform>& msdkTransforms,
		const TSet<metaMovementSDK_JointIndex>& skipRenderJointSet,
		const FTransform& componentAndConfigTransformToApply,
		const FColor& color,
		const bool bRenderAxis = true,
		const FString& drawQueue = kDefaultDrawQueue);

	void AddSkeletonMappings(
		metaMovementSDK_Handle retargetingHandle,
		const TArray<metaMovementSDK_Transform>& SourceTransforms,
		const TArray<metaMovementSDK_Transform>& TargetTransforms,
		const FTransform& componentAndConfigTransformToApply,
		const FColor& color,
		bool useTPose = false,
		const bool bRenderAxis = true,
		const FString& drawQueue = kDefaultDrawQueue);

	void AddLineSegment(const FVector& start, const FVector& end, const FColor& color, const FString& drawQueue = kDefaultDrawQueue);
	void AddAxis(const FVector& position, const FQuat& rotation, const FString& drawQueue = kDefaultDrawQueue);
	void AddAxis(const FTransform& transform, const FString& drawQueue = kDefaultDrawQueue);
	void ClearDrawQueue(const FString& drawQueue = kDefaultDrawQueue);

	virtual void Tick(float DeltaTime) override
	{
		DrawFrame();
		ClearDrawQueue(); // Clear the Default Queue
	}

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(DebugDrawUtility, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const override
	{
		return true;
	}
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}

private:
	void DrawFrame();

	mutable FCriticalSection MultiThreadLock;
	TMap<FString, DrawQueue> DrawQueues_;

#else
	// No Ops if disabled
	static void AddSkeleton(
		metaMovementSDK_Handle retargetingHandle,
		metaMovementSDK_SkeletonType skeletonType,
		const TArray<metaMovementSDK_Transform>& msdkTransforms,
		const FTransform& componentAndConfigTransformToApply,
		const FColor& color,
		const bool bRenderAxis = true,
		const FString& drawQueue = kDefaultDrawQueue) {}

	void AddSkeletonMappings(
		metaMovementSDK_Handle retargetingHandle,
		const FTransform& componentAndConfigTransformToApply,
		const FColor& color,
		bool useTPose = false,
		const FString& drawQueue = kDefaultDrawQueue) {}

	static void AddLineSegment(const FVector& start, const FVector& end, const FColor& color, const FString& drawQueue = kDefaultDrawQueue) {}
	static void AddAxis(const FVector& position, const FQuat& rotation, const FString& drawQueue = kDefaultDrawQueue) {}
	static void AddAxis(const FTransform& transform, const FString& drawQueue = kDefaultDrawQueue) {}
	static void ClearDrawQueue(const FString& drawQueue = kDefaultDrawQueue) {}
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
};
