/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "AnimNode_OculusXRFaceTracking.h"
#include "OculusXRMovement.h"
#include "OculusXRRetargeting.h"
#include "Animation/AnimInstanceProxy.h"
#include "OculusXRRetargetingUtils.h"

void FAnimNode_OculusXRFaceTracking::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	InputPose.Initialize(Context);

	// This animation node is executed during the packaging step.
	// During that time, the MetaXR plugin is not available and any calls to it will crash the editor,
	// preventing the packaging process from completing.
	// To avoid this, we check if the plugin is available before calling any of its functions.
	if (!GEngine || !GEngine->XRSystem.IsValid())
	{
		UE_LOG(LogOculusXRRetargeting, Warning, TEXT("XR tracking is not loaded and available. Cannot retarget body at this time."));
		return;
	}

	SkeletalMeshComponent = Context.AnimInstanceProxy->GetSkelMeshComponent();
}

void FAnimNode_OculusXRFaceTracking::PreUpdate(const UAnimInstance* InAnimInstance) {}

void FAnimNode_OculusXRFaceTracking::Evaluate_AnyThread(FPoseContext& Output)
{
	InputPose.Evaluate(Output);

	// This animation node is executed during the packaging step.
	// During that time, the MetaXR plugin is not available and any calls to it will crash the editor,
	// preventing the packaging process from completing.
	// To avoid this, we check if the plugin is available before calling any of its functions.
	if (!GEngine || !GEngine->XRSystem.IsValid())
	{
		UE_LOG(LogOculusXRRetargeting, Warning, TEXT("XR tracking is not loaded and available. Cannot retarget body at this time."));
		return;
	}

	FOculusXRFaceState FaceState;
	OculusXRMovement::GetFaceState(FaceState);

	for (int32 FaceExpressionIndex = 0; FaceExpressionIndex < FaceState.ExpressionWeights.Num(); ++FaceExpressionIndex)
	{
		const auto FaceExpression = static_cast<EOculusXRFaceExpression>(FaceExpressionIndex);
		if (ExpressionNames.Contains(FaceExpression))
		{
			auto ExpressionCurves = ExpressionNames[FaceExpression];
			float Value = FaceState.ExpressionWeights[FaceExpressionIndex];

			// Apply Facial Expression Modifiers
			if (ExpressionModifiers.Contains(FaceExpression))
			{
				FOculusXRFaceExpressionModifierNew Modifier = ExpressionModifiers[FaceExpression];
				Value = FMath::Clamp(Value * Modifier.Multiplier, Modifier.MinValue, Modifier.MaxValue);
			}

			for (const auto& CurveName : ExpressionCurves.CurveNames)
			{
				Output.Curve.Set(CurveName, Value);
			}
		}
	}
}

void FAnimNode_OculusXRFaceTracking::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	InputPose.Update(Context);
}
