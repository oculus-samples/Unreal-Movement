/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "AnimNode_OculusXREyeTracking.h"
#include "OculusXRMovement.h"
#include "OculusXRRetargeting.h"
#include "Animation/AnimInstanceProxy.h"
#include "OculusXRRetargetingUtils.h"

void FAnimNode_OculusXREyeTracking::Initialize_AnyThread(const FAnimationInitializeContext& Context)
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
}

void FAnimNode_OculusXREyeTracking::PreUpdate(const UAnimInstance* InAnimInstance) {}

void FAnimNode_OculusXREyeTracking::Evaluate_AnyThread(FPoseContext& Output)
{
	InputPose.Evaluate(Output);

	FCSPose<FCompactPose> MeshPoses;
	MeshPoses.InitPose(Output.Pose);

	// This animation node is executed during the packaging step.
	// During that time, the MetaXR plugin is not available and any calls to it will crash the editor,
	// preventing the packaging process from completing.
	// To avoid this, we check if the plugin is available before calling any of its functions.
	if (!GEngine || !GEngine->XRSystem.IsValid())
	{
		UE_LOG(LogOculusXRRetargeting, Warning, TEXT("XR tracking is not loaded and available. Cannot retarget body at this time."));
		return;
	}

	FBoneContainer BoneContainer = Output.Pose.GetBoneContainer();

	if (!HasSetInitialRotations)
		RecalculateInitialRotations(BoneContainer);

	FOculusXREyeGazesState GazesState;
	OculusXRMovement::GetEyeGazesState(GazesState);

	// Left eye

	uint32 LeftEyeIndex = BoneContainer.GetPoseBoneIndexForBoneName(LeftEyeBone);

	if (LeftEyeIndex != INDEX_NONE)
	{
		FCompactPoseBoneIndex LeftEyeId = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(LeftEyeIndex));

		FTransform CurrentTransform = MeshPoses.GetComponentSpaceTransform(LeftEyeId);
		FTransform GazeTransform = FTransform(GazesState.EyeGazes[0].Orientation, FVector::ZeroVector);

		CurrentTransform.SetRotation(GazeTransform.GetRotation() * InitialLeftRotation);
		MeshPoses.SetComponentSpaceTransform(LeftEyeId, CurrentTransform);
	}

	// Right eye

	uint32 RightEyeIndex = BoneContainer.GetPoseBoneIndexForBoneName(RightEyeBone);

	if (RightEyeIndex != INDEX_NONE)
	{
		FCompactPoseBoneIndex RightEyeId = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(RightEyeIndex));

		FTransform CurrentTransform = MeshPoses.GetComponentSpaceTransform(RightEyeId);
		FTransform GazeTransform = FTransform(GazesState.EyeGazes[1].Orientation, FVector::ZeroVector);

		CurrentTransform.SetRotation(GazeTransform.GetRotation() * InitialRightRotation);
		MeshPoses.SetComponentSpaceTransform(RightEyeId, CurrentTransform);
	}

	FCSPose<FCompactPose>::ConvertComponentPosesToLocalPosesSafe(MeshPoses, Output.Pose);
}

void FAnimNode_OculusXREyeTracking::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	InputPose.Update(Context);
}

void FAnimNode_OculusXREyeTracking::RecalculateInitialRotations(FBoneContainer BoneContainer)
{
	const auto BoneIds = new TArray<FCompactPoseBoneIndex>;
	const auto ParentIndices = new TArray<int>;
	const auto ComponentTransforms = new TArray<FTransform>;
	const auto LocalTransforms = new TArray<FTransform>;

	// Get full skeleton
	const auto TargetReferenceSkeleton = BoneContainer.GetReferenceSkeleton();

	// Get bones used in current LOD
	const auto BoneIndicesArray = BoneContainer.GetBoneIndicesArray();

	for (int i = 0; i < BoneIndicesArray.Num(); ++i)
	{
		// BoneIndex = Index in full skeleton, only used to get transform, not parent
		// BoneId = Index in bone container / LOD skeleton, used to find correct parent index

		const auto BoneIndex = BoneIndicesArray[i];
		const auto BoneId = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneIndex));
		const auto ParentBoneIndex = BoneContainer.GetParentBoneIndex(BoneId);
		const auto LocalBoneTransform = TargetReferenceSkeleton.GetRawRefBonePose()[BoneIndex];

		BoneIds->Add(BoneId);
		ParentIndices->Add(BoneIds->Find(ParentBoneIndex));
		LocalTransforms->Add(LocalBoneTransform);
	}

	for (int i = 0; i < BoneIndicesArray.Num(); ++i)
	{
		const auto ParentBoneIndex = ParentIndices->operator[](i);
		const auto BoneLocalTransform = LocalTransforms->operator[](i);
		if (ParentBoneIndex == INDEX_NONE)
		{
			ComponentTransforms->Add(BoneLocalTransform);
		}
		else
		{
			const auto ParentBoneTransform = ComponentTransforms->operator[](ParentBoneIndex);
			ComponentTransforms->Add(BoneLocalTransform * ParentBoneTransform);
		}
	}

	auto LeftEyeIndex = BoneContainer.GetPoseBoneIndexForBoneName(LeftEyeBone);
	if (LeftEyeIndex != INDEX_NONE)
		InitialLeftRotation = ComponentTransforms->operator[](LeftEyeIndex).GetRotation();

	auto RightEyeIndex = BoneContainer.GetPoseBoneIndexForBoneName(RightEyeBone);
	if (RightEyeIndex != INDEX_NONE)
		InitialRightRotation = ComponentTransforms->operator[](RightEyeIndex).GetRotation();

	HasSetInitialRotations = true;
}
