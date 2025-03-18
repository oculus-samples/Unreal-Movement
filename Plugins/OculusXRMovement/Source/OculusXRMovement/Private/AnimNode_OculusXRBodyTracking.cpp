/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "AnimNode_OculusXRBodyTracking.h"
#include "OculusXRAnimNodeBodyRetargeter.h"
#include "OculusXRMovement.h"
#include "OculusXRRetargeting.h"
#include "Animation/AnimInstanceProxy.h"
#include "OculusXRRetargetingUtils.h"
#include "DrawDebugHelpers.h"

void FAnimNode_OculusXRBodyTracking::Initialize_AnyThread(const FAnimationInitializeContext& Context)
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

	if (SkeletalMeshComponent == nullptr)
		UE_LOG(LogOculusXRRetargeting, Warning, TEXT("SkeletalMeshComponent is null"))

	if (!FOculusXRRetargetingUtils::GetUnitScaleFactorFromSettings(SkeletalMeshComponent->GetWorld(), Scale))
	{
		UE_LOG(LogOculusXRRetargeting, Warning, TEXT("Cannot get world settings for body retargetting asset."));
	}
}

void FAnimNode_OculusXRBodyTracking::PreUpdate(const UAnimInstance* InAnimInstance) {}

void FAnimNode_OculusXRBodyTracking::Evaluate_AnyThread(FPoseContext& Output)
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

	FOculusXRBodyState BodyState;
	OculusXRMovement::GetBodyState(BodyState, Scale);

	if (!RetargeterInstance)
	{
		RetargeterInstance = TSharedPtr<FOculusXRBodyRetargeter>(reinterpret_cast<FOculusXRBodyRetargeter*>(new FOculusXRAnimNodeBodyRetargeter()));
		RetargeterInstance->Initialize(RetargetingMode, RootMotionBehavior, ForwardMesh, &BoneRemapping);
	}
	if (RetargetingMode != RetargeterInstance->GetRetargetingMode() || RootMotionBehavior != RetargeterInstance->GetRootMotionBehavior())
	{
		RetargeterInstance->Initialize(RetargetingMode, RootMotionBehavior, ForwardMesh, &BoneRemapping);
	}
	if (!RetargeterInstance->RetargetFromBodyState(BodyState, SkeletalMeshComponent, Scale, Output))
	{
		if (SkeletalMeshComponent && SkeletalMeshComponent->GetWorld()->IsGameWorld())
		{
			UE_LOG(LogOculusXRRetargeting, Warning, TEXT("No valid delta rotations or skeletons"));
		}
	}
	RetargeterInstance->SetDebugPoseMode(DebugPoseMode);
	RetargeterInstance->SetDebugDrawMode(DebugDrawMode);
}

void FAnimNode_OculusXRBodyTracking::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	InputPose.Update(Context);
	// Evaluate pin inputs
	GetEvaluateGraphExposedInputs().Execute(Context);
}
