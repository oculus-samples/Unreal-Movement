/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRAnimNodeBodyRetargeter.h"
#include "OculusXRMovement.h"
#include "OculusXRRetargeting.h"
#include "OculusXRRetargetingUtils.h"

#define OCULUS_XR_DEBUG_DRAW_MODIFIED_ROOT_MOTION_BEHAVIOR (OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW && 0)

// Twist joints should diverge no more than 2 degrees from the joint they are aligned with
const float FOculusXRAnimNodeBodyRetargeter::kTWIST_JOINT_MIN_ANGLE_THRESHOLD = FMath::DegreesToRadians(2.0f);

const TArray<EOculusXRBoneID> FOculusXRAnimNodeBodyRetargeter::kALIGNABLE_HAND_JOINTS({
	EOculusXRBoneID::BodyLeftHandThumbMetacarpal,
	EOculusXRBoneID::BodyLeftHandThumbProximal,
	EOculusXRBoneID::BodyLeftHandThumbDistal,
	EOculusXRBoneID::BodyLeftHandThumbTip,
	EOculusXRBoneID::BodyLeftHandIndexMetacarpal,
	EOculusXRBoneID::BodyLeftHandIndexProximal,
	EOculusXRBoneID::BodyLeftHandIndexIntermediate,
	EOculusXRBoneID::BodyLeftHandIndexDistal,
	EOculusXRBoneID::BodyLeftHandIndexTip,
	EOculusXRBoneID::BodyLeftHandMiddleMetacarpal,
	EOculusXRBoneID::BodyLeftHandMiddleProximal,
	EOculusXRBoneID::BodyLeftHandMiddleIntermediate,
	EOculusXRBoneID::BodyLeftHandMiddleDistal,
	EOculusXRBoneID::BodyLeftHandMiddleTip,
	EOculusXRBoneID::BodyLeftHandRingMetacarpal,
	EOculusXRBoneID::BodyLeftHandRingProximal,
	EOculusXRBoneID::BodyLeftHandRingIntermediate,
	EOculusXRBoneID::BodyLeftHandRingDistal,
	EOculusXRBoneID::BodyLeftHandRingTip,
	EOculusXRBoneID::BodyLeftHandLittleMetacarpal,
	EOculusXRBoneID::BodyLeftHandLittleProximal,
	EOculusXRBoneID::BodyLeftHandLittleIntermediate,
	EOculusXRBoneID::BodyLeftHandLittleDistal,
	EOculusXRBoneID::BodyLeftHandLittleTip,
	EOculusXRBoneID::BodyRightHandThumbMetacarpal,
	EOculusXRBoneID::BodyRightHandThumbProximal,
	EOculusXRBoneID::BodyRightHandThumbDistal,
	EOculusXRBoneID::BodyRightHandThumbTip,
	EOculusXRBoneID::BodyRightHandIndexMetacarpal,
	EOculusXRBoneID::BodyRightHandIndexProximal,
	EOculusXRBoneID::BodyRightHandIndexIntermediate,
	EOculusXRBoneID::BodyRightHandIndexDistal,
	EOculusXRBoneID::BodyRightHandIndexTip,
	EOculusXRBoneID::BodyRightHandMiddleMetacarpal,
	EOculusXRBoneID::BodyRightHandMiddleProximal,
	EOculusXRBoneID::BodyRightHandMiddleIntermediate,
	EOculusXRBoneID::BodyRightHandMiddleDistal,
	EOculusXRBoneID::BodyRightHandMiddleTip,
	EOculusXRBoneID::BodyRightHandRingMetacarpal,
	EOculusXRBoneID::BodyRightHandRingProximal,
	EOculusXRBoneID::BodyRightHandRingIntermediate,
	EOculusXRBoneID::BodyRightHandRingDistal,
	EOculusXRBoneID::BodyRightHandRingTip,
	EOculusXRBoneID::BodyRightHandLittleMetacarpal,
	EOculusXRBoneID::BodyRightHandLittleProximal,
	EOculusXRBoneID::BodyRightHandLittleIntermediate,
	EOculusXRBoneID::BodyRightHandLittleDistal,
	EOculusXRBoneID::BodyRightHandLittleTip,
});

const TSet<EOculusXRBoneID> FOculusXRAnimNodeBodyRetargeter::GenerateTPoseJointSet()
{
	TSet<EOculusXRBoneID> BaseSet({ EOculusXRBoneID::BodyLeftArmLower,
		EOculusXRBoneID::BodyRightArmLower,
		EOculusXRBoneID::BodyLeftHandWrist,
		EOculusXRBoneID::BodyRightHandWrist,
		EOculusXRBoneID::BodyLeftHandPalm,
		EOculusXRBoneID::BodyRightHandPalm,
		EOculusXRBoneID::BodyLeftLowerLeg,
		EOculusXRBoneID::BodyLeftFootAnkle,
		EOculusXRBoneID::BodyRightLowerLeg,
		EOculusXRBoneID::BodyRightFootAnkle });

	BaseSet.Append(kALIGNABLE_HAND_JOINTS);

	return BaseSet;
}

const TSet<EOculusXRBoneID> FOculusXRAnimNodeBodyRetargeter::kTPOSE_ADJUSTABLE_JOINT_SET = GenerateTPoseJointSet();

// NOTE: This set is defined for joints that we skip alignment on during the T-Pose Alignment process.
// The metacarpel to proximal alignment of the tracking skeleton differs in both proportion and alignment from the rest
// pose of many skeletons.  So we need to skip these to get to align the fingers correctly.
// The Thumb doesn't have this same issue (on most hands) - so those are commented out, but left in as a reminder.
// Will revisit to see if there's a better technique for aligning the hands without changing the proportions through rotation
// (ie - ensuring the applied rotation is only against the relative forward/back vector based on the pose of the wrist).
const TSet<EOculusXRBoneID> FOculusXRAnimNodeBodyRetargeter::kTPOSE_SPECIAL_HANDLING_ADJUSTABLE_HAND_JOINTS({
	// EOculusXRBoneID::BodyLeftHandThumbProximal,
	EOculusXRBoneID::BodyLeftHandIndexProximal,
	EOculusXRBoneID::BodyLeftHandMiddleProximal,
	EOculusXRBoneID::BodyLeftHandRingProximal,
	EOculusXRBoneID::BodyLeftHandLittleProximal,
	// EOculusXRBoneID::BodyRightHandThumbProximal,
	EOculusXRBoneID::BodyRightHandIndexProximal,
	EOculusXRBoneID::BodyRightHandMiddleProximal,
	EOculusXRBoneID::BodyRightHandRingProximal,
	EOculusXRBoneID::BodyRightHandLittleProximal,
});

void FOculusXRAnimNodeBodyRetargeter::Initialize(
	const EOculusXRBodyRetargetingMode RetargetingMode,
	const EOculusXRBodyRetargetingRootMotionBehavior RootMotionBehavior,
	const EOculusXRAxis MeshForwardFacingDir,
	const TMap<EOculusXRBoneID, FName>* SourceToTargetNameMap)

{
	InitData.RetargetingMode = RetargetingMode;
	InitData.RootMotionBehavior = RootMotionBehavior;
	InitData.MeshForwardFacingDir = MeshForwardFacingDir;

	check(SourceToTargetNameMap && !SourceToTargetNameMap->IsEmpty());
	InitData.SourceToTargetNameMap = SourceToTargetNameMap;

	InitData.TargetFacingTransform = FOculusXRRetargetingUtils::DirectionTransform(MeshForwardFacingDir);
	InitData.TrackingSpaceToComponentSpace = FOculusXRRetargetingUtils::GetTrackingSpaceToComponentSpace(InitData.TargetFacingTransform);

	// Ensure we force an update to our Skeleton
	SourceReferenceInfo.Invalidate();
}

bool FOculusXRAnimNodeBodyRetargeter::IsInitialized() const
{
	return InitData.SourceToTargetNameMap && !InitData.SourceToTargetNameMap->IsEmpty();
}

bool FOculusXRAnimNodeBodyRetargeter::UpdateSkeleton(
	const FOculusXRBodyState& BodyState,
	const FBoneContainer& BoneContainer,
	const USkeletalMeshComponent* SkeletalMeshComponent,
	const float WorldScale)
{
	if (BodyState.IsActive && IsInitialized() && SourceReferenceInfo.RequiresUpdate(BodyState.SkeletonChangedCount, BoneContainer.GetSerialNumber()) && OculusXRMovement::GetBodySkeleton(SourceReferenceInfo.SourceReferenceSkeleton, WorldScale))

	{
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
		DebugDrawUtility.ClearDrawQueue(kRestPoseDebugDrawCategory);
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW

		// Store the new serial number and Skeleton Change Count
		SourceReferenceInfo.BoneContainerSerialNumber = BoneContainer.GetSerialNumber();
		SourceReferenceInfo.SourceChangeCount = BodyState.SkeletonChangedCount;

		// We may have new mapping data, so we need to recalculate constants
		TargetToSourceMap = RecalculateMapping(BoneContainer, InitData.SourceToTargetNameMap);

		// Generate the Target Skeleton Rest Pose
		const FOculusXRRetargetSkeletonFCompactPoseBoneIndex TargetSkeletonRestPose = Factory::FromBoneContainer(BoneContainer);

		// Call AFTER Initialize Target Facing Direction
		// We need to determine the TrackingToComponentSpace Transform in that function
		SourceReferenceInfo.SourceSkeleton = Factory::FromOculusXRBodySkeleton(SourceReferenceInfo.SourceReferenceSkeleton, InitData.TrackingSpaceToComponentSpace);

		// Fill TargetSkeletonData
		const TArray<TOculusXRRetargetSkeletonJoint<FCompactPoseBoneIndex>>& TargetJointArray = TargetSkeletonRestPose.GetJointDataArray();

		// Reset our Tracking structures
		TargetAdjustedRestPoseData.PoseData.Empty(TargetJointArray.Num());
		SourceReferenceInfo.SourceToTargetIdxMap.Empty(SourceReferenceInfo.SourceToTargetIdxMap.Num());

		// TODO: T197814847 - Fix to be more bulletproof - Based on an assumption that the Joints are
		// sorted from Parent to Child.  If it isn't, then the Parent Transform/Index may not get calculated before the child.
		for (int i = 0; i < TargetJointArray.Num(); ++i)
		{
			// Child Joint Array will be populated later.
			const EOculusXRBoneID sourceJointID = TargetToSourceMap.Contains(TargetJointArray[i].BoneId) ? TargetToSourceMap[TargetJointArray[i].BoneId] : EOculusXRBoneID::None;
			TargetAdjustedRestPoseData.PoseData.Add({ TargetJointArray[i].BoneId,
				TargetJointArray[i].ParentIdx,
				TargetJointArray[i].LocalTransform,
				TargetJointArray[i].ComponentTransform,
				sourceJointID,
				static_cast<float>(TargetJointArray[i].LocalTransform.GetLocation().Length()) });

			if (sourceJointID != EOculusXRBoneID::None)
			{
				SourceReferenceInfo.SourceToTargetIdxMap.Add(sourceJointID, i);
			}

			// Populate parent child array
			const int ParentIdx = TargetAdjustedRestPoseData.GetParentBoneIndex(i);
			if (ParentIdx != INDEX_NONE)
			{
				TargetAdjustedRestPoseData.PoseData[ParentIdx].childJoints.Add(i);
			}
		}

		// Calculate the Ancestor Indexes
		const EOculusXRBoneID characterRootParent = SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyRoot) ? EOculusXRBoneID::BodyRoot : EOculusXRBoneID::BodyHips;
		if (SourceReferenceInfo.SourceToTargetIdxMap.Contains(characterRootParent))
		{
			// We need to start with the root joint, if it's not mappped, we need to output an error
			CalculateMappedAncestorValues(SourceReferenceInfo, INDEX_NONE, SourceReferenceInfo.SourceToTargetIdxMap[characterRootParent], TargetAdjustedRestPoseData);
		}
		else
		{
			UE_LOG(LogOculusXRRetargeting, Warning, TEXT("Root And Hip Joints are not mapped - T-Pose Alignment will not function correctly."));
		}

		SetTargetToTPose();

		CacheTwistJoints();

		ApplyScaleAndProportion();

		InitializeScaleAndOffsetData();

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
		if (SkeletalMeshComponent && (DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPose || DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPoseWithMapping))

		{
			const FTransform& MeshTransform = SkeletalMeshComponent->GetComponentTransform();
			DebugDrawUtility.AddSkeleton(SourceReferenceInfo.SourceSkeleton, MeshTransform, FColor::Yellow, kRestPoseDebugDrawCategory);
			DebugDrawUtility.AddSkeleton(TargetAdjustedRestPoseData, MeshTransform, FColor::Green, kRestPoseDebugDrawCategory);

			// Draw the Mappings in White
			if (DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPoseWithMapping)
			{
				DebugDrawUtility.AddSkeletonMapping(SourceReferenceInfo.SourceSkeleton, TargetAdjustedRestPoseData, SourceReferenceInfo.SourceToTargetIdxMap,
					MeshTransform, FColor::White, kRestPoseDebugDrawCategory);
			}
		}
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	}
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	else if (!(DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPose || DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPoseWithMapping))

	{
		DebugDrawUtility.ClearDrawQueue(kRestPoseDebugDrawCategory);
	}
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	return SourceReferenceInfo.IsValid();
}

bool FOculusXRAnimNodeBodyRetargeter::ProcessFrameRetargeting(
	const FOculusXRBodyState& BodyState,
	const USkeletalMeshComponent* SkeletalMeshComponent,
	FPoseContext& Output)
{
	// Sanity Check - these should all be valid for this function to execute
	if (!(SkeletalMeshComponent && SourceReferenceInfo.IsValid()))
	{
		return false;
	}

	FCSPose<FCompactPose> MeshPoses;
	MeshPoses.InitPose(Output.Pose);
	TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>> FramePoses;

	FramePoses.Reserve(TargetAdjustedRestPoseData.GetNumBones());

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	// Feature to Retarget to Rest Pose ONLY available in non-shipping builds
	if (DebugPoseMode == EOculusXRBodyDebugPoseMode::RestPose)
	{
		// Slam the Rest Pose into the Target
		for (int iBoneIdx = 0; iBoneIdx < TargetAdjustedRestPoseData.GetNumBones(); ++iBoneIdx)
		{
			const auto& jointEntry = TargetAdjustedRestPoseData.PoseData[iBoneIdx];
			FTransform jointFrameTransform = jointEntry.ComponentTransform;
			FramePoses.Add({ jointEntry.BoneId, jointFrameTransform, jointEntry.componentSpaceScale });
		}

		SourceReferenceInfo.LastFrameBodyState = SourceReferenceInfo.SourceSkeleton;
	}
	else
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	{
		// TODO: T197814847 - Longer Term - Fix to be more bulletproof - Based on an assumption that the Joints are
		// sorted from Parent to Child.  If it isn't, then the Parent Transform/Index may not get calculated before the child.

		// If the BodyState isn't active and we have a valid cached pose from last frame, use it to freeze the character in space
		// until the operations in the OS complete and we get valid data again.
		if (BodyState.IsActive)
		{
			SourceReferenceInfo.LastFrameBodyState = Factory::FromOculusXRBodyState(BodyState,
				SourceReferenceInfo.SourceReferenceSkeleton, InitData.TrackingSpaceToComponentSpace, InitData.RootMotionBehavior);
		}

		for (int iBoneIdx = 0; iBoneIdx < TargetAdjustedRestPoseData.GetNumBones(); ++iBoneIdx)
		{
			const auto& jointEntry = TargetAdjustedRestPoseData.PoseData[iBoneIdx];
			FTransform jointFrameTransform = jointEntry.ComponentTransform;
			if (jointEntry.ParentIdx != INDEX_NONE)
			{
				// Append the Local Transform (We will interpolate Twist Joint Chains later)
				check(jointEntry.ParentIdx < FramePoses.Num());
				const FTransform& parentTransform = FramePoses[jointEntry.ParentIdx].Get<FTransform>();
				jointFrameTransform = jointEntry.LocalTransform * parentTransform;
			}

			const int sourceJointIndex = SourceReferenceInfo.LastFrameBodyState.GetBoneIndex(jointEntry.sourceJointID);
			if (SourceReferenceInfo.LastFrameBodyState.IsValidIndex(sourceJointIndex))
			{
				// If we have a valid Mapping, then Apply the Mapping to Retarget the Joint
				FTransform retargetedJoint = jointEntry.sourceJointLocalOffset * SourceReferenceInfo.LastFrameBodyState.GetComponentTransform(sourceJointIndex);

				if (IsRotationOnlyRetargetingMode(InitData.RetargetingMode))
				{
					if (IsHipOrRootSourceJoint(jointEntry.sourceJointID))
					{
						jointFrameTransform = retargetedJoint;
					}
					else
					{
						jointFrameTransform.SetRotation(retargetedJoint.GetRotation());
					}
				}
				else
				{
					check(IsRotationAndPositionRetargetingMode(InitData.RetargetingMode));
					// If this is a hand joint and our alignment mode is something that doesn't scale the hands
					// then apply rotation only retargeting to the hand joints to avoid scaling them from the
					// hand tracking system.
					const int RightWristBoneIndex = SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyRightHandWrist) ? SourceReferenceInfo.SourceToTargetIdxMap[EOculusXRBoneID::BodyRightHandWrist] : INDEX_NONE;
					const int LeftWristBoneIndex = SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyLeftHandWrist) ? SourceReferenceInfo.SourceToTargetIdxMap[EOculusXRBoneID::BodyLeftHandWrist] : INDEX_NONE;

					if (InitData.RetargetingMode == EOculusXRBodyRetargetingMode::RotationAndPositionsHandsRotationOnly && (iBoneIdx == RightWristBoneIndex || TargetAdjustedRestPoseData.IsAncestorToBoneIndex(RightWristBoneIndex, iBoneIdx) || iBoneIdx == LeftWristBoneIndex || TargetAdjustedRestPoseData.IsAncestorToBoneIndex(LeftWristBoneIndex, iBoneIdx)))
					{
						jointFrameTransform.SetRotation(retargetedJoint.GetRotation());
					}
					else
					{
						// Check to see whether we need to modify the parent orientation due to deformation.
						if (!IsHipOrRootSourceJoint(jointEntry.sourceJointID) && jointEntry.ParentIdx != INDEX_NONE && jointEntry.sourceJointLocalOffset.GetLocation().Length() > 0.0f)
						{
							// Check to see if the parent only has one non-twist child joint.
							// We can rotate it if we're the only child joint that matters.
							const auto& parentJointEntry = TargetAdjustedRestPoseData.PoseData[jointEntry.ParentIdx];
							const int nonTwistChildJointCount = parentJointEntry.GetNonTwistChildJointCount();
							check(nonTwistChildJointCount > 0);
							if (parentJointEntry.GetNonTwistChildJointCount() == 1)
							{
								// We're only doing parent rotation here, then propagating to it's child twist joints.
								// We'll handle the twist joint spacing during the twist joint update below

								// NOTE: The twist joint pass also captures unmapped joints in a chain so that we can apply
								// twist interpolation.  We don't cache those joints or need to handle those in this pass
								// since a qualification of those joints is that they only have a single parent and have a
								// single child that terminates the chain.  There won't be a situation where a sibling joint
								// should/could affect their rotation.

								// Fix the Parent Rotation to re-align with our translated child joint
								FTransform& parentComponentTransform = FramePoses[jointEntry.ParentIdx].Get<FTransform>();
								FVector frameRayToCurrentJoint = retargetedJoint.GetLocation() - parentComponentTransform.GetLocation();
								FVector restPoseRayToCurrentJoint = jointFrameTransform.GetLocation() - parentComponentTransform.GetLocation();
								frameRayToCurrentJoint.Normalize();
								restPoseRayToCurrentJoint.Normalize();

								const FQuat alignmentRotationToApply = FQuat::FindBetween(restPoseRayToCurrentJoint, frameRayToCurrentJoint);
								parentComponentTransform.SetRotation(alignmentRotationToApply * parentComponentTransform.GetRotation());

								// Propagate the rotational change to all siblings on this parent joint that have already been
								// processed.  The pose is processed in hierarchical order, but there is a chance that a
								// sibling (and it's chain) may have been processed prior to this joint.  We can determine
								// if a joint has been processed by comparing it's index against the number of joints in FramePoses.
								for (int iTwistChild : parentJointEntry.childTwistJoints)
								{
									// If we've already processed the child, update it's Frame Transform
									if (iTwistChild < FramePoses.Num())
									{
										FramePoses[iTwistChild].Get<FTransform>() = TargetAdjustedRestPoseData.GetLocalTransform(iTwistChild) * parentComponentTransform;
									}
								}
							}
						}
						jointFrameTransform = retargetedJoint;
					}
				}
			}
			// DO NOT Scale the joints during update, it will affect the child joint calculation from local space in the loop
			FramePoses.Add({ jointEntry.BoneId, jointFrameTransform, jointEntry.componentSpaceScale });
		}
	}

	// Twist Joints
	ProcessFrameInterpolateTwistJoints(FramePoses);

	// Update the hand scale joint scale
	if (InitData.RetargetingMode == EOculusXRBodyRetargetingMode::RotationAndPositions)
	{
		// Rotation and Positions retargeting is the only mode where the hand sizes are changed based on the frame data
		if (SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyLeftHandWrist) && SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyRightHandWrist))
		{
			UpdateScaleForFrame(SourceReferenceInfo.SourceToTargetIdxMap[EOculusXRBoneID::BodyLeftHandWrist], FramePoses);
			UpdateScaleForFrame(SourceReferenceInfo.SourceToTargetIdxMap[EOculusXRBoneID::BodyRightHandWrist], FramePoses);
		}
	}

	// Now Apply the FramePoses to the MeshPoses struct
	for (auto& framePoseEntry : FramePoses)
	{
		// Apply Scale here so it won't affect child transforms
		framePoseEntry.Get<FTransform>().SetScale3D(FVector::OneVector * framePoseEntry.Get<float>());
		MeshPoses.SetComponentSpaceTransform(framePoseEntry.Get<FCompactPoseBoneIndex>(), framePoseEntry.Get<FTransform>());
	}

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	if (DebugDrawMode == EOculusXRBodyDebugDrawMode::FramePose || DebugDrawMode == EOculusXRBodyDebugDrawMode::FramePoseWithMapping)
	{
		const FTransform& MeshTransform = SkeletalMeshComponent->GetComponentTransform();

#if OCULUS_XR_DEBUG_DRAW_MODIFIED_ROOT_MOTION_BEHAVIOR
		if (IsModifiedRootBehavior(InitData.RootMotionBehavior) && BodyState.IsActive)
		{
			FOculusXRRetargetSkeletonEOculusXRBoneID unmodifiedBodyState = Factory::FromOculusXRBodyState(BodyState,
				SourceReferenceInfo.SourceReferenceSkeleton, InitData.TrackingSpaceToComponentSpace, EOculusXRBodyRetargetingRootMotionBehavior::RootFlatTranslationHipRotation);

			DebugDrawUtility.AddSkeleton(unmodifiedBodyState, MeshTransform, FColor::Cyan);
		}
#endif // OCULUS_XR_DEBUG_DRAW_MODIFIED_ROOT_MOTION_BEHAVIOR

		DebugDrawUtility.AddSkeleton(SourceReferenceInfo.LastFrameBodyState, MeshTransform, FColor::Yellow);

		// Calculate the target skeleton for the Frame
		FOculusXRRetargetSkeletonFCompactPoseBoneIndex RetargetedSkeleton = Factory::FromComponentSpaceTransformArray(TargetAdjustedRestPoseData, FramePoses);
		DebugDrawUtility.AddSkeleton(RetargetedSkeleton, MeshTransform, FColor::Green);

		if (DebugDrawMode == EOculusXRBodyDebugDrawMode::FramePoseWithMapping)
		{
			DebugDrawUtility.AddSkeletonMapping(SourceReferenceInfo.LastFrameBodyState,
				RetargetedSkeleton, SourceReferenceInfo.SourceToTargetIdxMap, MeshTransform, FColor::White);
		}
	}
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW

	FCSPose<FCompactPose>::ConvertComponentPosesToLocalPosesSafe(MeshPoses, Output.Pose);
	return true;
}

void FOculusXRAnimNodeBodyRetargeter::ProcessFrameInterpolateTwistJoints(TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses) const
{
	// Interpolate Twist Joints
	for (const auto& twistJointPair : TargetAdjustedRestPoseData.TwistJoints)
	{
		const TwistJointEntry& twistJoint = twistJointPair.Value;

		const FTransform& twistComponentTransform = FramePoses[twistJoint.TargetTwistJointIdx].Get<FTransform>();
		const FTransform& twistParentComponentTranform = FramePoses[twistJoint.TargetTwistParentJointIdx].Get<FTransform>();
		const FTransform& twistSourceComponentTransform = FramePoses[twistJoint.TargetSourceJointIdx].Get<FTransform>();
		const FTransform& twistSourceParentTransform = FramePoses[twistJoint.TargetSourceParentJointIdx].Get<FTransform>();

		// Put the source joint full joint vector in the same local space to our twist joint
		const FVector FrameParentToSourceRayInTargetLocalSpace = twistParentComponentTranform.GetRotation().Inverse() * (twistSourceComponentTransform.GetLocation() - twistSourceParentTransform.GetLocation());

		// Calculate how much the joint translation has scaled
		const float translationScalar = twistJoint.RestPoseParentToSourceJointLength > 0.0f ? FrameParentToSourceRayInTargetLocalSpace.Length() / twistJoint.RestPoseParentToSourceJointLength : 0.0f;
		const FVector localTranslationToSubtract = (1.0f - translationScalar) * twistJoint.ProjectedSourceJointAlignmentLocalSpace;

		// Put the source joint in local space to our twist joint
		const FTransform twistSourceLocalTransform = twistSourceComponentTransform.GetRelativeTransform(twistComponentTransform);

		const FQuat targetLocalRotation = twistSourceLocalTransform.GetRotation() * twistJoint.TargetSourceLocalRotationOffset;
		const FTransform& twistLocalTransform = TargetAdjustedRestPoseData.GetLocalTransform(twistJoint.TargetTwistJointIdx);

		// Adjust the rotation so it only rotates along the axis of the joint relative to it's parent
		const FVector twistJointReferenceLocalDirection = twistLocalTransform.GetRotation().RotateVector(twistLocalTransform.GetLocation());
		const FVector twistJointTargetLocalDirection = targetLocalRotation.RotateVector(twistLocalTransform.GetLocation());

		// Slerp from our current rotation to our target rotation based on the weight determined during twist joint detection
		const FQuat adjustedTargetLocalRotation =
			FQuat::Slerp(
				twistLocalTransform.GetRotation(),
				FQuat::FindBetween(twistJointTargetLocalDirection, twistJointReferenceLocalDirection) * targetLocalRotation,
				twistJoint.weight);

		// Update our frame pose to reflect the twisted rotation
		FramePoses[twistJoint.TargetTwistJointIdx].Get<FTransform>() = FTransform(adjustedTargetLocalRotation, twistLocalTransform.GetLocation() - localTranslationToSubtract) * twistParentComponentTranform;
	}
}

void FOculusXRAnimNodeBodyRetargeter::UpdateScaleForFrame(const int TargetIndex, TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses) const
{
	if (TargetIndex != INDEX_NONE)
	{
		TTuple<float, float> targetIndexTotalJointLengths = GetFrameMaxCurrentAndUnModifiedJointLengths(TargetIndex, FramePoses);
		const float Scale = targetIndexTotalJointLengths.Value > 0.0f ? targetIndexTotalJointLengths.Key / targetIndexTotalJointLengths.Value : TargetAdjustedRestPoseData.GlobalComponentSpaceScale;
		UpdateScaleForFrameRecursive(TargetIndex, Scale, FramePoses);
	}
}

void FOculusXRAnimNodeBodyRetargeter::UpdateScaleForFrameRecursive(const int TargetIndex, const float scale, TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses) const
{
	if (TargetIndex != INDEX_NONE)
	{
		FramePoses[TargetIndex].Get<float>() = scale;
		const TargetSkeletonJointEntry& jointEntry = TargetAdjustedRestPoseData.PoseData[TargetIndex];
		for (int childIdx : jointEntry.childJoints)
		{
			UpdateScaleForFrameRecursive(childIdx, scale, FramePoses);
		}
	}
}

TTuple<float, float> FOculusXRAnimNodeBodyRetargeter::GetFrameMaxCurrentAndUnModifiedJointLengths(int targetJointIndex, const TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses, float currentLength, float unmodifiedLength) const
{
	TTuple<float, float> retVal({ currentLength, unmodifiedLength });
	if (targetJointIndex != INDEX_NONE)
	{
		const TargetSkeletonJointEntry& jointEntry = TargetAdjustedRestPoseData.PoseData[targetJointIndex];
		if (!jointEntry.childJoints.IsEmpty())
		{
			const FVector parentJointPosition = FramePoses[targetJointIndex].Get<FTransform>().GetLocation();
			for (int childIdx : jointEntry.childJoints)
			{
				const TargetSkeletonJointEntry& childJointEntry = TargetAdjustedRestPoseData.PoseData[childIdx];
				const float childUnmodifiedLength = childJointEntry.unmodifiedJointLength;
				const float childCurrentLength = (FramePoses[childIdx].Get<FTransform>().GetLocation() - parentJointPosition).Length();

				TTuple<float, float> childLengths = GetFrameMaxCurrentAndUnModifiedJointLengths(childIdx, FramePoses, currentLength + childCurrentLength, unmodifiedLength + childUnmodifiedLength);
				if (childLengths.Key > retVal.Key)
				{
					retVal = childLengths;
				}
			}
		}
	}

	return retVal;
}

bool FOculusXRAnimNodeBodyRetargeter::RetargetFromBodyState(
	const FOculusXRBodyState& BodyState,
	const USkeletalMeshComponent* SkeletalMeshComponent,
	const float WorldScale,
	FPoseContext& Output)
{
	if (SkeletalMeshComponent && UpdateSkeleton(BodyState, Output.Pose.GetBoneContainer(), SkeletalMeshComponent, WorldScale))
	{
		return ProcessFrameRetargeting(BodyState, SkeletalMeshComponent, Output);
	}
	return false;
}

void FOculusXRAnimNodeBodyRetargeter::SetTargetToTPose()
{
	TMap<int, TArray<int>> AncestorToChildTPoseAlignmentMap;
	TSet<int> MappedAdjustableJointsWithNoAdjustableChildren;
	TSet<int> SpecialCaseRotationBehaviorAncestors;

	// Iterate the newly generated list and generate the Child Joint Arrays
	for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
	{
		if (TargetAdjustedRestPoseData.GetParentBoneIndex(i) != INDEX_NONE && TargetAdjustedRestPoseData.GetMappedAncestorIndex(i) != INDEX_NONE && kTPOSE_ADJUSTABLE_JOINT_SET.Contains(TargetAdjustedRestPoseData.GetSourceJointID(i)))
		{
			int TargetMappedAncestorIndex = TargetAdjustedRestPoseData.GetMappedAncestorIndex(i);
			const EOculusXRBoneID sourceJointID = TargetAdjustedRestPoseData.GetSourceJointID(i);
			int SourceJointIdx = SourceReferenceInfo.SourceSkeleton.GetBoneIndex(sourceJointID);
			int SourceAncestorIdx = SourceReferenceInfo.SourceSkeleton.GetBoneIndex(
				TargetAdjustedRestPoseData.GetSourceJointID(TargetMappedAncestorIndex));
			if (SourceJointIdx != INDEX_NONE)
			{
				if (SourceAncestorIdx != INDEX_NONE)
				{
					if (kTPOSE_SPECIAL_HANDLING_ADJUSTABLE_HAND_JOINTS.Contains(sourceJointID) && TargetAdjustedRestPoseData.GetChildJointCount(TargetMappedAncestorIndex) == 1)
					{
						// Hack - this is specific for proximital finger joints
						// it only works for these finger joints with a single child.
						// Just skip if we have a non-conforming target skeleton
						SpecialCaseRotationBehaviorAncestors.Add(TargetMappedAncestorIndex);
					}
					if (AncestorToChildTPoseAlignmentMap.Contains(TargetMappedAncestorIndex))
					{
						AncestorToChildTPoseAlignmentMap[TargetMappedAncestorIndex].Add(i);
					}
					else
					{
						AncestorToChildTPoseAlignmentMap.Add(TargetMappedAncestorIndex, { i });
					}
				}

				// Special case to catch the finger tips - Check if the joint has any mapped children
				if (TargetAdjustedRestPoseData.IsValidIndex(TargetAdjustedRestPoseData.GetParentBoneIndex(i)) && TargetAdjustedRestPoseData.FindNextChildJointMappedToSource(i) == INDEX_NONE)
				{
					MappedAdjustableJointsWithNoAdjustableChildren.Add(i);
				}
			}
		}
	}

	// Set Joints to T-Pose using the TargetAdjustedRestPoseData.PoseData Map we generated
	for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
	{
		if (!AncestorToChildTPoseAlignmentMap.Contains(i) && !MappedAdjustableJointsWithNoAdjustableChildren.Contains(i))
		{
			continue;
		}

		FTransform& TargetJointTransform = TargetAdjustedRestPoseData.PoseData[i].ComponentTransform;

		if (AncestorToChildTPoseAlignmentMap.Contains(i))
		{
			if (SpecialCaseRotationBehaviorAncestors.Contains(i))
			{
				// TODO: For now, just skip these joints - revisit to see if we
				// can get better alignment of the hands
				continue;
			}
			const EOculusXRBoneID sourceJointID = TargetAdjustedRestPoseData.GetSourceJointID(i);
			int SourceJointIdx = SourceReferenceInfo.SourceSkeleton.GetBoneIndex(TargetAdjustedRestPoseData.GetSourceJointID(i));

			const FTransform& SourceJointTransform = SourceReferenceInfo.SourceSkeleton.GetComponentTransform(SourceJointIdx);

			FVector sourceAlignment = FVector::ZeroVector;
			FVector targetAlignment = FVector::ZeroVector;

			FVector sourceTwistAlignment = FVector::ZeroVector;
			FVector targetTwistAlignment = FVector::ZeroVector;

			FVector lastSourceChildLocation = FVector::ZeroVector;
			FVector lastTargetChildLocation = FVector::ZeroVector;

			const TArray<int>& AncestorChildIndexArray = AncestorToChildTPoseAlignmentMap[i];
			for (int iChild = 0; iChild < AncestorChildIndexArray.Num(); iChild++)
			{
				const int TargetChildIdx = AncestorChildIndexArray[iChild];
				int SourceChildIdx = SourceReferenceInfo.SourceSkeleton.GetBoneIndex(TargetAdjustedRestPoseData.GetSourceJointID(TargetChildIdx));
				check(SourceJointIdx != INDEX_NONE && SourceChildIdx != INDEX_NONE);

				const FTransform& SourceChildTransform = SourceReferenceInfo.SourceSkeleton.GetComponentTransform(SourceChildIdx);
				const FTransform& TargetChildTransform = TargetAdjustedRestPoseData.GetComponentTransform(TargetChildIdx);

				if (iChild > 0)
				{
					sourceTwistAlignment = sourceTwistAlignment + (SourceChildTransform.GetLocation() - lastSourceChildLocation);
					targetTwistAlignment = targetTwistAlignment + (TargetChildTransform.GetLocation() - lastTargetChildLocation);
				}

				sourceAlignment = sourceAlignment + (SourceChildTransform.GetLocation() - SourceJointTransform.GetLocation());
				targetAlignment = targetAlignment + (TargetChildTransform.GetLocation() - TargetJointTransform.GetLocation());

				lastSourceChildLocation = SourceChildTransform.GetLocation();
				lastTargetChildLocation = TargetChildTransform.GetLocation();
			}

			// If we have more than one child, twist to align more with the source as calculated
			// Note - this is primarily for wrist joints.
			if (AncestorChildIndexArray.Num() > 0)
			{
				FQuat TwistRotationToApply = FQuat::FindBetween(targetTwistAlignment, sourceTwistAlignment);

				FQuat TargetTwistAdjustedRotation = TwistRotationToApply * TargetJointTransform.GetRotation();
				TargetTwistAdjustedRotation.Normalize();
				TargetJointTransform.SetRotation(TargetTwistAdjustedRotation);
			}

			FQuat RotationToApply = FQuat::FindBetween(targetAlignment, sourceAlignment);

			// Rotate the Target by the Delta
			FQuat TargetNewRotation = RotationToApply * TargetJointTransform.GetRotation();
			TargetNewRotation.Normalize();
			TargetJointTransform.SetRotation(TargetNewRotation);
		}
		else
		{
			// This is a terminating mapped joint (like a finger tip, etc)
			// Use it's parent joint as it's rotation (resolves mappings where the finger tip doesn't have a non-rendering joint)
			check(MappedAdjustableJointsWithNoAdjustableChildren.Contains(i));
			TargetJointTransform.SetRotation(TargetAdjustedRestPoseData.GetComponentTransform(TargetAdjustedRestPoseData.GetParentBoneIndex(i)).GetRotation());
		}

		// Update the Local Transform
		const int ParentIdx = TargetAdjustedRestPoseData.GetParentBoneIndex(i);
		if (ParentIdx == INDEX_NONE)
		{
			TargetAdjustedRestPoseData.PoseData[i].LocalTransform = TargetJointTransform;
			TargetAdjustedRestPoseData.PoseData[i].ComponentTransform = TargetJointTransform;
		}
		else
		{
			TargetAdjustedRestPoseData.PoseData[i].LocalTransform =
				TargetJointTransform.GetRelativeTransform(TargetAdjustedRestPoseData.GetComponentTransform(ParentIdx));
		}

		// Update all the children from the Ancestor after we've made the adjustment
		UpdateAllChildrenComponentFromLocalRecursive(TargetAdjustedRestPoseData.PoseData, ParentIdx != INDEX_NONE ? ParentIdx : i);
	}
}

// Captures unmapped joints that have mapped joints on both ends of the chain.
// Retargeter will treat as twist joints to interpolate motion (ie - unmapped spine joints, etc)
// If the chain ends with no terminating child mapping, it will be skipped.
// Chains terminate at joints with multiple children, if this joint is unmapped, the chain will be skipped.
void FOculusXRAnimNodeBodyRetargeter::CaptureUnmappedJointChainRecursive(const TArray<TargetSkeletonJointEntry>& jointData, const int jointIdx, TArray<int>& outLinkArray)
{
	outLinkArray.Add(jointIdx);
	const TargetSkeletonJointEntry& jointEntry = jointData[jointIdx];
	for (int childJointIdx : jointEntry.childJoints)
	{
		if (jointData[childJointIdx].sourceJointID != EOculusXRBoneID::None)
		{
			// We've found our terminating joint for the chain
			outLinkArray.Add(childJointIdx);
			return;
		}
	}
	// We didn't find a terminating joint, if we only have a single child joint
	// recursively generate the joint chain.
	if (jointEntry.childJoints.Num() == 1)
	{
		CaptureUnmappedJointChainRecursive(jointData, jointEntry.childJoints[0], outLinkArray);
	}
}

void FOculusXRAnimNodeBodyRetargeter::CacheTwistJoints()
{
	check(!TargetAdjustedRestPoseData.IsEmpty());
	TargetAdjustedRestPoseData.TwistJoints.Empty();

	for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
	{
		if (TargetAdjustedRestPoseData.IsJointMappedToSource(i))
		{
			// First, look for unmapped joints in a chain between two mapped joints
			TArray<int> jointLinkArray;
			CaptureUnmappedJointChainRecursive(TargetAdjustedRestPoseData.PoseData, i, jointLinkArray);

			if (jointLinkArray.Num() > 2 && TargetAdjustedRestPoseData.IsJointMappedToSource(jointLinkArray.Last()))
			{
				// Now Add the joints (Skip the first and Last Array entries as they are mapped joints)
				const int chainTerminatingJointIdx = jointLinkArray.Last();
				const FTransform& chainTerminatingJointComponentTransform = TargetAdjustedRestPoseData.GetComponentTransform(chainTerminatingJointIdx);
				const FTransform& chainStartJointComponentTransform = TargetAdjustedRestPoseData.GetComponentTransform(jointLinkArray[0]);
				const FVector chainComponentRay = chainTerminatingJointComponentTransform.GetLocation() - chainStartJointComponentTransform.GetLocation();
				const float chainRayLength = chainComponentRay.Length();
				for (int iChainIdx = 1; iChainIdx < jointLinkArray.Num() - 1; ++iChainIdx)
				{
					const int linkJointIdx = jointLinkArray[iChainIdx];
					const TargetSkeletonJointEntry& linkJointEntry = TargetAdjustedRestPoseData.PoseData[linkJointIdx];
					const FTransform& linkComponentTransform = linkJointEntry.ComponentTransform;

					// Calculate the weight by projecting our joint to the ray between the start of the chain and the end
					const FVector linkComponentRay = linkComponentTransform.GetLocation() - chainStartJointComponentTransform.GetLocation();
					const float linkProjectionLengthFromStartJoint = linkComponentRay.Dot(chainComponentRay) / chainRayLength;
					const float weight = linkProjectionLengthFromStartJoint / chainRayLength;

					// Convert this ray to the same space as the chainJoint Transform using the parent joint
					const FVector projectedRayToTerminatingJointLocalSpace = TargetAdjustedRestPoseData.GetComponentTransform(linkJointEntry.ParentIdx).GetRotation().Inverse() * chainComponentRay * weight;

					TargetAdjustedRestPoseData.TwistJoints.Emplace(linkJointIdx, { chainTerminatingJointIdx, linkJointIdx, linkJointEntry.ParentIdx, jointLinkArray[0], chainTerminatingJointComponentTransform.GetRelativeTransform(linkComponentTransform).GetRotation(), projectedRayToTerminatingJointLocalSpace, chainRayLength, linkProjectionLengthFromStartJoint, weight, false });
				}
			}

			// Now look for Twist Joints
			TargetSkeletonJointEntry& parentJointEntry = TargetAdjustedRestPoseData.PoseData[i];
			if (parentJointEntry.childJoints.Num() > 1)
			{
				// Twist joints should all share the same common parent.
				// They should have a mapped sibling and be in line with the sibling relative to the parent
				TArray<int> possibleTwistJoints;
				TArray<int> possibleTerminatingJoints;

				for (int childJointIdx : parentJointEntry.childJoints)
				{
					if (TargetAdjustedRestPoseData.IsJointMappedToSource(childJointIdx))
					{
						// Joints that are mapped may possibly drive a twist joint
						possibleTerminatingJoints.Add(childJointIdx);
					}
					else
					{
						// This might be a twist joint, will validate below
						possibleTwistJoints.Add(childJointIdx);
					}
				}

				// If we have possible twist joints and possible terminating joints, iterate the list
				if (!possibleTwistJoints.IsEmpty() && !possibleTerminatingJoints.IsEmpty())
				{
					for (int twistJointIdx : possibleTwistJoints)
					{
						for (int terminatingJointIdx : possibleTerminatingJoints)
						{
							const FVector rayToTerminatingJoint = TargetAdjustedRestPoseData.GetLocalTransform(terminatingJointIdx).GetLocation();
							const float distanceToTerminatingJoint = rayToTerminatingJoint.Length();
							const FVector rayToTwistJoint = TargetAdjustedRestPoseData.GetLocalTransform(twistJointIdx).GetLocation();
							const float distanceToTwistJoint = rayToTwistJoint.Length();
							const float twistRayToTerminatingDot = rayToTwistJoint.Dot(rayToTerminatingJoint);
							const float twistProjectionLength = twistRayToTerminatingDot / distanceToTerminatingJoint;
							const float weight = twistProjectionLength / distanceToTerminatingJoint;

							// Twist Joint should always be closer to the parent than the terminating joint
							if (weight <= 1.0f)
							{
								// Check the angle between the rays
								const float angleCheck = FMath::Acos(twistRayToTerminatingDot / (rayToTwistJoint.Length() * distanceToTerminatingJoint));
								// Rotateable joints are within 2 degrees of the ray between the parent and the terminating joint
								const bool isRotateableJoint = angleCheck <= kTWIST_JOINT_MIN_ANGLE_THRESHOLD;
								const bool MapContainsTwistJoint = TargetAdjustedRestPoseData.TwistJoints.Contains(twistJointIdx);
								// For the Rare Case that we have a rig with two possible terminating joints and this joint
								// is "rotateable" ie. in line with the joint, overwrite the record for this twist joint
								if (isRotateableJoint || !MapContainsTwistJoint)
								{
									const FTransform terminatiingJointLocalToTwistJoint =
										TargetAdjustedRestPoseData.GetComponentTransform(terminatingJointIdx).GetRelativeTransform(TargetAdjustedRestPoseData.GetComponentTransform(twistJointIdx));

									// Avoid double additions if we're in the rare case
									// of overwriting the non-rotateable record.
									if (!MapContainsTwistJoint)
									{
										parentJointEntry.childTwistJoints.Add(twistJointIdx);
									}
									// We've located a twist joint (NOTE: Emplace should overwrite if the key already exists)
									TargetAdjustedRestPoseData.TwistJoints.Emplace(twistJointIdx, { terminatingJointIdx, twistJointIdx, i, i, terminatiingJointLocalToTwistJoint.GetRotation().Inverse(), rayToTerminatingJoint * weight, distanceToTerminatingJoint, twistProjectionLength, weight, isRotateableJoint });
								}
							}
						}
					}
				}
			}
		}
	}
}

void FOculusXRAnimNodeBodyRetargeter::ApplyScaleAndProportion()
{
	// Reset Global Component Scale
	TargetAdjustedRestPoseData.GlobalComponentSpaceScale = 1.0f;

	if (InitData.RetargetingMode != EOculusXRBodyRetargetingMode::RotationOnlyNoScaling)
	{
		// Use the Right Wrist for Alignment
		const EOculusXRBoneID rightWristJoint = EOculusXRBoneID::BodyRightHandWrist;
		const EOculusXRBoneID leftWristJoint = EOculusXRBoneID::BodyLeftHandWrist;

		if (SourceReferenceInfo.SourceToTargetIdxMap.Contains(rightWristJoint) && SourceReferenceInfo.SourceSkeleton.IsValid(rightWristJoint) && SourceReferenceInfo.SourceToTargetIdxMap.Contains(leftWristJoint) && SourceReferenceInfo.SourceSkeleton.IsValid(leftWristJoint))

		{
			const TargetSkeletonJointEntry& rightWristTargetJoint =
				TargetAdjustedRestPoseData.PoseData[SourceReferenceInfo.SourceToTargetIdxMap[rightWristJoint]];
			check(rightWristTargetJoint.sourceJointID == rightWristJoint);
			const FTransform& SourceRightWrist = SourceReferenceInfo.SourceSkeleton.GetComponentTransform(SourceReferenceInfo.SourceSkeleton.GetBoneIndex(rightWristJoint));
			const FTransform& TargetRightWrist = TargetAdjustedRestPoseData.GetComponentTransform(SourceReferenceInfo.SourceToTargetIdxMap[rightWristJoint]);
			const FTransform& TargetLeftWrist = TargetAdjustedRestPoseData.GetComponentTransform(SourceReferenceInfo.SourceToTargetIdxMap[leftWristJoint]);

			const FVector ForwardDirection = InitData.TargetFacingTransform.GetRotation().GetAxisX();
			const FVector RightDirection = InitData.TargetFacingTransform.GetRotation().GetAxisY();
			const FVector UpDirection = InitData.TargetFacingTransform.GetRotation().GetAxisZ();

			const float TargetWristHeight = (TargetRightWrist.GetLocation() * UpDirection).Length();
			TargetAdjustedRestPoseData.GlobalComponentSpaceScale = (SourceRightWrist.GetLocation() * UpDirection).Length() / (TargetWristHeight > 0.0f ? TargetWristHeight : 1.0f);

			// Scale the entire character based on the ratio for the height
			for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
			{
				TargetAdjustedRestPoseData.PoseData[i].ComponentTransform.SetLocation(
					TargetAdjustedRestPoseData.GetComponentTransform(i).GetLocation() * TargetAdjustedRestPoseData.GlobalComponentSpaceScale);
			}

			// Shift the entire Model against the Forward Vector to align the Wrists
			const FVector Offset =
				(SourceRightWrist.GetLocation() - rightWristTargetJoint.ComponentTransform.GetLocation()) * FVector(FMath::Abs(ForwardDirection.X), FMath::Abs(ForwardDirection.Y), FMath::Abs(ForwardDirection.Z));

			for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
			{
				FVector NewLocation = TargetAdjustedRestPoseData.GetComponentTransform(i).GetLocation() + Offset;
				TargetAdjustedRestPoseData.PoseData[i].ComponentTransform.SetLocation(NewLocation);
			}

			// Now Scale the Arms Towards the Body to Align the Wrists
			const EOculusXRBoneID rightShoulderJoint = EOculusXRBoneID::BodyRightShoulder;
			const EOculusXRBoneID leftShoulderJoint = EOculusXRBoneID::BodyLeftShoulder;

			if (
				IsRotationAndPositionRetargetingMode(InitData.RetargetingMode) && SourceReferenceInfo.SourceToTargetIdxMap.Contains(rightShoulderJoint) && SourceReferenceInfo.SourceSkeleton.IsValid(rightShoulderJoint) && SourceReferenceInfo.SourceToTargetIdxMap.Contains(leftShoulderJoint) && SourceReferenceInfo.SourceSkeleton.IsValid(leftShoulderJoint))

			{
				// Right Arm
				const float sourceRightWristAbsValue = (SourceRightWrist.GetLocation() * RightDirection).Length();
				const float targetRightWristAbsValue = (TargetRightWrist.GetLocation() * RightDirection).Length();
				const float RightArmScaleRatio = sourceRightWristAbsValue / (targetRightWristAbsValue > 0.0f ? targetRightWristAbsValue : 1.0f);
				const FVector rightArmScalar(RightArmScaleRatio, 1.0f, 1.0f);

				// Left Arm
				const FTransform& SourceLeftWrist = SourceReferenceInfo.SourceSkeleton.GetComponentTransform(SourceReferenceInfo.SourceSkeleton.GetBoneIndex(leftWristJoint));

				const float sourceLeftWristAbsValue = (SourceLeftWrist.GetLocation() * RightDirection).Length();
				const float targetLeftWristAbsValue = (TargetLeftWrist.GetLocation() * RightDirection).Length();
				const float LeftArmScaleRatio = sourceLeftWristAbsValue / (targetLeftWristAbsValue > 0.0f ? targetLeftWristAbsValue : 1.0f);
				const FVector leftArmScalar(LeftArmScaleRatio, 1.0f, 1.0f);

				// Scale the Right Arm
				ScaleAllChildrenComponentSpaceRecursive(TargetAdjustedRestPoseData.PoseData, SourceReferenceInfo.SourceToTargetIdxMap[rightShoulderJoint], rightArmScalar);

				// Scale the Left Arm
				ScaleAllChildrenComponentSpaceRecursive(TargetAdjustedRestPoseData.PoseData, SourceReferenceInfo.SourceToTargetIdxMap[leftShoulderJoint], leftArmScalar);

				// Last Scale/Align the Fingers
				if (InitData.RetargetingMode == EOculusXRBodyRetargetingMode::RotationAndPositions)
				{
					// TODO: Improve this Alignment code to better respect the target rig hand proportions

					// NOTE: Assumption is that order of Array is from Parent -> Child
					TSet<int> AdjustedParentJoints;
					AdjustedParentJoints.Reserve(kALIGNABLE_HAND_JOINTS.Num());
					for (EOculusXRBoneID fingerSourceJoint : kALIGNABLE_HAND_JOINTS)
					{
						if (SourceReferenceInfo.SourceToTargetIdxMap.Contains(fingerSourceJoint) && SourceReferenceInfo.SourceSkeleton.IsValid(fingerSourceJoint))
						{
							const FTransform& SourceFingerTransform =
								SourceReferenceInfo.SourceSkeleton.GetComponentTransform(SourceReferenceInfo.SourceSkeleton.GetBoneIndex(fingerSourceJoint));
							const int fingerIndex = SourceReferenceInfo.SourceToTargetIdxMap[fingerSourceJoint];
							TargetSkeletonJointEntry& fingerToAdjust = TargetAdjustedRestPoseData.PoseData[fingerIndex];
							check(fingerToAdjust.sourceJointID == fingerSourceJoint);

							// Fingers should always have a parent joint - check added, but also protected in case someone maps
							// incorrectly
							check(fingerToAdjust.ParentIdx != INDEX_NONE);
							if (fingerToAdjust.ParentIdx != INDEX_NONE)
							{
								TargetSkeletonJointEntry& fingerParentJoint = TargetAdjustedRestPoseData.PoseData[fingerToAdjust.ParentIdx];

								// We only want to rotate a parent joint once
								if (!AdjustedParentJoints.Contains(fingerToAdjust.ParentIdx))
								{
									// Add the index so we won't rotate it again
									AdjustedParentJoints.Add(fingerToAdjust.ParentIdx);

									// Only Rotate the parent if we are the first child
									const FVector currentRayToJoint = fingerToAdjust.ComponentTransform.GetLocation() - fingerParentJoint.ComponentTransform.GetLocation();
									const FVector targetRayToJoint = SourceFingerTransform.GetLocation() - fingerParentJoint.ComponentTransform.GetLocation();

									// Update our Joint's local location length
									FVector fingerCurrentLocalOffset = fingerToAdjust.LocalTransform.GetLocation();
									fingerCurrentLocalOffset.Normalize();
									fingerToAdjust.LocalTransform.SetLocation(fingerCurrentLocalOffset * targetRayToJoint.Length());

									// Apply the rotation to our parent
									const FQuat ParentRotationToApply = FQuat::FindBetween(currentRayToJoint, targetRayToJoint);
									FQuat NewParentComponentRotation = ParentRotationToApply * fingerParentJoint.ComponentTransform.GetRotation();
									NewParentComponentRotation.Normalize();
									fingerParentJoint.ComponentTransform.SetRotation(NewParentComponentRotation);
									if (fingerParentJoint.ParentIdx == INDEX_NONE)
									{
										fingerParentJoint.LocalTransform = fingerParentJoint.ComponentTransform;
									}
									else
									{
										fingerParentJoint.LocalTransform = fingerParentJoint.ComponentTransform.GetRelativeTransform(
											TargetAdjustedRestPoseData.GetComponentTransform(fingerParentJoint.ParentIdx));
									}
									UpdateAllChildrenComponentFromLocalRecursive(TargetAdjustedRestPoseData.PoseData, fingerToAdjust.ParentIdx);
								}
								else
								{
									// Update our position, the update the hierarchy
									fingerToAdjust.ComponentTransform.SetLocation(SourceFingerTransform.GetLocation());
									fingerToAdjust.LocalTransform = fingerToAdjust.ComponentTransform.GetRelativeTransform(fingerParentJoint.ComponentTransform);
									UpdateAllChildrenComponentFromLocalRecursive(TargetAdjustedRestPoseData.PoseData, fingerIndex);
								}
							}
						}
					}
				}
			}

			// Update the Local Transforms
			for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
			{
				const int ParentIdx = TargetAdjustedRestPoseData.GetParentBoneIndex(i);
				if (ParentIdx == INDEX_NONE)
				{
					TargetAdjustedRestPoseData.PoseData[i].LocalTransform = TargetAdjustedRestPoseData.GetComponentTransform(i);
				}
				else
				{
					TargetAdjustedRestPoseData.PoseData[i].LocalTransform =
						TargetAdjustedRestPoseData.GetComponentTransform(i).GetRelativeTransform(TargetAdjustedRestPoseData.GetComponentTransform(ParentIdx));
				}
			}
		}
	}
}

void FOculusXRAnimNodeBodyRetargeter::InitializeScaleAndOffsetData()
{
	// Scale and unmodified joint length are calculated and stored.
	// This will require some testing - scale may need to be applied to the parent joint,
	// not the child joint.  This data should be accessible from the parent using the
	// child joint array stored in each joint record.

	// Calculate Hand Scale
	const int RightWristBoneIndex = SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyRightHandWrist) ? SourceReferenceInfo.SourceToTargetIdxMap[EOculusXRBoneID::BodyRightHandWrist] : INDEX_NONE;
	const int LeftWristBoneIndex = SourceReferenceInfo.SourceToTargetIdxMap.Contains(EOculusXRBoneID::BodyLeftHandWrist) ? SourceReferenceInfo.SourceToTargetIdxMap[EOculusXRBoneID::BodyLeftHandWrist] : INDEX_NONE;

	const TTuple<float, float> RightHandLengths = GetMaxCurrentAndUnModifiedJointLengths(RightWristBoneIndex);
	const TTuple<float, float> LeftHandLengths = GetMaxCurrentAndUnModifiedJointLengths(LeftWristBoneIndex);

	const float RightHandScale = RightHandLengths.Value > 0.0f ? RightHandLengths.Key / RightHandLengths.Value : TargetAdjustedRestPoseData.GlobalComponentSpaceScale;
	const float LeftHandScale = LeftHandLengths.Value > 0.0f ? LeftHandLengths.Key / LeftHandLengths.Value : TargetAdjustedRestPoseData.GlobalComponentSpaceScale;

	for (int i = 0; i < TargetAdjustedRestPoseData.GetNumBones(); ++i)
	{
		TargetSkeletonJointEntry& jointEntry = TargetAdjustedRestPoseData.PoseData[i];
		jointEntry.componentSpaceScale = TargetAdjustedRestPoseData.GlobalComponentSpaceScale;

		// If this is a wrist or Hand, scale to the hand scale
		if (i == RightWristBoneIndex || TargetAdjustedRestPoseData.IsAncestorToBoneIndex(RightWristBoneIndex, i))
		{
			jointEntry.componentSpaceScale = RightHandScale;
		}
		else if (i == LeftWristBoneIndex || TargetAdjustedRestPoseData.IsAncestorToBoneIndex(LeftWristBoneIndex, i))
		{
			jointEntry.componentSpaceScale = LeftHandScale;
		}
		else if (!IsHipOrRootSourceJoint(jointEntry.sourceJointID) && !jointEntry.childJoints.IsEmpty())
		{
			float totalUnmodifiedJointLength = 0.0f;
			float totalCurrentJointLength = 0.0f;
			for (int childJointIdx : jointEntry.childJoints)
			{
				const TargetSkeletonJointEntry& childJointEntry = TargetAdjustedRestPoseData.PoseData[childJointIdx];
				totalUnmodifiedJointLength += childJointEntry.unmodifiedJointLength;
				totalCurrentJointLength += childJointEntry.LocalTransform.GetLocation().Length();
			}

			if (totalUnmodifiedJointLength > 0.0f)
			{
				jointEntry.componentSpaceScale = totalCurrentJointLength / totalUnmodifiedJointLength;
			}
		}

		if (TargetAdjustedRestPoseData.IsJointMappedToSource(i))
		{
			int SourceJointIdx = SourceReferenceInfo.SourceSkeleton.GetBoneIndex(TargetAdjustedRestPoseData.GetSourceJointID(i));
			if (SourceJointIdx != INDEX_NONE)
			{
				const FTransform& SourceJoint = SourceReferenceInfo.SourceSkeleton.GetComponentTransform(SourceJointIdx);
				const FTransform& TargetJoint = TargetAdjustedRestPoseData.GetComponentTransform(i);

				const FQuat SourceJointInvRot = SourceJoint.GetRotation().Inverse();
				const FQuat sourceToTargetRotation = SourceJointInvRot * TargetJoint.GetRotation();
				FVector sourceToTargetOffset = SourceJointInvRot * (TargetJoint.GetLocation() - SourceJoint.GetLocation());

				jointEntry.sourceJointLocalOffset = FTransform(sourceToTargetRotation, sourceToTargetOffset);
			}
		}
	}
}

TTuple<float, float> FOculusXRAnimNodeBodyRetargeter::GetMaxCurrentAndUnModifiedJointLengths(int targetJointIndex, float currentLength, float unmodifiedLength) const
{
	TTuple<float, float> retVal({ currentLength, unmodifiedLength });
	if (targetJointIndex != INDEX_NONE)
	{
		const TargetSkeletonJointEntry& jointEntry = TargetAdjustedRestPoseData.PoseData[targetJointIndex];
		if (!jointEntry.childJoints.IsEmpty())
		{
			for (int childIdx : jointEntry.childJoints)
			{
				const TargetSkeletonJointEntry& childJointEntry = TargetAdjustedRestPoseData.PoseData[childIdx];
				const float childUnmodifiedLength = childJointEntry.unmodifiedJointLength;
				const float childCurrentLength = childJointEntry.LocalTransform.GetLocation().Length();

				TTuple<float, float> childLengths = GetMaxCurrentAndUnModifiedJointLengths(childIdx, currentLength + childCurrentLength, unmodifiedLength + childUnmodifiedLength);
				if (childLengths.Key > retVal.Key)
				{
					retVal = childLengths;
				}
			}
		}
	}

	return retVal;
}

void FOculusXRAnimNodeBodyRetargeter::UpdateAllChildrenComponentFromLocalRecursive(TArray<TargetSkeletonJointEntry>& jointData, const int rootJointIdx)
{
	for (int childJointIdx : jointData[rootJointIdx].childJoints)
	{
		jointData[childJointIdx].ComponentTransform = jointData[childJointIdx].LocalTransform * jointData[rootJointIdx].ComponentTransform;
		UpdateAllChildrenComponentFromLocalRecursive(jointData, childJointIdx);
	}
}

void FOculusXRAnimNodeBodyRetargeter::ScaleAllChildrenComponentSpaceRecursive(TArray<TargetSkeletonJointEntry>& jointData, const int rootJointIdx, const FVector& scalar)
{
	for (int childJointIdx : jointData[rootJointIdx].childJoints)
	{
		// Scale the Child Joint
		jointData[childJointIdx].ComponentTransform.SetLocation(jointData[childJointIdx].ComponentTransform.GetLocation() * scalar);
		// Recurse through the children
		ScaleAllChildrenComponentSpaceRecursive(jointData, childJointIdx, scalar);
	}
}

/**
 * Identify the parents of bones in the SkeletonMesh that are part of the bone map.
 *
 * The Ancestor should reflect the hierarchy of the source skeleton in the case that
 * a sibling or child joint was mapped out of hierarchy order to the target skeleton.
 */
void FOculusXRAnimNodeBodyRetargeter::CalculateMappedAncestorValues(const SourceInfo& SourceReferenceInfo, const int CurrentMappedAncestorIdx, const int JointIdx, TargetSkeletonPoseData& TargetSkeleton)
{
	if (JointIdx != INDEX_NONE)
	{
		TargetSkeletonJointEntry& jointEntry = TargetSkeleton.PoseData[JointIdx];

		// The structure should be initialized so that all ancestors are marked to none.
		check(jointEntry.mappedAncestorIdx == INDEX_NONE);

		int nextMappedIndex = CurrentMappedAncestorIdx;

		// If this is a mapped joint, we need to update the nextMapped index and ensure that our
		// mapped ancesstor is relative to the child joint the source skeleton.
		// This is to allow for alignment of target skeletons that are in a different hierarchical order than the source skeleton.
		// (Example: The tracking skeleton is elbow -> Wrist and WristTwist as children/siblings to each other, some target skeletons
		// are mapped as elbow -> WristTwist -> Wrist.  This complicates alignment code without using the source skeleton hierarchy as
		// the reference).
		if (jointEntry.sourceJointID != EOculusXRBoneID::None)
		{
			nextMappedIndex = JointIdx;

			// Walk up the source skeleton parenting to find the next mapped joint.  We can't trust that the CurrentMappedAncestorIdx is actually correct for this mapped joint.
			EOculusXRBoneID sourceParentJoint = SourceReferenceInfo.SourceSkeleton.GetParentBoneId(jointEntry.sourceJointID);
			while (sourceParentJoint != EOculusXRBoneID::None)
			{
				// Found a possible match - check if the joint is an actual ancestor to our current joint (in the target skeleton)
				if (SourceReferenceInfo.SourceToTargetIdxMap.Contains(sourceParentJoint) && TargetSkeleton.IsAncestorToBoneIndex(SourceReferenceInfo.SourceToTargetIdxMap[sourceParentJoint], JointIdx))
				{
					jointEntry.mappedAncestorIdx = SourceReferenceInfo.SourceToTargetIdxMap[sourceParentJoint];
					break;
				}
				sourceParentJoint = SourceReferenceInfo.SourceSkeleton.GetParentBoneId(sourceParentJoint);
			}
		}
		else
		{
			// In the case that this joint is unmapped, we use the current tracked ancestor index.
			// It's the last joint we found that was mapped.  This is a recursive function, so that value is passed
			// in as a parameter.
			jointEntry.mappedAncestorIdx = CurrentMappedAncestorIdx;
		}

		// Recursively iterate down the child joints
		for (int childJointIdx : jointEntry.childJoints)
		{
			CalculateMappedAncestorValues(SourceReferenceInfo, nextMappedIndex, childJointIdx, TargetSkeleton);
		}
	}
}

/**
 * This gets called when the bone container changes and we have to remap the bones in case the skeleton has changed.
 * @param BoneContainer
 */
TMap<FCompactPoseBoneIndex, EOculusXRBoneID> FOculusXRAnimNodeBodyRetargeter::RecalculateMapping(
	const FBoneContainer& BoneContainer, const TMap<EOculusXRBoneID, FName>* SourceToTargetNameMap)
{
	TMap<FCompactPoseBoneIndex, EOculusXRBoneID> TargetToSourceMap;
	if (SourceToTargetNameMap && !SourceToTargetNameMap->IsEmpty())
	{
		// Iterate through all possible bones in Source Skeleton and map them to the Target Skeleton
		// according to what the user has input in the retargeting asset
		for (uint8 SourceBoneId = 0; SourceBoneId < static_cast<uint8>(EOculusXRBoneID::COUNT); ++SourceBoneId)
		{
			const auto TargetBoneName = SourceToTargetNameMap->Find(static_cast<EOculusXRBoneID>(SourceBoneId));
			if (!TargetBoneName || TargetBoneName->IsNone())
			{
				// Skip the entry in the loop if the bone is not mapped
				continue;
			}
			// Check if the bone name exists in the current skeleton
			if (const int32 TargetBoneIndex = BoneContainer.GetPoseBoneIndexForBoneName(*TargetBoneName); TargetBoneIndex != INDEX_NONE)
			{
				auto TargetBoneId = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(TargetBoneIndex));
				if (TargetBoneId == INDEX_NONE)
				{
					UE_LOG(LogOculusXRRetargeting, Warning,
						TEXT("Bone %s was intentionally mapped to %s. But this target doesn't exist in skeleton."),
						*StaticEnum<EOculusXRBoneID>()->GetValueAsString(static_cast<EOculusXRBoneID>(SourceBoneId)),
						*TargetBoneName->ToString());
					continue;
				}
				TargetToSourceMap.Add(TargetBoneId, static_cast<EOculusXRBoneID>(SourceBoneId));
			}
		}

		// Iterate through all possible bones in Target Skeleton and add them to the TargetToSourceMap if they are not already present
		for (int32 TargetBoneIndex = 0; TargetBoneIndex < BoneContainer.GetNumBones(); ++TargetBoneIndex)
		{
			if (auto TargetBoneId = BoneContainer.MakeCompactPoseIndex(FMeshPoseBoneIndex(TargetBoneIndex)); !TargetToSourceMap.Contains(TargetBoneId))
			{
				TargetToSourceMap.Add(TargetBoneId, EOculusXRBoneID::None);
			}
		}
	}

	return TargetToSourceMap;
}

void FOculusXRAnimNodeBodyRetargeter::SetDebugPoseMode(const EOculusXRBodyDebugPoseMode mode)
{
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	DebugPoseMode = mode;
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
}

void FOculusXRAnimNodeBodyRetargeter::SetDebugDrawMode(const EOculusXRBodyDebugDrawMode mode)
{
#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	DebugDrawMode = mode;

	// Ensure we force an update to our Skeleton so if we're drawing the T-Pose it renders
	SourceReferenceInfo.Invalidate();
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
}

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
const FString FOculusXRAnimNodeBodyRetargeter::kRestPoseDebugDrawCategory = "RestPoseDraw";

const float FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::kLineDrawThickness = 0.2f;
const float FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::kAxisLineLength = 2.0f;
const FString FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::kDefaultDrawQueue = "default";

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::AddSkeleton(const FAbstractRetargetSkeleton& RestSkeleton, const FTransform& MeshTransform, const FColor& color, const FString& drawQueue, const bool bRenderAxis)
{
	// Do NOT FScopeLock this function - the arrays will be protected in the calls to AddLineSegment and AddAxis
	for (int BoneIndex = 0; BoneIndex < RestSkeleton.GetNumBones(); ++BoneIndex)
	{
		const auto ParentBoneIndex = RestSkeleton.GetParentBoneIndex(BoneIndex);
		auto BoneTransform = RestSkeleton.GetLocalTransform(BoneIndex);
		if (ParentBoneIndex == INDEX_NONE)
		{
			BoneTransform = BoneTransform * MeshTransform;
		}
		else
		{
			const auto ParentTransform = RestSkeleton.GetComponentTransform(ParentBoneIndex) * MeshTransform;
			BoneTransform = BoneTransform * ParentTransform;
			AddLineSegment(ParentTransform.GetLocation(), BoneTransform.GetLocation(), color, drawQueue);
		}
		if (bRenderAxis)
		{
			AddAxis(BoneTransform.GetLocation(), BoneTransform.GetRotation(), drawQueue);
		}
	}
}

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::AddSkeletonMapping(
	const FOculusXRRetargetSkeletonEOculusXRBoneID& SourceSkeleton,
	const FAbstractRetargetSkeleton& TargetSkeleton,
	const TMap<EOculusXRBoneID, int>& SourceToTargetIdxMap,
	const FTransform& MeshTransform,
	const FColor& color,
	const FString& drawQueue)
{
	// Do NOT FScopeLock this function - the arrays will be protected in the calls to AddLineSegment and AddAxis
	for (const auto& sourceMappingPair : SourceToTargetIdxMap)
	{
		const int SourceIdx = SourceSkeleton.GetBoneIndex(sourceMappingPair.Key);
		if (SourceIdx != INDEX_NONE && TargetSkeleton.IsValidIndex(sourceMappingPair.Value))
		{
			const FTransform sourceTransform = SourceSkeleton.GetComponentTransform(SourceIdx) * MeshTransform;
			const FTransform targetTransform = TargetSkeleton.GetComponentTransform(sourceMappingPair.Value) * MeshTransform;
			AddLineSegment(sourceTransform.GetLocation(), targetTransform.GetLocation(), color, drawQueue);
		}
	}
}

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::AddLineSegment(const FVector& start, const FVector& end, const FColor& color, const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	m_drawQueues.FindOrAdd(drawQueue).lines.Add(LineSegment{ start, end, color });
}

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::AddAxis(const FVector& position, const FQuat& rotation, const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	m_drawQueues.FindOrAdd(drawQueue).axis.Add(Axis{ position, rotation });
}

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::AddAxis(const FTransform& transform, const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	m_drawQueues.FindOrAdd(drawQueue).axis.Add(Axis{ transform.GetLocation(), transform.GetRotation() });
}

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::DrawFrame()
{
	const UWorld* World = GWorld;
#if WITH_EDITOR
	if (World && GIsEditor)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				World = Context.World();
				break;
			}
		}
	}
#endif // WITH_EDITOR
	if (!World)
	{
		return;
	}

	FScopeLock Lock(&MultiThreadLock);
	for (auto& queuePair : m_drawQueues)
	{
		for (LineSegment segment : queuePair.Value.lines)
		{
			DrawDebugLine(World, segment.start, segment.end, segment.color, false, -1, 1, kLineDrawThickness);
		}

		for (Axis axis : queuePair.Value.axis)
		{
			DrawDebugLine(World, axis.position, axis.position + (axis.rotation.GetAxisX() * kAxisLineLength), FColor::Red, false, -1, 1, kLineDrawThickness);
			DrawDebugLine(World, axis.position, axis.position + (axis.rotation.GetAxisY() * kAxisLineLength), FColor::Green, false, -1, 1, kLineDrawThickness);
			DrawDebugLine(World, axis.position, axis.position + (axis.rotation.GetAxisZ() * kAxisLineLength), FColor::Blue, false, -1, 1, kLineDrawThickness);
		}
	}
}

void FOculusXRAnimNodeBodyRetargeter::DebugDrawUtility::ClearDrawQueue(const FString& drawQueue)
{
	FScopeLock Lock(&MultiThreadLock);
	m_drawQueues.Remove(drawQueue);
}

#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
