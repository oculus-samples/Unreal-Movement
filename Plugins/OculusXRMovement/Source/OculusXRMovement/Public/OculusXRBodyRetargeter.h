/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "Animation/AnimNodeBase.h"
#include "OculusXRLiveLinkRetargetBodyAsset.h"
#include "OculusXRMovementTypes.h"

UENUM(BlueprintType, meta = (DisplayName = "Retargeting mode"))
enum class EOculusXRBodyRetargetingMode : uint8
{
	RotationAndPositions UMETA(DisplayName = "Rotation & Positions"),
	RotationAndPositionsHandsRotationOnly UMETA(DisplayName = "Rotation & Position - Hands Rotation Only"),
	RotationOnlyUniformScale UMETA(DisplayName = "Rotation Only - Uniform Scale"),
	RotationOnlyNoScaling UMETA(DisplayName = "Rotation Only - No Scaling"),
};

UENUM(BlueprintType, meta = (DisplayName = "Root Motion Behavior"))
enum class EOculusXRBodyRetargetingRootMotionBehavior : uint8
{
	CombineToRoot UMETA(DisplayName = "Combine Motion Into Root"),
	RootFlatTranslationHipRotation UMETA(DisplayName = "Root Translation with Full Hip Rotation"),
	ZeroOutRootTranslationHipYaw UMETA(DisplayName = "Zero Root Translation with Zero Hip Yaw"),
};

UENUM(BlueprintType, meta = (DisplayName = "DebugDraw mode"))
enum class EOculusXRBodyDebugDrawMode : uint8
{
	None UMETA(DisplayName = "None"),
	RestPose UMETA(DisplayName = "Rest Poses"),
	RestPoseWithMapping UMETA(DisplayName = "Rest Pose With Mapping"),
	FramePose UMETA(DisplayName = "Frame Pose"),
	FramePoseWithMapping UMETA(DisplayName = "Frame Pose With Mapping"),
};

UENUM(BlueprintType, meta = (DisplayName = "Pose Debug mode"))
enum class EOculusXRBodyDebugPoseMode : uint8
{
	None UMETA(DisplayName = "None"),
	RestPose UMETA(DisplayName = "Rest Pose"),
};

class OCULUSXRRETARGETING_API FOculusXRBodyRetargeter
{
public:
	virtual ~FOculusXRBodyRetargeter() = default;
	virtual void Initialize(const EOculusXRBodyRetargetingMode RetargetingMode,
		const EOculusXRBodyRetargetingRootMotionBehavior RootMotionBehavior,
		const EOculusXRAxis MeshForwardFacingDir,
		const TMap<EOculusXRBoneID, FName>* SourceToTargetNameMap) = 0;

	virtual bool RetargetFromBodyState(const FOculusXRBodyState& BodyState,
		const USkeletalMeshComponent* SkeletalMeshComponent,
		const float WorldScale,
		FPoseContext& Output) = 0;

	virtual EOculusXRBodyRetargetingMode GetRetargetingMode() = 0;
	virtual EOculusXRBodyRetargetingRootMotionBehavior GetRootMotionBehavior() = 0;

	virtual void SetDebugPoseMode(const EOculusXRBodyDebugPoseMode mode) = 0;
	virtual void SetDebugDrawMode(const EOculusXRBodyDebugDrawMode mode) = 0;

	static bool IsRotationAndPositionRetargetingMode(EOculusXRBodyRetargetingMode mode)
	{
		return mode == EOculusXRBodyRetargetingMode::RotationAndPositions || mode == EOculusXRBodyRetargetingMode::RotationAndPositionsHandsRotationOnly;
	}

	static bool IsRotationOnlyRetargetingMode(EOculusXRBodyRetargetingMode mode)
	{
		return mode == EOculusXRBodyRetargetingMode::RotationOnlyUniformScale || mode == EOculusXRBodyRetargetingMode::RotationOnlyNoScaling;
	}

	static bool IsModifiedRootBehavior(EOculusXRBodyRetargetingRootMotionBehavior behavior)
	{
		return behavior != EOculusXRBodyRetargetingRootMotionBehavior::RootFlatTranslationHipRotation;
	}

	static bool IsHipOrRootSourceJoint(EOculusXRBoneID boneID)
	{
		return boneID == EOculusXRBoneID::BodyRoot || boneID == EOculusXRBoneID::BodyHips;
	}
};
