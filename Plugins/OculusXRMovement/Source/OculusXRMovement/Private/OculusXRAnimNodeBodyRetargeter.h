/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "OculusXRBodyRetargeter.h"
#include "OculusXRMovementTypes.h"
#include "OculusXRRetargetSkeleton.h"
#if !UE_BUILD_SHIPPING
#include "Tickable.h"
#define OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW 1
#else
#define OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW 0
#endif // !UE_BUILD_SHIPPING

class FOculusXRAnimNodeBodyRetargeter : public FOculusXRBodyRetargeter
{
public:
	FOculusXRAnimNodeBodyRetargeter() {}

	virtual void Initialize(
		const EOculusXRBodyRetargetingMode RetargetingMode,
		const EOculusXRBodyRetargetingRootMotionBehavior RootMotionBehavior,
		const EOculusXRAxis MeshForwardFacingDir,
		const TMap<EOculusXRBoneID, FName>* SourceToTargetNameMap) override;

	virtual bool RetargetFromBodyState(const FOculusXRBodyState& BodyState,
		const USkeletalMeshComponent* SkeletalMeshComponent,
		const float WorldScale,
		FPoseContext& Output) override;

	virtual void SetDebugPoseMode(const EOculusXRBodyDebugPoseMode mode) override;
	virtual void SetDebugDrawMode(const EOculusXRBodyDebugDrawMode mode) override;

	virtual EOculusXRBodyRetargetingMode GetRetargetingMode() override { return InitData.RetargetingMode; }
	virtual EOculusXRBodyRetargetingRootMotionBehavior GetRootMotionBehavior() { return InitData.RootMotionBehavior; }

private:
	struct InitializationData
	{
		EOculusXRBodyRetargetingMode RetargetingMode = EOculusXRBodyRetargetingMode::RotationAndPositions;
		EOculusXRBodyRetargetingRootMotionBehavior RootMotionBehavior = EOculusXRBodyRetargetingRootMotionBehavior::CombineToRoot;
		EOculusXRAxis MeshForwardFacingDir = EOculusXRAxis::Y;

		FTransform TargetFacingTransform = FTransform::Identity;
		FTransform TrackingSpaceToComponentSpace = FTransform::Identity;

		const TMap<EOculusXRBoneID, FName>* SourceToTargetNameMap = nullptr;
	};

	struct SourceInfo
	{
		inline bool IsValid() const { return BoneContainerSerialNumber != 0; }
		inline bool RequiresUpdate(const int ChangeCount, const uint16 ContainerSerialNumber) const
		{
			return !IsValid() || SourceChangeCount != ChangeCount || BoneContainerSerialNumber != ContainerSerialNumber;
		}

		// Invalidate allows us to force an update to the skeleton
		void Invalidate() { BoneContainerSerialNumber = 0; }

		FOculusXRBodySkeleton SourceReferenceSkeleton;
		uint16 BoneContainerSerialNumber = 0;
		int SourceChangeCount = 0;
		TMap<EOculusXRBoneID, int> SourceToTargetIdxMap;
		FOculusXRRetargetSkeletonEOculusXRBoneID SourceSkeleton;
		FOculusXRRetargetSkeletonEOculusXRBoneID LastFrameBodyState;
	};

	struct TargetSkeletonJointEntry
	{
		// Initialized Section
		const FCompactPoseBoneIndex BoneId = FCompactPoseBoneIndex(INDEX_NONE); // Bone ID in Target Space (Index Value)
		const int ParentIdx = INDEX_NONE;										// Target Index of Parent Joint (or NONE)
		FTransform LocalTransform = FTransform::Identity;						// Transform in child Local relative space
		FTransform ComponentTransform = FTransform::Identity;					// Transform in Component (root relative) space
		const EOculusXRBoneID sourceJointID = EOculusXRBoneID::None;			// Source Joint ID if this Joint is Mapped
		const float unmodifiedJointLength = 0.0f;								// LocalTransform Translation scale before scaling and deformation

		// Calculated Section
		int mappedAncestorIdx = INDEX_NONE; // Closest Parent that is mapped
		FTransform sourceJointLocalOffset = FTransform::Identity;
		float componentSpaceScale = 1.0f; // Scale is based on the difference between the joint lengths after the character is adjusted

		TArray<int> childJoints;	  // Array of all Children for this Joint
		TArray<int> childTwistJoints; // Array of Children that are Twist Joints (subset of childJoints)

		inline int GetNonTwistChildJointCount() const { return childJoints.Num() - childTwistJoints.Num(); }
	};

	struct TwistJointEntry
	{
		const int TargetSourceJointIdx = INDEX_NONE;
		const int TargetTwistJointIdx = INDEX_NONE;
		const int TargetTwistParentJointIdx = INDEX_NONE;
		const int TargetSourceParentJointIdx = INDEX_NONE; // Typically the same value as TargetTwistParentJointIdx unless this is a chain
		const FQuat TargetSourceLocalRotationOffset = FQuat::Identity;
		const FVector ProjectedSourceJointAlignmentLocalSpace = FVector::ZeroVector;
		const float RestPoseParentToSourceJointLength = 0.0f;
		const float ProjectedTwistJointLength = 0.0f;
		const float weight = 0.5f;
		const bool isRotateable = true;
	};

	// Extends FAbstractRetargetSkeleton specifically to reduce code needed to debug draw as a skeleton
	struct TargetSkeletonPoseData : public FAbstractRetargetSkeleton
	{
		virtual bool IsValidIndex(const int BoneIndex) const override
		{
			return BoneIndex >= 0 && BoneIndex < PoseData.Num();
		}
		virtual int GetNumBones() const override
		{
			return PoseData.Num();
		}
		virtual int GetParentBoneIndex(const int BoneIndex) const override
		{
			return IsValidIndex(BoneIndex) ? PoseData[BoneIndex].ParentIdx : INDEX_NONE;
		}
		virtual const FTransform& GetLocalTransform(const int BoneIndex) const override
		{
			return IsValidIndex(BoneIndex) ? PoseData[BoneIndex].LocalTransform : FTransform::Identity;
		}
		virtual const FTransform& GetComponentTransform(const int BoneIndex) const override
		{
			return IsValidIndex(BoneIndex) ? PoseData[BoneIndex].ComponentTransform : FTransform::Identity;
		}
		virtual bool IsEmpty() const override
		{
			return PoseData.IsEmpty();
		}
		EOculusXRBoneID GetSourceJointID(const int BoneIndex) const
		{
			return IsValidIndex(BoneIndex) ? PoseData[BoneIndex].sourceJointID : EOculusXRBoneID::None;
		}
		bool IsJointMappedToSource(const int BoneIndex) const
		{
			return GetSourceJointID(BoneIndex) != EOculusXRBoneID::None;
		}
		int GetMappedAncestorIndex(const int BoneIndex) const
		{
			return IsValidIndex(BoneIndex) ? PoseData[BoneIndex].mappedAncestorIdx : INDEX_NONE;
		}
		bool IsAncestorToBoneIndex(const int AncestorBoneIdx, const int BoneIndex) const
		{
			int parentBoneIdx = GetParentBoneIndex(BoneIndex);
			while (parentBoneIdx != INDEX_NONE)
			{
				if (parentBoneIdx == AncestorBoneIdx)
				{
					return true;
				}
				parentBoneIdx = GetParentBoneIndex(parentBoneIdx);
			}
			return false;
		}
		int GetChildJointCount(const int BoneIndex) const
		{
			return IsValidIndex(BoneIndex) ? PoseData[BoneIndex].childJoints.Num() : 0;
		}
		// Recursive function - depth first search
		int FindNextChildJointMappedToSource(const int ParentBoneIndex) const
		{
			if (IsValidIndex(ParentBoneIndex))
			{
				for (int childIdx : PoseData[ParentBoneIndex].childJoints)
				{
					if (IsJointMappedToSource(childIdx))
					{
						return childIdx;
					}
					int nextChildJointMapped = FindNextChildJointMappedToSource(childIdx);
					if (nextChildJointMapped != INDEX_NONE)
					{
						return nextChildJointMapped;
					}
				}
			}
			return INDEX_NONE;
		}

		TArray<TargetSkeletonJointEntry> PoseData;

		// Store Identified Twist Joint Chains here
		TMap<int, TwistJointEntry> TwistJoints;

		// This scale is based on the overall height scaling to align the
		// wrists of the target rig on the Z-Axis
		float GlobalComponentSpaceScale = 1.0f;
	};

	inline bool IsInitialized() const;

	// Update Section:

	// These functions are called during RetargetFromBodyState
	// Separated so we can better identify/mark in a profiler capture.
	bool UpdateSkeleton(const FOculusXRBodyState& BodyState,
		const FBoneContainer& BoneContainer,
		const USkeletalMeshComponent* SkeletalMeshComponent,
		const float WorldScale);

	bool ProcessFrameRetargeting(const FOculusXRBodyState& BodyState,
		const USkeletalMeshComponent* SkeletalMeshComponent,
		FPoseContext& Output);

	// Called from within ProcessFrameRetargeting
	void ProcessFrameInterpolateTwistJoints(TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses) const;
	void UpdateScaleForFrame(const int TargetIndex, TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses) const;
	void UpdateScaleForFrameRecursive(const int TargetIndex, const float scale, TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses) const;
	TTuple<float, float> GetFrameMaxCurrentAndUnModifiedJointLengths(int targetJointIndex, const TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& FramePoses, float currentLength = 0.0f, float unmodifiedLength = 0.0f) const;

	// End of Update Section

	// Setup/Calculation section - Called from UpdateSkeleton when a state change occurs

	void SetTargetToTPose();
	void CacheTwistJoints();
	void ApplyScaleAndProportion();
	void InitializeScaleAndOffsetData();
	TTuple<float, float> GetMaxCurrentAndUnModifiedJointLengths(int targetJointIndex, float currentLength = 0.0f, float unmodifiedLength = 0.0f) const;

	// End of Setup/Calculation section

	static void CaptureUnmappedJointChainRecursive(const TArray<TargetSkeletonJointEntry>& jointData, const int jointIdx, TArray<int>& outLinkArray);
	static void UpdateAllChildrenComponentFromLocalRecursive(TArray<TargetSkeletonJointEntry>& jointData, const int rootJointIdx);
	static void ScaleAllChildrenComponentSpaceRecursive(TArray<TargetSkeletonJointEntry>& jointData, const int rootJointIdx, const FVector& scalar);

	static void CalculateMappedAncestorValues(const SourceInfo& SourceReferenceInfo, const int CurrentMappedAncestorIdx, const int JointIdx, TargetSkeletonPoseData& TargetSkeleton);
	static TMap<FCompactPoseBoneIndex, EOculusXRBoneID> RecalculateMapping(const FBoneContainer& BoneContainer, const TMap<EOculusXRBoneID, FName>* SourceToTargetNameMap);

	static const float kTWIST_JOINT_MIN_ANGLE_THRESHOLD;
	static const TSet<EOculusXRBoneID> GenerateTPoseJointSet();
	static const TArray<EOculusXRBoneID> kALIGNABLE_HAND_JOINTS;
	static const TSet<EOculusXRBoneID> kTPOSE_ADJUSTABLE_JOINT_SET;
	static const TSet<EOculusXRBoneID> kTPOSE_SPECIAL_HANDLING_ADJUSTABLE_HAND_JOINTS;

	InitializationData InitData;
	SourceInfo SourceReferenceInfo;
	TMap<FCompactPoseBoneIndex, EOculusXRBoneID> TargetToSourceMap;
	TargetSkeletonPoseData TargetAdjustedRestPoseData;

#if OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
	static const FString kRestPoseDebugDrawCategory;

	class DebugDrawUtility : public FTickableGameObject
	{
	public:
		static const float kLineDrawThickness;
		static const float kAxisLineLength;
		static const FString kDefaultDrawQueue;

		struct LineSegment
		{
			FVector start;
			FVector end;
			FColor color;
		};

		struct Axis
		{
			FVector position;
			FQuat rotation;
		};

		struct DrawQueue
		{
			TArray<LineSegment> lines;
			TArray<Axis> axis;
		};

		DebugDrawUtility() {}
		virtual ~DebugDrawUtility() {}

		void AddSkeleton(const FAbstractRetargetSkeleton& RestSkeleton, const FTransform& MeshTransform, const FColor& color, const FString& drawQueue = kDefaultDrawQueue, const bool bRenderAxis = true);
		void AddSkeletonMapping(
			const FOculusXRRetargetSkeletonEOculusXRBoneID& SourceSkeleton,
			const FAbstractRetargetSkeleton& TargetSkeleton,
			const TMap<EOculusXRBoneID, int>& SourceToTargetIdxMap,
			const FTransform& MeshTransform, const FColor& color, const FString& drawQueue = kDefaultDrawQueue);
		void AddLineSegment(const FVector& start, const FVector& end, const FColor& color, const FString& drawQueue = kDefaultDrawQueue);
		void AddAxis(const FVector& position, const FQuat& rotation, const FString& drawQueue = kDefaultDrawQueue);
		void AddAxis(const FTransform& transform, const FString& drawQueue = kDefaultDrawQueue);
		void DrawFrame();
		void ClearDrawQueue(const FString& drawQueue = kDefaultDrawQueue);

		virtual void Tick(float DeltaTime) override
		{
			DrawFrame();
			ClearDrawQueue(); // Clear the Default Queue
		}

		virtual ETickableTickType GetTickableTickType() const override
		{
			return ETickableTickType::Always;
		}
		virtual TStatId GetStatId() const override
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(DebugDrawUtility, STATGROUP_Tickables);
		}
		virtual bool IsTickableWhenPaused() const
		{
			return true;
		}
		virtual bool IsTickableInEditor() const
		{
			return false;
		}

	private:
		mutable FCriticalSection MultiThreadLock;
		TMap<FString, DrawQueue> m_drawQueues;
	};

	DebugDrawUtility DebugDrawUtility;
	EOculusXRBodyDebugDrawMode DebugDrawMode = EOculusXRBodyDebugDrawMode::None;
	EOculusXRBodyDebugPoseMode DebugPoseMode = EOculusXRBodyDebugPoseMode::None;
#endif // OCULUS_XR_TRACKING_ENABLE_DEBUG_DRAW
};
