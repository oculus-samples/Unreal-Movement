/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <string>
#include <unordered_set>
#include "CoreMinimal.h"
#include "OculusXRLiveLinkRetargetBodyAsset.h"
#include "Animation/AnimNodeBase.h"
#include "UJsonDataAsset.h"
#include "OculusXRBoneName.h"
#include "OculusXRMovementUtility_DebugDraw.h"
#include "OculusXRMovement_UETypes.h"
#include "AnimNode_OculusXRBodyTracking.generated.h"

USTRUCT(Blueprintable)
struct OCULUSXRMOVEMENTUTILITY_API FAnimNode_OculusXRBodyTracking : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FPoseLink InputPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug, meta = (PinShownByDefault))
	EOculusXRBodyDebugPoseMode DebugPoseMode = EOculusXRBodyDebugPoseMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug, meta = (PinShownByDefault))
	EOculusXRBodyDebugDrawMode DebugDrawMode = EOculusXRBodyDebugDrawMode::None;
	/**
	 * Remapping from bone ID to target skeleton's bone name.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (DisplayName = "Known Joints Mappings"))
	TMap<EOculusXRBodyKnownJointType, FOculusXRBoneName> KnownJoints = {
		{ EOculusXRBodyKnownJointType::Root, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::Hips, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::RightUpperArm, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::LeftUpperArm, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::RightWrist, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::LeftWrist, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::Chest, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::Neck, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::RightUpperLeg, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::LeftUpperLeg, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::RightAnkle, FOculusXRBoneName(NAME_None) },
		{ EOculusXRBodyKnownJointType::LeftAnkle, FOculusXRBoneName(NAME_None) },
	};

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (DisplayName = "Auto Mapping Excluded Joints",
			ToolTip = "Add Joints to be ignored from the Auto Mapping process. They will move relative to their parent joint."))
	TSet<FOculusXRBoneName> AutoMappingExcludeJoints;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (DisplayName = "Auto Mapping No Twist Behavior",
			ToolTip = "Add Joints that should not be considered as twist joints to this list.  They may still be used for mapping, but will not have twist behavior applied."))
	TSet<FOculusXRBoneName> AutoMappingExcludeTwistBehaviorOnly;

	/**
	 * Switch between retargeting modes.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (PinShownByDefault,
			DisplayName = "Retargeting Mode",
			ToolTip = "Options define rotations only or roations w/positions modes.  Positions allow for deformations and scale, rotation only can either uniform scale or not scale at all."))
	EOculusXRBodyRetargetingMode RetargetingMode = EOculusXRBodyRetargetingMode::RotationAndPositions;

	/**
	 * Behavior for Root Motion - Combine to Root is more compatible with most Locomotion systems.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (PinShownByDefault,
			DisplayName = "Root Motion Behavior",
			ToolTip = "Defines behavior or the root node motion in relation to inclusion of rotation and offset from the tracking origin.  CombineToRoot is most common for locomotion."))
	EOculusXRBodyRetargetingRootMotionBehavior RootMotionBehavior = EOculusXRBodyRetargetingRootMotionBehavior::CombineToRoot;

	/**
	 * Tracking source JSON config file that is used to generate config data in editor or at runtime.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (AllowedClasses = "/Script/OculusXRMovementUtility.JsonDataAsset",
			DisplayName = "Tracking Source Config Asset",
			ToolTip = "Source/Input skeleton to map to.  This is typically the OVRSekeltonData_Unreal JSON data (MovementSDK SKeleton)"))
	TSoftObjectPtr<UJsonDataAsset> TrackingSourceConfigAsset =
		TSoftObjectPtr<UJsonDataAsset>(FSoftObjectPath(TEXT("/Game/Config/OVRSkeletonData_Unreal.OVRSkeletonData_Unreal")));

	/**
	 * Configuration text file that can be loaded at runtime.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "OculusXR|BodyTracking",
		meta = (AllowedClasses = "/Script/OculusXRMovementUtility.JsonDataAsset",
			DisplayName = "JSON Config Asset",
			ToolTip = "This is a JSON config asset that defines a mapping from source to target skeleton.  NOTE: If blank, the config will be generated at runtime."))
	TSoftObjectPtr<UJsonDataAsset> JsonConfigAsset;

	virtual ~FAnimNode_OculusXRBodyTracking();

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	virtual bool HasPreUpdate() const override { return true; };
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	metaMovementSDK_Handle GenerateRetargetingConfig(USkeleton* Skeleton) const;

	metaMovementSDK_Handle CreateTargetConfigFromSkeletonJoints(USkeleton* Skeleton) const;
	static TSet<FOculusXRBoneName> IdentifyPossibleExcludeJointsFromConfig(metaMovementSDK_Handle configHandle);

	static void MsdkTransformArrayFromXRBodySkeleton(const FOculusXRBodySkeleton& xrBodySkeleton, TArray<metaMovementSDK_Transform>& outTransformArray);
	static void MsdkTransformArrayFromXRBodyState(const FOculusXRBodyState& xrBodyState, TArray<metaMovementSDK_Transform>& outTransformArray);

private:
	inline bool IsDebugDrawEnabled() const
	{
		return OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW && DebugDrawMode != EOculusXRBodyDebugDrawMode::None;
	}

	static const FCompactPoseBoneIndex kInvalidBoneIndex;

	class LoadedConfigInfo
	{
	public:
		bool ReInitializeBoneContainerIfNecessary(const FBoneContainer& targetSkeletonBoneContainer);
		void Initialize(metaMovementSDK_Handle msdkHandle, const FBoneContainer& targetSkeletonBoneContainer);
		void Clear();

		inline bool IsValid() const
		{
			return IS_VALID_META_MOVEMENTSDK_HANDLE(RetargetingHandle);
		}

		inline metaMovementSDK_Handle GetHandle() const
		{
			return RetargetingHandle;
		}

		inline const FTransform& GetConfigToComponentSpaceTransform() const
		{
			return ConfigToComponentSpace;
		}

		inline const TArray<FCompactPoseBoneIndex>& GetTargetIndexArray() const
		{
			return TargetIndexArray;
		}

		inline const TSet<metaMovementSDK_JointIndex>& GetTargetJointsMissingFromCurrentSkeletonLOD()
		{
			return MSDKJointsMissingFromCurrentSkeletonLOD;
		}

		inline const TMap<int, FString>& GetSourceJointCountToManifestationTable() const
		{
			return SourceJointCountToManifestationTable;
		}

		inline TArray<metaMovementSDK_Transform>& GetTPoseTransformCache()
		{
			return SourceTPoseTransformCache;
		}

		inline TArray<metaMovementSDK_Transform>& GetTransformCache(metaMovementSDK_SkeletonType skeletonType)
		{
			return skeletonType == metaMovementSDK_SkeletonType::SourceSkeleton ? SourceTransformCache : TargetTransformCache;
		}

	private:
		void InitializeConfigToComponentSpace(const FReferenceSkeleton& RefSkeleton);
		void InitializeTargetIndexArray(const FBoneContainer& targetSkeletonBoneContainer);

		metaMovementSDK_Handle RetargetingHandle = META_MOVEMENTSDK_INVALID_HANDLE;
		TArray<FCompactPoseBoneIndex> TargetIndexArray;
		TSet<metaMovementSDK_JointIndex> MSDKJointsMissingFromCurrentSkeletonLOD;

		// Cache the source manifestation info here:
		TMap<int, FString> SourceJointCountToManifestationTable;

		// Cached information about the loaded config
		FTransform ConfigToComponentSpace = FTransform::Identity;
		TMap<EOculusXRBodyKnownJointType, metaMovementSDK_JointIndex> ConfigTargetKnownJointLookupTable;

		// These are arrays to simply hold allocations so we don't
		// re-allocate temp storage every frame.
		TArray<metaMovementSDK_Transform> SourceTPoseTransformCache;
		TArray<metaMovementSDK_Transform> SourceTransformCache;
		TArray<metaMovementSDK_Transform> TargetTransformCache;

		uint16 TargetBoneContainerSerialNumber = 0;
	};

	class CachedSkeletonInfo
	{
	public:
		void ResetSourceChangeCounter()
		{
			SourceChangeCount = 0;
		}

		inline bool UpdateFrameCounts(int FrameSourceChangeCount, int FrameJointCount)
		{
			bool retVal = false;
			if (FrameSourceChangeCount > 0)
			{
				if (FrameSourceChangeCount != SourceChangeCount)
				{
					SourceChangeCount = FrameSourceChangeCount;
					retVal = true;
				}
				if (FrameJointCount != SourceJointCount)
				{
					SourceJointCount = FrameJointCount;
					retVal = true;
				}
			}
			return retVal;
		}

		void Initialize();

	private:
		int SourceChangeCount = 0;
		int SourceJointCount = 0;
	};

	struct TAutoMappingJointData
	{
		std::unordered_set<std::string> jointNames;
		TArray<metaMovementSDK_AutoMappingJointData> additionalJointData;
	};

	void ConvertExcludeAndTwistJointsToANSISets(
		TAutoMappingJointData& autoMappingExcludeJointData,
		metaMovementSDK_Handle targetConfigHandle = META_MOVEMENTSDK_INVALID_HANDLE) const;

	static constexpr metaMovementSDK_JointRelativeSpaceType kMSDKJointSpaceOutputType =
		metaMovementSDK_JointRelativeSpaceType::RootOriginRelativeWithJointScale;

	bool ShouldUseFallbackPose() const;

	bool ProcessMovementSDKBodyPose();
	void UpdateRetargeting(FCompactPose& Pose);
	void UpdateDebugDraw();

	bool CreateRetargetingHandle(const FBoneContainer& targetSkeletonBoneContainer);

	static std::unordered_set<std::string> BoneNameSetToANSIStringSet(const TSet<FOculusXRBoneName>& BoneNameSet);

	// U Type Data - cached from other location
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	USkeleton* CachedUSkeleton = nullptr;

	LoadedConfigInfo ConfigInfo;
	CachedSkeletonInfo SkeletonInfo;

	// Need to use a shared pointer due to the Tickable not being copyable
	TSharedPtr<OculusXRMovementUtility_DebugDraw> DebugDraw;
	bool IsUserPlayer = false;
};
