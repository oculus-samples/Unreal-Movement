/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <vector>
#include "MetaMovementSDK_Utility.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMetaMovementSDK_Utility, Log, All);

// Static class implementation
class METAMOVEMENTSDK_UTILITY_API FMetaMovementSDK_UtilityModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void* DllHandle = nullptr;
};

class METAMOVEMENTSDK_UTILITY_API MetaMovementSDK_UEUtility
{
public:
	// FString Wrapper Functions
	static bool GetConfigName(metaMovementSDK_Handle handle, FString& outString);

	static bool GetJointNames(metaMovementSDK_Handle handle, metaMovementSDK_SkeletonType skeletonType, TArray<FString>& outJointNames);
	static bool GetJointName(metaMovementSDK_Handle handle, metaMovementSDK_SkeletonType skeletonType, metaMovementSDK_JointIndex jointIndex, FString& outName);

	static bool GetManifestationNames(
		metaMovementSDK_Handle handle,
		metaMovementSDK_SkeletonType skeletonType,
		TArray<FString>& outManifestationNames);

	static bool WriteConfigDataToJSON(
		metaMovementSDK_Handle handle,
		FString& outJSONData,
		metaMovementSDK_JointRelativeSpaceType JointRelativeSpace = metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeSpace);

	// Helper Wrapper Functions
	static bool GetSkeletonInfo(metaMovementSDK_Handle handle, metaMovementSDK_SkeletonType skeletonType, metaMovementSDK_SkeletonInfo& out_skeletonInfo);
	static bool GetSkeletonTPose(
		metaMovementSDK_Handle handle,
		metaMovementSDK_SkeletonType skeletonType,
		metaMovementSDK_SkeletonTPoseType tPoseType,
		TArray<metaMovementSDK_Transform>& outTransformData,
		metaMovementSDK_JointRelativeSpaceType jointSpaceType = metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeSpace);

	static bool GetLastProcessedFramePose(
		metaMovementSDK_Handle handle,
		metaMovementSDK_SkeletonType skeletonType,
		TArray<metaMovementSDK_Transform>& outTransformData,
		metaMovementSDK_JointRelativeSpaceType out_jointSpaceType = metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeSpace);

	static bool GetParentJointIndexes(
		metaMovementSDK_Handle handle,
		metaMovementSDK_SkeletonType skeletonType,
		TArray<metaMovementSDK_JointIndex>& outParentJointIndices);

	static bool GetJointIndexesInHumanoidLimb(
		metaMovementSDK_Handle handle,
		metaMovementSDK_SkeletonType skeletonType,
		metaMovementSDK_HumanoidLimbType humanoidLimbType,
		TArray<metaMovementSDK_JointIndex>& outJointIndicesInHumanoidLimb);

	static bool GetSkeletonMappingTargetJoints(
		metaMovementSDK_Handle handle,
		TArray<metaMovementSDK_JointIndex>& outTargetJointIndices);

	static bool GetLastRetargetedMappingData(
		metaMovementSDK_Handle handle,
		metaMovementSDK_JointIndex targetJointIndex,
		metaMovementSDK_SkeletonType& outSourceSkeletonType,
		metaMovementSDK_Transform& outTPoseBlendedTransform,
		metaMovementSDK_Transform& outLastPoseBlendedTransform,
		TArray<metaMovementSDK_JointIndex> outSourceJointIndexList);

	// Conversion Functions
	static inline metaMovementSDK_Vector3f ToMsdkVector3f(const FVector3f& InVector)
	{
		return { InVector.X, InVector.Y, InVector.Z };
	}

	static inline metaMovementSDK_Vector3f ToMsdkVector3f(const FVector& InVector)
	{
		return ToMsdkVector3f(static_cast<FVector3f>(InVector));
	}

	static inline metaMovementSDK_Quatf ToMsdkQuatf(const FQuat4f& InQuat)
	{
		return { InQuat.X, InQuat.Y, InQuat.Z, InQuat.W };
	}

	static inline metaMovementSDK_Quatf ToMsdkQuatf(const FQuat& InQuat)
	{
		return ToMsdkQuatf(static_cast<FQuat4f>(InQuat));
	}

	static inline FVector ToFVector(const metaMovementSDK_Vector3f& InVector)
	{
		return { InVector.x, InVector.y, InVector.z };
	}

	static inline FQuat ToFQuat(const metaMovementSDK_Quatf& InQuat)
	{
		return { InQuat.x, InQuat.y, InQuat.z, InQuat.w };
	}

	static inline metaMovementSDK_Transform ToMsdkTransform(const FQuat4f& InOrientation, const FVector3f& InPosition, const FVector3f& InScale)
	{
		return { ToMsdkQuatf(InOrientation), ToMsdkVector3f(InPosition), ToMsdkVector3f(InScale) };
	}

	static inline metaMovementSDK_Transform ToMsdkTransform(const FQuat& InOrientation, const FVector& InPosition, const FVector& InScale)
	{
		return { ToMsdkQuatf(InOrientation), ToMsdkVector3f(InPosition), ToMsdkVector3f(InScale) };
	}

	static inline metaMovementSDK_Transform ToMsdkTransform(const FTransform& InTransform)
	{
		return { ToMsdkQuatf(InTransform.GetRotation()), ToMsdkVector3f(InTransform.GetLocation()), ToMsdkVector3f(InTransform.GetScale3D()) };
	}

	static inline FTransform ToFTransform(const metaMovementSDK_Quatf& InOrientation, const metaMovementSDK_Vector3f& InPosition, const metaMovementSDK_Vector3f& InScale)
	{
		return FTransform(ToFQuat(InOrientation), ToFVector(InPosition), ToFVector(InScale));
	}
	static inline FTransform ToFTransform(const metaMovementSDK_Transform& InTransform)
	{
		return FTransform(ToFQuat(InTransform.orientation), ToFVector(InTransform.position), ToFVector(InTransform.scale));
	}

	// Pose Conversions
	static inline TArray<FTransform> FTransformArray(const TArray<metaMovementSDK_Transform>& msdkTransforArray)
	{
		TArray<FTransform> retVal;
		retVal.Reserve(msdkTransforArray.Num());
		for (const metaMovementSDK_Transform& msdkTransform : msdkTransforArray)
		{
			retVal.Emplace(ToFTransform(msdkTransform));
		}
		return retVal;
	}

	static inline TArray<metaMovementSDK_Transform> ToMsdkTransformArray(const TArray<FTransform>& transformArray)
	{
		TArray<metaMovementSDK_Transform> retVal;
		retVal.Reserve(transformArray.Num());
		for (const FTransform& transform : transformArray)
		{
			retVal.Emplace(ToMsdkTransform(transform));
		}
		return retVal;
	}

	// Utility Functions
	// Helper function to get the world to meters scale.

	// Can be called from any thread.
	static inline float GetWorldToMetersScale()
	{
		if (GWorld != nullptr && GWorld->GetWorldSettings() != nullptr)
		{
#if WITH_EDITOR
			// Workaround to allow WorldToMeters scaling to work correctly for controllers while running inside PIE.
			// The main world will most likely not be pointing at the PIE world while polling input, so if we find a world context
			// of that type, use that world's WorldToMeters instead.
			if (GIsEditor)
			{
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (Context.WorldType == EWorldType::PIE)
					{
						return Context.World()->GetWorldSettings()->WorldToMeters;
					}
				}
			}
#endif // WITH_EDITOR

			// We're not currently rendering a frame, so just use whatever world to meters the main world is using.
			// This can happen when we're polling input in the main engine loop, before ticking any worlds.
			return GWorld->GetWorldSettings()->WorldToMeters;
		}
		return 100.0f;
	}

	static inline float GetMetersToWorldScale()
	{
		const float worldToMetersScale = GetWorldToMetersScale();
		// Default to 1/100.0f if the world to meters scale is invalid.
		return 1.0f / (worldToMetersScale > 0.0f ? worldToMetersScale : 100.0f);
	}

	// TODO: Potentially handle the case where GetWorldToMetersScale() changes at runtime.
	// Written as a function in case we need dynamic behavior of the WorldToMetersScale
	static inline const metaMovementSDK_CoordinateSpace& GetEngineCoordinateSpace()
	{
		static const metaMovementSDK_CoordinateSpace kEngineCoordinateSpace = {
			{ 0.0f, 0.0f, 1.0f }, // Z-up
			{ 1.0f, 0.0f, 0.0f }, // X-forward
			{ 0.0f, 1.0f, 0.0f }, // Y-right
			// Ensure to use Meters to World Scale.
			// A centimeter as unit space is 0.01 meters to the world unit.
			GetMetersToWorldScale() // Typically 1/100.0f (Centimeters)
		};

		return kEngineCoordinateSpace;
	}
};
