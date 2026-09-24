/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRMovementUtility_DebugDraw.h"

#include "MetaMovementSDK_UEUtility.h"
#include "OculusXRMovementUtility.h"

const float OculusXRMovementUtility_DebugDraw::kLineDrawThickness = 0.2f;
const float OculusXRMovementUtility_DebugDraw::kAxisLineLength = 2.0f;
const FString OculusXRMovementUtility_DebugDraw::kDefaultDrawQueue = "default";

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW

// Handle and Skeleton Type are used to get the hierarchy.
// msdkTransforms is assumed to be the same joint count as the source/target
// skeleton provided by the retargetingHandle
void OculusXRMovementUtility_DebugDraw::AddSkeleton(
	metaMovementSDK_Handle retargetingHandle,
	metaMovementSDK_SkeletonType skeletonType,
	const TArray<metaMovementSDK_Transform>& msdkTransforms,
	const TSet<metaMovementSDK_JointIndex>& skipRenderJointSet,
	const FTransform& componentAndConfigTransformToApply,
	const FColor& color,
	const bool bRenderAxis,
	const FString& drawQueue)
{
	// Do NOT FScopeLock this function - the arrays will be protected in the calls to AddLineSegment and AddAxis

	// Step 1: Get the skeleton Information and validate the handle/array size
	metaMovementSDK_SkeletonInfo skeletonInfo;
	if (!MetaMovementSDK_UEUtility::GetSkeletonInfo(retargetingHandle, skeletonType, skeletonInfo))
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("OculusXRMovementUtility_DebugDraw could not render skeleton - could not retrieve skeleton info from Handle."));
		return;
	}

	if (skeletonInfo.jointCount != msdkTransforms.Num())
	{
		UE_LOG(
			LogOculusXRMovementUtility,
			Error,
			TEXT("OculusXRMovementUtility_DebugDraw could not render skeleton - Joint Count mismatch.  Expected: %d, Transform Array Size: %d"),
			skeletonInfo.jointCount,
			msdkTransforms.Num());
		return;
	}

	// Step 2: Retrieve the parent joint index data
	TArray<metaMovementSDK_JointIndex> parentJointIndices;
	parentJointIndices.SetNum(skeletonInfo.jointCount);
	if (!MetaMovementSDK_UEUtility::GetParentJointIndexes(retargetingHandle, skeletonType, parentJointIndices))
	{
		UE_LOG(
			LogOculusXRMovementUtility,
			Error,
			TEXT("OculusXRMovementUtility_DebugDraw could not render skeleton - Cannot retrieve parent joint index data"));
		return;
	}

	// Step 3: Iterate the joints and add line segments/axis for the joints
	for (metaMovementSDK_JointIndex i = 0; i < skeletonInfo.jointCount; ++i)
	{
		if (!skipRenderJointSet.Contains(i))
		{
			// Calculate the joint's transform
			const FTransform jointTransform = MetaMovementSDK_UEUtility::ToFTransform(msdkTransforms[i]) * componentAndConfigTransformToApply;
			const metaMovementSDK_JointIndex parentJointIndex = parentJointIndices[i];
			if (IS_VALID_META_MOVEMENTSDK_JOINT(parentJointIndex))
			{
				// Add the line segment
				const FTransform parentJointTransform = MetaMovementSDK_UEUtility::ToFTransform(msdkTransforms[parentJointIndex]) * componentAndConfigTransformToApply;
				AddLineSegment(parentJointTransform.GetLocation(), jointTransform.GetLocation(), color, drawQueue);
			}

			if (bRenderAxis)
			{
				// Add the Axis
				AddAxis(jointTransform.GetLocation(), jointTransform.GetRotation(), drawQueue);
			}
		}
	}
}

void OculusXRMovementUtility_DebugDraw::AddSkeletonMappings(
	metaMovementSDK_Handle retargetingHandle,
	const TArray<metaMovementSDK_Transform>& SourceTransforms,
	const TArray<metaMovementSDK_Transform>& TargetTransforms,
	const FTransform& componentAndConfigTransformToApply,
	const FColor& color,
	bool useTPose,
	const bool bRenderAxis,
	const FString& drawQueue)
{
	// Sanity check with the transform arrays
	metaMovementSDK_SkeletonInfo sourceSkeletonInfo;
	if (!MetaMovementSDK_UEUtility::GetSkeletonInfo(retargetingHandle, metaMovementSDK_SkeletonType::SourceSkeleton, sourceSkeletonInfo) || sourceSkeletonInfo.jointCount != SourceTransforms.Num())
	{
		return;
	}

	metaMovementSDK_SkeletonInfo targetSkeletonInfo;
	if (!MetaMovementSDK_UEUtility::GetSkeletonInfo(retargetingHandle, metaMovementSDK_SkeletonType::TargetSkeleton, targetSkeletonInfo) || targetSkeletonInfo.jointCount != TargetTransforms.Num())
	{
		return;
	}

	TArray<metaMovementSDK_JointIndex> mappedTargetJoints;
	TArray<metaMovementSDK_JointIndex> sourceJointIndexes;
	metaMovementSDK_SkeletonType sourceSkeletonType = metaMovementSDK_SkeletonType::SourceSkeleton;
	metaMovementSDK_Transform tPoseBlendedTransform = META_MOVEMENTSDK_IDENTITY_TRANSFORM;
	metaMovementSDK_Transform frameBlendedTransform = META_MOVEMENTSDK_IDENTITY_TRANSFORM;
	if (MetaMovementSDK_UEUtility::GetSkeletonMappingTargetJoints(retargetingHandle, mappedTargetJoints))
	{
		for (const metaMovementSDK_JointIndex targetJointIndex : mappedTargetJoints)
		{
			if (MetaMovementSDK_UEUtility::GetLastRetargetedMappingData(
					retargetingHandle,
					targetJointIndex,
					sourceSkeletonType,
					tPoseBlendedTransform,
					frameBlendedTransform,
					sourceJointIndexes)
				&& sourceSkeletonType == metaMovementSDK_SkeletonType::SourceSkeleton)
			{
				// Only Draw Source to Target Mappings
				const FTransform blendedTransform = MetaMovementSDK_UEUtility::ToFTransform(useTPose ? tPoseBlendedTransform : frameBlendedTransform) * componentAndConfigTransformToApply;
				const FTransform targetTransform = MetaMovementSDK_UEUtility::ToFTransform(TargetTransforms[targetJointIndex]) * componentAndConfigTransformToApply;
				// Just draw a line from the source to the target (source will be the blended transform)
				AddLineSegment(blendedTransform.GetLocation(), targetTransform.GetLocation(), color, drawQueue);
				if (sourceJointIndexes.Num() > 1)
				{
					for (const metaMovementSDK_JointIndex sourceJointIndex : sourceJointIndexes)
					{
						check(sourceJointIndex < SourceTransforms.Num());
						const FTransform sourceTransform = MetaMovementSDK_UEUtility::ToFTransform(SourceTransforms[sourceJointIndex]) * componentAndConfigTransformToApply;
						AddLineSegment(blendedTransform.GetLocation(), sourceTransform.GetLocation(), color, drawQueue);
					}
				}
				if (bRenderAxis)
				{
					AddAxis(blendedTransform.GetLocation(), blendedTransform.GetRotation(), drawQueue);
				}
			}
		}
	}
}

void OculusXRMovementUtility_DebugDraw::AddLineSegment(const FVector& start, const FVector& end, const FColor& color, const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	DrawQueues_.FindOrAdd(drawQueue).lines.Add(LineSegment{ start, end, color });
}

void OculusXRMovementUtility_DebugDraw::AddAxis(const FVector& position, const FQuat& rotation, const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	DrawQueues_.FindOrAdd(drawQueue).axis.Add(Axis{ position, rotation });
}

void OculusXRMovementUtility_DebugDraw::AddAxis(const FTransform& transform, const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	DrawQueues_.FindOrAdd(drawQueue).axis.Add(Axis{ transform.GetLocation(), transform.GetRotation() });
}

void OculusXRMovementUtility_DebugDraw::DrawFrame()
{
	const UWorld* World = GWorld;
#if WITH_EDITOR
	if (World && GIsEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				World = Context.World();
				break;
			}
		}
	}
#endif // WITH_EDITOR
	if (!World)
	{
		return;
	}

	FScopeLock Lock(&MultiThreadLock);
	for (auto& queuePair : DrawQueues_)
	{
		for (LineSegment segment : queuePair.Value.lines)
		{
			DrawDebugLine(World, segment.start, segment.end, segment.color, false, -1, 1, kLineDrawThickness);
		}

		for (Axis axis : queuePair.Value.axis)
		{
			DrawDebugLine(World, axis.position, axis.position + (axis.rotation.GetAxisX() * kAxisLineLength), FColor::Red, false, -1, 1, kLineDrawThickness);
			DrawDebugLine(World, axis.position, axis.position + (axis.rotation.GetAxisY() * kAxisLineLength), FColor::Green, false, -1, 1, kLineDrawThickness);
			DrawDebugLine(World, axis.position, axis.position + (axis.rotation.GetAxisZ() * kAxisLineLength), FColor::Blue, false, -1, 1, kLineDrawThickness);
		}
	}
}

void OculusXRMovementUtility_DebugDraw::ClearDrawQueue(const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	DrawQueues_.Remove(drawQueue);
}

#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
