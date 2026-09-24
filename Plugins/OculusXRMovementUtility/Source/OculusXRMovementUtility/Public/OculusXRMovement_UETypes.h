/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "MetaMovementSDK_Types.h"

// Maps to metaMovementSDK_KnownJointType
UENUM(BlueprintType, meta = (DisplayName = "Known Joint Type"))
enum class EOculusXRBodyKnownJointType : uint8
{
	// Unknown is not included due to the uint8 enum type requirement
	Root = metaMovementSDK_KnownJointType::Root UMETA(DisplayName = "Root"),
	Hips = metaMovementSDK_KnownJointType::Hips UMETA(DisplayName = "Hips"),
	RightUpperArm = metaMovementSDK_KnownJointType::RightUpperArm UMETA(DisplayName = "Right Upper Arm"),
	LeftUpperArm = metaMovementSDK_KnownJointType::LeftUpperArm UMETA(DisplayName = "Left Upper Arm"),
	RightWrist = metaMovementSDK_KnownJointType::RightWrist UMETA(DisplayName = "Right Wrist"),
	LeftWrist = metaMovementSDK_KnownJointType::LeftWrist UMETA(DisplayName = "Left Wrist"),
	Chest = metaMovementSDK_KnownJointType::Chest UMETA(DisplayName = "Chest"),
	Neck = metaMovementSDK_KnownJointType::Neck UMETA(DisplayName = "Neck"),
	RightUpperLeg = metaMovementSDK_KnownJointType::RightUpperLeg UMETA(DisplayName = "Right Upper Leg"),
	LeftUpperLeg = metaMovementSDK_KnownJointType::LeftUpperLeg UMETA(DisplayName = "Left Upper Leg"),
	RightAnkle = metaMovementSDK_KnownJointType::RightAnkle UMETA(DisplayName = "Right Ankle"),
	LeftAnkle = metaMovementSDK_KnownJointType::LeftAnkle UMETA(DisplayName = "Left Ankle"),
};

// Maps to metaMovementSDK_RetargetingBehavior
UENUM(BlueprintType, meta = (DisplayName = "Retargeting mode"))
enum class EOculusXRBodyRetargetingMode : uint8
{
	RotationAndPositions = metaMovementSDK_RetargetingBehavior::RotationsAndPositions UMETA(DisplayName = "Rotation & Positions"),
	RotationAndPositionsHandsRotationOnly = metaMovementSDK_RetargetingBehavior::RotationsAndPositionsHandsRotationOnly UMETA(DisplayName = "Rotation & Position - Hands Rotation Only"),
	RotationOnlyUniformScale = metaMovementSDK_RetargetingBehavior::RotationOnlyUniformScale UMETA(DisplayName = "Rotation Only - Uniform Scale"),
	RotationOnlyNoScaling = metaMovementSDK_RetargetingBehavior::RotationOnlyNoScaling UMETA(DisplayName = "Rotation Only - No Scaling"),
};

// Maps to metaMovementSDK_RootMotionBehavior
UENUM(BlueprintType, meta = (DisplayName = "Root Motion Behavior"))
enum class EOculusXRBodyRetargetingRootMotionBehavior : uint8
{
	CombineToRoot = metaMovementSDK_RetargetingRootMotionBehavior::CombineHipRotationIntoRoot UMETA(DisplayName = "Combine Motion Into Root"),
	RootFlatTranslationHipRotation = metaMovementSDK_RetargetingRootMotionBehavior::RootFlatTranslationFullHipRotation UMETA(DisplayName = "Root Translation with Full Hip Rotation"),
	ZeroOutRootTranslationHipYaw = metaMovementSDK_RetargetingRootMotionBehavior::ZeroOutAllRootTranslationAndHipYaw UMETA(DisplayName = "Zero Root Translation with Zero Hip Yaw"),
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
