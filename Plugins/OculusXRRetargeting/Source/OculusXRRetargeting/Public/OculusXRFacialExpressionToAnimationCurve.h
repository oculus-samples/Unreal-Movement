/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimNodeBase.h"
#include "OculusXRMovement/Public/OculusXRFaceTrackingComponent.h"
#include "OculusXRFacialExpressionMap.h"

#include "OculusXRFacialExpressionToAnimationCurve.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct OCULUSXRRETARGETING_API FOculusXRFacialExpressionToAnimationCurve : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, transient, Category = Oculus, meta = (PinShownByDefault))
	UOculusXRFaceTrackingComponent* FaceTrackingComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, transient, Category = Oculus, meta = (PinShownByDefault))
	UOculusXRFacialExpressionMap* OculusExpressionMapping = nullptr;

	/** Base Pose*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink BasePose;

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;

private:
	TMap<FName, float> CurveValues;
	TMap<FName, FSmartName> NameToSmartName;
};
