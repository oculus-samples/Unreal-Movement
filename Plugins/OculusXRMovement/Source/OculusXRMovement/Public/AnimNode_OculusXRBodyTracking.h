/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "OculusXRLiveLinkRetargetBodyAsset.h"
#include "OculusXRBodyRetargeter.h"
#include "OculusXRRetargetSkeleton.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_OculusXRBodyTracking.generated.h"

USTRUCT(Blueprintable)
struct OCULUSXRRETARGETING_API FAnimNode_OculusXRBodyTracking : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FPoseLink InputPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug, meta = (PinShownByDefault))
	EOculusXRBodyDebugPoseMode DebugPoseMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug, meta = (PinShownByDefault))
	EOculusXRBodyDebugDrawMode DebugDrawMode;
	/**
	 * Remapping from bone ID to target skeleton's bone name.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|BodyTracking")
	TMap<EOculusXRBoneID, FName> BoneRemapping = {
		{ EOculusXRBoneID::BodyRoot, "root" },
		{ EOculusXRBoneID::BodyHips, "pelvis" },
		{ EOculusXRBoneID::BodySpineLower, "spine_01" },
		{ EOculusXRBoneID::BodySpineMiddle, "spine_02" },
		{ EOculusXRBoneID::BodySpineUpper, "spine_04" },
		{ EOculusXRBoneID::BodyChest, "spine_05" },
		{ EOculusXRBoneID::BodyNeck, "neck_02" },
		{ EOculusXRBoneID::BodyHead, "head" },

		{ EOculusXRBoneID::BodyLeftShoulder, "clavicle_l" },
		{ EOculusXRBoneID::BodyLeftScapula, NAME_None },
		{ EOculusXRBoneID::BodyLeftArmUpper, "upperarm_l" },
		{ EOculusXRBoneID::BodyLeftArmLower, "lowerarm_l" },
		{ EOculusXRBoneID::BodyLeftHandWristTwist, NAME_None },

		{ EOculusXRBoneID::BodyRightShoulder, "clavicle_r" },
		{ EOculusXRBoneID::BodyRightScapula, NAME_None },
		{ EOculusXRBoneID::BodyRightArmUpper, "upperarm_r" },
		{ EOculusXRBoneID::BodyRightArmLower, "lowerarm_r" },
		{ EOculusXRBoneID::BodyRightHandWristTwist, NAME_None },

		{ EOculusXRBoneID::BodyLeftHandPalm, NAME_None },
		{ EOculusXRBoneID::BodyLeftHandWrist, "hand_l" },
		{ EOculusXRBoneID::BodyLeftHandThumbMetacarpal, "thumb_01_l" },
		{ EOculusXRBoneID::BodyLeftHandThumbProximal, "thumb_02_l" },
		{ EOculusXRBoneID::BodyLeftHandThumbDistal, "thumb_03_l" },
		{ EOculusXRBoneID::BodyLeftHandThumbTip, NAME_None },
		{ EOculusXRBoneID::BodyLeftHandIndexMetacarpal, "index_metacarpal_l" },
		{ EOculusXRBoneID::BodyLeftHandIndexProximal, "index_01_l" },
		{ EOculusXRBoneID::BodyLeftHandIndexIntermediate, "index_02_l" },
		{ EOculusXRBoneID::BodyLeftHandIndexDistal, "index_03_l" },
		{ EOculusXRBoneID::BodyLeftHandIndexTip, NAME_None },
		{ EOculusXRBoneID::BodyLeftHandMiddleMetacarpal, "middle_metacarpal_l" },
		{ EOculusXRBoneID::BodyLeftHandMiddleProximal, "middle_01_l" },
		{ EOculusXRBoneID::BodyLeftHandMiddleIntermediate, "middle_02_l" },
		{ EOculusXRBoneID::BodyLeftHandMiddleDistal, "middle_03_l" },
		{ EOculusXRBoneID::BodyLeftHandMiddleTip, NAME_None },
		{ EOculusXRBoneID::BodyLeftHandRingMetacarpal, "ring_metacarpal_l" },
		{ EOculusXRBoneID::BodyLeftHandRingProximal, "ring_01_l" },
		{ EOculusXRBoneID::BodyLeftHandRingIntermediate, "ring_02_l" },
		{ EOculusXRBoneID::BodyLeftHandRingDistal, "ring_03_l" },
		{ EOculusXRBoneID::BodyLeftHandRingTip, NAME_None },
		{ EOculusXRBoneID::BodyLeftHandLittleMetacarpal, "pinky_metacarpal_l" },
		{ EOculusXRBoneID::BodyLeftHandLittleProximal, "pinky_01_l" },
		{ EOculusXRBoneID::BodyLeftHandLittleIntermediate, "pinky_02_l" },
		{ EOculusXRBoneID::BodyLeftHandLittleDistal, "pinky_03_l" },
		{ EOculusXRBoneID::BodyLeftHandLittleTip, NAME_None },

		{ EOculusXRBoneID::BodyRightHandPalm, NAME_None },
		{ EOculusXRBoneID::BodyRightHandWrist, "hand_r" },
		{ EOculusXRBoneID::BodyRightHandThumbMetacarpal, "thumb_01_r" },
		{ EOculusXRBoneID::BodyRightHandThumbProximal, "thumb_02_r" },
		{ EOculusXRBoneID::BodyRightHandThumbDistal, "thumb_03_r" },
		{ EOculusXRBoneID::BodyRightHandThumbTip, NAME_None },
		{ EOculusXRBoneID::BodyRightHandIndexMetacarpal, "index_metacarpal_r" },
		{ EOculusXRBoneID::BodyRightHandIndexProximal, "index_01_r" },
		{ EOculusXRBoneID::BodyRightHandIndexIntermediate, "index_02_r" },
		{ EOculusXRBoneID::BodyRightHandIndexDistal, "index_03_r" },
		{ EOculusXRBoneID::BodyRightHandIndexTip, NAME_None },
		{ EOculusXRBoneID::BodyRightHandMiddleMetacarpal, "middle_metacarpal_r" },
		{ EOculusXRBoneID::BodyRightHandMiddleProximal, "middle_01_r" },
		{ EOculusXRBoneID::BodyRightHandMiddleIntermediate, "middle_02_r" },
		{ EOculusXRBoneID::BodyRightHandMiddleDistal, "middle_03_r" },
		{ EOculusXRBoneID::BodyRightHandMiddleTip, NAME_None },
		{ EOculusXRBoneID::BodyRightHandRingMetacarpal, "ring_metacarpal_r" },
		{ EOculusXRBoneID::BodyRightHandRingProximal, "ring_01_r" },
		{ EOculusXRBoneID::BodyRightHandRingIntermediate, "ring_02_r" },
		{ EOculusXRBoneID::BodyRightHandRingDistal, "ring_03_r" },
		{ EOculusXRBoneID::BodyRightHandRingTip, NAME_None },
		{ EOculusXRBoneID::BodyRightHandLittleMetacarpal, "pinky_metacarpal_r" },
		{ EOculusXRBoneID::BodyRightHandLittleProximal, "pinky_01_r" },
		{ EOculusXRBoneID::BodyRightHandLittleIntermediate, "pinky_02_r" },
		{ EOculusXRBoneID::BodyRightHandLittleDistal, "pinky_03_r" },
		{ EOculusXRBoneID::BodyRightHandLittleTip, NAME_None },

		{ EOculusXRBoneID::BodyLeftUpperLeg, "thigh_l" },
		{ EOculusXRBoneID::BodyLeftLowerLeg, "calf_l" },
		{ EOculusXRBoneID::BodyLeftFootAnkleTwist, NAME_None },
		{ EOculusXRBoneID::BodyLeftFootAnkle, "foot_l" },
		{ EOculusXRBoneID::BodyLeftFootSubtalar, NAME_None },
		{ EOculusXRBoneID::BodyLeftFootTransverse, NAME_None },
		{ EOculusXRBoneID::BodyLeftFootBall, "ball_l" },
		{ EOculusXRBoneID::BodyRightUpperLeg, "thigh_r" },
		{ EOculusXRBoneID::BodyRightLowerLeg, "calf_r" },
		{ EOculusXRBoneID::BodyRightFootAnkleTwist, NAME_None },
		{ EOculusXRBoneID::BodyRightFootAnkle, "foot_r" },
		{ EOculusXRBoneID::BodyRightFootSubtalar, NAME_None },
		{ EOculusXRBoneID::BodyRightFootTransverse, NAME_None },
		{ EOculusXRBoneID::BodyRightFootBall, "ball_r" },
	};

	/**
	 * Switch between retargeting modes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|BodyTracking", meta = (PinShownByDefault))
	EOculusXRBodyRetargetingMode RetargetingMode = EOculusXRBodyRetargetingMode::RotationAndPositions;

	/**
	 * Forward vector axis is the direction towards which the target mesh is oriented.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "OculusXR|BodyTracking")
	EOculusXRAxis ForwardMesh = EOculusXRAxis::Y;

	/**
	 * Behavior for Root Motion - Combine to Root is more compatible with most Locomotion systems.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXR|BodyTracking", meta = (PinShownByDefault))
	EOculusXRBodyRetargetingRootMotionBehavior RootMotionBehavior = EOculusXRBodyRetargetingRootMotionBehavior::CombineToRoot;

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

private:
	TSharedPtr<FOculusXRBodyRetargeter> RetargeterInstance;

	// U Type Data - cached from other location
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	USkeleton* Skeleton = nullptr;

	float Scale = 100.f;
};
