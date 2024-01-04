/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRRetargeterAnimNode.h"
#include "Animation/AnimInstanceProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OculusXRRetargeterAnimNode)

void FOculusXRRetargeterAnimNode::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	FAnimNode_Base::Initialize_AnyThread(Context);

	// Initial update of the node, so we dont have a frame-delay on setup
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (!Processor && IsInGameThread())
	{
		Processor = NewObject<UIKRetargetProcessor>(Context.AnimInstanceProxy->GetSkelMeshComponent());
	}
}

void FOculusXRRetargeterAnimNode::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)

	const FBoneContainer& RequiredBones = Context.AnimInstanceProxy->GetRequiredBones();
	if (!RequiredBones.IsValid())
	{
		return;
	}

	if (!Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSkeletalMeshAsset())
	{
		return;
	}

	// rebuild mapping
	RequiredToTargetBoneMapping.Reset();

	const FReferenceSkeleton& RefSkeleton = RequiredBones.GetReferenceSkeleton();
	const FReferenceSkeleton& TargetSkeleton = Context.AnimInstanceProxy->GetSkelMeshComponent()->GetSkeletalMeshAsset()->GetRefSkeleton();
	const TArray<FBoneIndexType>& RequiredBonesArray = RequiredBones.GetBoneIndicesArray();
	for (int32 Index = 0; Index < RequiredBonesArray.Num(); ++Index)
	{
		const FBoneIndexType ReqBoneIndex = RequiredBonesArray[Index];
		if (ReqBoneIndex != INDEX_NONE)
		{
			const FName Name = RefSkeleton.GetBoneName(ReqBoneIndex);
			const int32 TargetBoneIndex = TargetSkeleton.FindBoneIndex(Name);
			if (TargetBoneIndex != INDEX_NONE)
			{
				// store require bone to target bone indices
				RequiredToTargetBoneMapping.Emplace(Index, TargetBoneIndex);
			}
		}
	}
}

void FOculusXRRetargeterAnimNode::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)
	FAnimNode_Base::Update_AnyThread(Context);
	// this introduces a frame of latency in setting the pin-driven source component,
	// but we cannot do the work to extract transforms on a worker thread as it is not thread safe.
	GetEvaluateGraphExposedInputs().Execute(Context);
	DeltaTime += Context.GetDeltaTime();
}

void FOculusXRRetargeterAnimNode::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)

	//SCOPE_CYCLE_COUNTER(STAT_IKRetarget);

	if (!(IKRetargeterAsset && Processor && SourceMeshComponent.IsValid()))
	{
		Output.ResetToRefPose();
		return;
	}

	// it's possible in editor to have anim instances initialized before PreUpdate() is called
	// which results in trying to run the retargeter without an source pose to copy from
	const bool bSourcePoseCopied = !SourceMeshComponentSpaceBoneTransforms.IsEmpty();

	// ensure processor was initialized with the currently used assets (source/target meshes and retarget asset)
	// if processor is not ready this tick, it will be next tick as this state will trigger re-initialization
	const TObjectPtr<USkeletalMesh> SourceMesh = Cast<class USkeletalMesh>(SourceMeshComponent->GetSkinnedAsset()); // SourceMeshComponent->GetSkeletalMeshAsset();
	const TObjectPtr<USkeletalMesh> TargetMesh = Output.AnimInstanceProxy->GetSkelMeshComponent()->GetSkeletalMeshAsset();
	const bool bIsProcessorReady = Processor->WasInitializedWithTheseAssets(SourceMesh, TargetMesh, IKRetargeterAsset);

	// if not ready to run, skip retarget and output the ref pose
	if (!(bIsProcessorReady && bSourcePoseCopied))
	{
		Output.ResetToRefPose();
		return;
	}

#if WITH_EDITOR
	// live preview source asset settings in the retarget, editor only
	// NOTE: this copies goal targets as well, but these are overwritten by IK chain goals
	if (bDriveWithAsset)
	{
		Processor->ApplySettingsFromAsset();
		if (const FRetargetProfile* CurrentProfile = IKRetargeterAsset->GetCurrentProfile())
		{
			Processor->ApplySettingsFromProfile(*CurrentProfile);
		}
	}
#endif

	// apply custom profile settings to the processor
	Processor->ApplySettingsFromProfile(CustomRetargetProfile);

	// run the retargeter
	const TArray<FTransform>& RetargetedPose = Processor->RunRetargeter(SourceMeshComponentSpaceBoneTransforms, SpeedValuesFromCurves, DeltaTime);
	DeltaTime = 0.0f;

	// copy pose back
	FCSPose<FCompactPose> ComponentPose;
	ComponentPose.InitPose(Output.Pose);
	const FCompactPose& CompactPose = ComponentPose.GetPose();
	for (const TPair<int32, int32>& Pair : RequiredToTargetBoneMapping)
	{
		const FCompactPoseBoneIndex CompactBoneIndex(Pair.Key);
		if (CompactPose.IsValidIndex(CompactBoneIndex))
		{
			const int32 TargetBoneIndex = Pair.Value;
			ComponentPose.SetComponentSpaceTransform(CompactBoneIndex, RetargetedPose[TargetBoneIndex]);
		}
	}

	// convert to local space
	FCSPose<FCompactPose>::ConvertComponentPosesToLocalPoses(ComponentPose, Output.Pose);
}

void FOculusXRRetargeterAnimNode::PreUpdate(const UAnimInstance* InAnimInstance)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(PreUpdate)

	if (!IKRetargeterAsset)
	{
		return;
	}

	if (!Processor)
	{
		Processor = NewObject<UIKRetargetProcessor>(InAnimInstance->GetOwningComponent());
	}

	const TObjectPtr<USkeletalMeshComponent> TargetMeshComponent = InAnimInstance->GetSkelMeshComponent();
	if (EnsureProcessorIsInitialized(TargetMeshComponent))
	{
		CopyBoneTransformsFromSource(TargetMeshComponent);
	}
}

UIKRetargetProcessor* FOculusXRRetargeterAnimNode::GetRetargetProcessor() const
{
	return Processor;
}

bool FOculusXRRetargeterAnimNode::EnsureProcessorIsInitialized(const TObjectPtr<USkeletalMeshComponent> TargetMeshComponent)
{
	// has user supplied a retargeter asset?
	if (!IKRetargeterAsset)
	{
		return false;
	}

	// if user hasn't explicitly connected a source mesh, optionally use the parent mesh component (if there is one)
	if (!SourceMeshComponent.IsValid() && bUseAttachedParent)
	{
		// Walk up the attachment chain until we find a skeletal mesh component
		USkinnedMeshComponent* ParentComponent = nullptr;
		for (USceneComponent* AttachParentComp = TargetMeshComponent->GetAttachParent(); AttachParentComp != nullptr; AttachParentComp = AttachParentComp->GetAttachParent())
		{
			ParentComponent = Cast<USkinnedMeshComponent>(AttachParentComp);
			if (ParentComponent)
			{
				break;
			}
		}

		if (ParentComponent)
		{
			SourceMeshComponent = ParentComponent;
		}
	}

	// has a source mesh been plugged in or found?
	if (!SourceMeshComponent.IsValid())
	{
		return false; // can't do anything if we don't have a source mesh component
	}

	// check that both a source and target mesh exist
	const TObjectPtr<USkeletalMesh> SourceMesh = Cast<class USkeletalMesh>(SourceMeshComponent->GetSkinnedAsset()); // SourceMeshComponent->GetSkeletalMeshAsset();
	const TObjectPtr<USkeletalMesh> TargetMesh = TargetMeshComponent->GetSkeletalMeshAsset();
	if (!SourceMesh || !TargetMesh)
	{
		return false; // cannot initialize if components are missing skeletal mesh references
	}
	// check that both have skeleton assets (shouldn't get this far without a skeleton)
	const TObjectPtr<USkeleton> SourceSkeleton = SourceMesh->GetSkeleton();
	const TObjectPtr<USkeleton> TargetSkeleton = TargetMesh->GetSkeleton();
	if (!SourceSkeleton || !TargetSkeleton)
	{
		return false;
	}

	// try initializing the processor
	if (!Processor->WasInitializedWithTheseAssets(SourceMesh, TargetMesh, IKRetargeterAsset))
	{
		// initialize retarget processor with source and target skeletal meshes
		// (asset is passed in as outer UObject for new UIKRigProcessor)
		Processor->Initialize(SourceMesh, TargetMesh, IKRetargeterAsset);
	}

	return Processor->IsInitialized();
}

void FOculusXRRetargeterAnimNode::CopyBoneTransformsFromSource(USkeletalMeshComponent* TargetMeshComponent)
{
	// get the mesh component to use as the source
	const TObjectPtr<USkinnedMeshComponent> ComponentToCopyFrom = GetComponentToCopyPoseFrom();

	// this should not happen as we're guaranteed to be initialized at this stage
	// but just in case component is lost after initialization, we avoid a crash
	if (!ComponentToCopyFrom)
	{
		return;
	}

	// skip copying pose when component is no longer ticking
	if (!ComponentToCopyFrom->IsRegistered())
	{
		return;
	}

	SourceMeshComponentSpaceBoneTransforms.Reset();

	SourceMeshComponentSpaceBoneTransforms.Append(ComponentToCopyFrom->GetComponentSpaceTransforms()); // copy directly
}

TObjectPtr<USkinnedMeshComponent> FOculusXRRetargeterAnimNode::GetComponentToCopyPoseFrom() const
{
	// if our source is running under leader-pose, then get bone data from there
	if (SourceMeshComponent.IsValid())
	{
		if (USkeletalMeshComponent* LeaderPoseComponent = Cast<USkeletalMeshComponent>(SourceMeshComponent->LeaderPoseComponent.Get()))
		{
			return LeaderPoseComponent;
		}
	}
	return SourceMeshComponent.Get();
}
