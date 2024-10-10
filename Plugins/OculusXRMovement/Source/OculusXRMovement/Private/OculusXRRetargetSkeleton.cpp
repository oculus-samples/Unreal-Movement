/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRRetargetSkeleton.h"

#include "OculusXRRetargeting.h"

const EOculusXRBoneID FOculusXRRetargetSkeletonEOculusXRBoneID::kINVALID_BONE_ID = EOculusXRBoneID::None;
const FCompactPoseBoneIndex FOculusXRRetargetSkeletonFCompactPoseBoneIndex::kINVALID_BONE_ID = FCompactPoseBoneIndex(INDEX_NONE);

FOculusXRRetargetSkeletonEOculusXRBoneID Factory::FromOculusXRBodySkeleton(
	const FOculusXRBodySkeleton& SourceReferenceSkeleton,
	const FTransform& TrackingSpaceToComponentSpace)
{
	TArray<TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>> jointData;
	jointData.Reserve(SourceReferenceSkeleton.NumBones);

	for (int i = 0; i < SourceReferenceSkeleton.NumBones; ++i)
	{
		const auto& BoneData = SourceReferenceSkeleton.Bones[i];

		jointData.Add({ BoneData.BoneId,
			BoneData.ParentBoneIndex == EOculusXRBoneID::None ? INDEX_NONE : static_cast<int>(BoneData.ParentBoneIndex),
			FTransform::Identity, // No Local Transform data
			FTransform(
				SourceReferenceSkeleton.Bones[i].Orientation,
				SourceReferenceSkeleton.Bones[i].Position,
				FVector::OneVector)
				* TrackingSpaceToComponentSpace });
	}

	// Calculate the local transforms from the component transforms
	TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>::CalculateComponentToLocalSpace(jointData);

	return FOculusXRRetargetSkeletonEOculusXRBoneID(jointData);
}

FOculusXRRetargetSkeletonEOculusXRBoneID Factory::FromOculusXRBodyState(
	const FOculusXRBodyState& SourceFrameSkeleton,
	const FOculusXRBodySkeleton& SourceReferenceSkeleton,
	const FTransform& TrackingSpaceToComponentSpace,
	const EOculusXRBodyRetargetingRootMotionBehavior rootMotionBehavior)
{
	TArray<TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>> jointData;
	TMap<EOculusXRBoneID, int> BoneIdToIndexMap;
	jointData.Reserve(SourceReferenceSkeleton.NumBones);
	BoneIdToIndexMap.Reserve(SourceReferenceSkeleton.NumBones);

	// Use this if we're Combining the Hip Translation/Rotation to
	// Root - if the Tracking expands, the capsule will float if we use
	// the hip skeleton offset as a reference.
	float minZPosition = FLT_MAX;
	float minRestZPosition = FLT_MAX;

	for (int i = 0; i < SourceReferenceSkeleton.NumBones; ++i)
	{
		const auto& Joint = SourceFrameSkeleton.Joints[i];
		const auto& BoneData = SourceReferenceSkeleton.Bones[i];

		FTransform frameTransform(Joint.Orientation, Joint.Position, FVector::OneVector);
		const EOculusXRBoneID TrackingBoneId = Joint.bIsValid ? BoneData.BoneId : EOculusXRBoneID::None;

		jointData.Add({ TrackingBoneId,
			BoneData.ParentBoneIndex == EOculusXRBoneID::None ? INDEX_NONE : static_cast<int>(BoneData.ParentBoneIndex),
			FTransform::Identity, // No Local Transform data
			frameTransform * TrackingSpaceToComponentSpace });

		if (TrackingBoneId != EOculusXRBoneID::BodyRoot)
		{
			minZPosition = FMath::Min(minZPosition, frameTransform.GetLocation().Z);
			minRestZPosition = FMath::Min(minRestZPosition, BoneData.Position.Z);
		}

		if (TrackingBoneId != EOculusXRBoneID::None)
		{
			check(!BoneIdToIndexMap.Contains(TrackingBoneId));
			BoneIdToIndexMap.Add(TrackingBoneId, i);
		}
	}

	// Calculate the local transforms from the component transforms
	TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>::CalculateComponentToLocalSpace(jointData);

	// If mode is CombineToRoot, we'll shift the excess up motion from the hip to the root
	// (So when the character jumps, the root translates up)
	// We'll also extract the Yaw from the hip and shift it to the root.
	// This allows for better compatibility with Locomotion systems
	if (FOculusXRBodyRetargeter::IsModifiedRootBehavior(rootMotionBehavior) && BoneIdToIndexMap.Contains(EOculusXRBoneID::BodyHips) && BoneIdToIndexMap.Contains(EOculusXRBoneID::BodyRoot))
	{
		const int HipJointIdx = BoneIdToIndexMap[EOculusXRBoneID::BodyHips];
		const int rootJointIdx = BoneIdToIndexMap[EOculusXRBoneID::BodyRoot];

		const auto& HipRest = SourceReferenceSkeleton.Bones[HipJointIdx];
		const auto& HipFrame = SourceFrameSkeleton.Joints[HipJointIdx];
		const auto& RootRest = SourceReferenceSkeleton.Bones[rootJointIdx];

		TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>& HipJointEntry = jointData[HipJointIdx];
		TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>& RootJoint = jointData[rootJointIdx];

		// First cover translation.  Use the minZPosition we calculated in the first pass to accommodate
		// any error introduced in the tracker (it allows for ~10% scaling which can result in the standing
		// hip translation being longer than the T-Pose skeleton).
		const float CalculatedHipHeight = HipFrame.Position.Z - minZPosition + FMath::Max(minRestZPosition, 0.0f);
		// Hack 1.05 multiplier - 5% buffer due to floaty solver behavior (keeps feet/capsule on the floor unless jumping)
		const float MaxHipHeight = FMath::Max(HipRest.Position.Z, CalculatedHipHeight) * 1.05f;
		const float ExcessHipTranslation = FMath::Max(HipFrame.Position.Z - MaxHipHeight, 0.0f);

		if (ExcessHipTranslation > 0.0f)
		{
			const FVector ExcessHipTranslationVector(0.0f, 0.0f, ExcessHipTranslation);
			HipJointEntry.LocalTransform.SetLocation(HipJointEntry.LocalTransform.GetLocation() - ExcessHipTranslationVector);
			RootJoint.LocalTransform.SetLocation(RootJoint.LocalTransform.GetLocation() + ExcessHipTranslationVector);
		}

		// Calculate the Yaw next - we need to flatten the Hip so we can determine it's yaw relative to the rest pose
		const FQuat restLocalRotation = FQuat(RootRest.Orientation).Inverse() * FQuat(HipRest.Orientation);
		const FQuat frameLocalRotation(HipJointEntry.LocalTransform.GetRotation());

		const FVector flatRightDirection = restLocalRotation.RotateVector(FVector::RightVector);
		const FVector frameFwdVector = frameLocalRotation.RotateVector(FVector::ForwardVector);
		const FVector flatUpVector = frameFwdVector.Cross(flatRightDirection).GetSafeNormal();
		const FVector flatFwdVector = flatRightDirection.Cross(flatUpVector).GetSafeNormal();
		const FQuat flatFrameRotation = FRotationMatrix::MakeFromXZ(flatFwdVector, flatUpVector).ToQuat();
		const FQuat flatRotationDelta = (restLocalRotation * flatFrameRotation.Inverse()).GetNormalized();

		// Once calculated, apply the rotation to pull the hip back to identity facing, then apply the inverse to the root.
		HipJointEntry.LocalTransform.SetRotation((flatRotationDelta * HipJointEntry.LocalTransform.GetRotation()).GetNormalized());

		if (rootMotionBehavior == EOculusXRBodyRetargetingRootMotionBehavior::ZeroOutRootTranslationHipYaw)
		{
			// Reset the Root joint to it's RestPose (basically Identity Matrix)
			RootJoint.LocalTransform = FTransform(RootRest.Orientation, RootRest.Position, FVector::OneVector) * TrackingSpaceToComponentSpace;
		}
		else
		{
			RootJoint.LocalTransform.SetRotation((RootJoint.LocalTransform.GetRotation() * flatRotationDelta.Inverse()).GetNormalized());
		}

		// Recalculate our skeleton from local space since we changed the root and hips
		TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>::CalculateLocalToComponentSpace(jointData);
	}

	return FOculusXRRetargetSkeletonEOculusXRBoneID(jointData);
}

FOculusXRRetargetSkeletonFCompactPoseBoneIndex Factory::FromBoneContainer(
	const FBoneContainer& TargetBoneContainer)
{
	TArray<TOculusXRRetargetSkeletonJoint<FCompactPoseBoneIndex>> jointData;

	// TODO: Fix to be more bulletproof - Based on an assumption that the Joints are
	// sorted from Parent to Child.  If it isn't, then the Parent Transform/Index may not get calculated before the child.
	TMap<FCompactPoseBoneIndex, int> jointIdxMap;

	// Get full skeleton
	const auto& TargetReferenceSkeleton = TargetBoneContainer.GetReferenceSkeleton();

	// Get bones used in current LOD
	const auto& BoneIndicesArray = TargetBoneContainer.GetBoneIndicesArray();

	jointData.Reserve(BoneIndicesArray.Num());
	jointIdxMap.Reserve(BoneIndicesArray.Num());

	for (int i = 0; i < BoneIndicesArray.Num(); ++i)
	{
		// BoneIndex = Index in full skeleton, only used to get transform, not parent
		// BoneId = Index in bone container / LOD skeleton, used to find correct parent index

		const auto BoneIndex = BoneIndicesArray[i];
		const auto BoneId = TargetBoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(BoneIndex));
		const auto ParentBoneId = TargetBoneContainer.GetParentBoneIndex(BoneId);

		jointData.Add({ BoneId,
			jointIdxMap.Contains(ParentBoneId) ? jointIdxMap[ParentBoneId] : INDEX_NONE,
			TargetReferenceSkeleton.GetRawRefBonePose()[BoneIndex],
			FTransform::Identity });

		jointIdxMap.Add(BoneId, i);
	}

	// Calculate the component transforms from the local transforms
	TOculusXRRetargetSkeletonJoint<FCompactPoseBoneIndex>::CalculateLocalToComponentSpace(jointData);

	return FOculusXRRetargetSkeletonFCompactPoseBoneIndex(jointData);
}

FOculusXRRetargetSkeletonFCompactPoseBoneIndex Factory::FromComponentSpaceTransformArray(
	const FAbstractRetargetSkeleton& BaseSkeleton,
	const TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& ComponentSpaceTransformPairs)
{
	check(BaseSkeleton.GetNumBones() == ComponentSpaceTransformPairs.Num());
	TArray<TOculusXRRetargetSkeletonJoint<FCompactPoseBoneIndex>> jointData;
	jointData.Reserve(ComponentSpaceTransformPairs.Num());

	for (int i = 0; i < ComponentSpaceTransformPairs.Num(); ++i)
	{
		jointData.Add({ ComponentSpaceTransformPairs[i].Get<FCompactPoseBoneIndex>(),
			BaseSkeleton.GetParentBoneIndex(i),
			FTransform::Identity,
			ComponentSpaceTransformPairs[i].Get<FTransform>() });
	}

	// Calculate the local transforms from the component transforms
	TOculusXRRetargetSkeletonJoint<FCompactPoseBoneIndex>::CalculateComponentToLocalSpace(jointData);

	return FOculusXRRetargetSkeletonFCompactPoseBoneIndex(jointData);
}
