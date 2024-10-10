/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "OculusXRLiveLinkRetargetBodyAsset.h"
#include "OculusXRRetargetSkeleton.h"
#include "Animation/AnimNodeBase.h"
#include "OculusXRMorphTargetsController.h"
#include "AnimNode_OculusXREyeTracking.generated.h"

USTRUCT(Blueprintable)
struct OCULUSXRRETARGETING_API FAnimNode_OculusXREyeTracking : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FPoseLink InputPose;

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	UPROPERTY(EditDefaultsOnly, Category = "OculusXR|EyeTracking")
	FName LeftEyeBone = "LeftEye";

	UPROPERTY(EditDefaultsOnly, Category = "OculusXR|EyeTracking")
	FName RightEyeBone = "RightEye";

private:
	FQuat InitialLeftRotation;
	FQuat InitialRightRotation;

	bool HasSetInitialRotations = false;
	void RecalculateInitialRotations(FBoneContainer);
};
