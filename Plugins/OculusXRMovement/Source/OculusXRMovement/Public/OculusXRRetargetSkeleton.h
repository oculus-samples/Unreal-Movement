/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once
#include "OculusXRMovementTypes.h"
#include "OculusXRBodyRetargeter.h"

struct OCULUSXRRETARGETING_API FAbstractRetargetSkeleton
{
	virtual ~FAbstractRetargetSkeleton() = default;
	virtual bool IsEmpty() const = 0;
	virtual int GetNumBones() const = 0;
	virtual bool IsValidIndex(const int BoneIndex) const = 0;
	virtual int GetParentBoneIndex(const int BoneIndex) const = 0;
	virtual const FTransform& GetLocalTransform(const int BoneIndex) const = 0;
	virtual const FTransform& GetComponentTransform(const int BoneIndex) const = 0;
};

template <typename T>
struct OCULUSXRRETARGETING_API TOculusXRRetargetSkeletonJoint
{
	T BoneId;
	int ParentIdx;
	FTransform LocalTransform;
	FTransform ComponentTransform;

	static void CalculateComponentToLocalSpace(TArray<TOculusXRRetargetSkeletonJoint<T>>& JointData)
	{
		// Calculate the local transforms from the component transforms
		for (int i = 0; i < JointData.Num(); ++i)
		{
			int ParentIdx = JointData[i].ParentIdx;
			JointData[i].LocalTransform = ParentIdx == INDEX_NONE ? JointData[i].ComponentTransform : // No Parent - Root Space
				JointData[i].ComponentTransform.GetRelativeTransform(JointData[ParentIdx].ComponentTransform);
		}
	}

	static void CalculateLocalToComponentSpace(TArray<TOculusXRRetargetSkeletonJoint<T>>& JointData)
	{
		// TODO: Fix to be more bulletproof - Based on an assumption that the Joint Entry array is
		// sorted from Parent to Child.  If it isn't, then the Parent Transform may not get calculated before the child.

		// Calculate the component transforms from the local transforms
		for (int i = 0; i < JointData.Num(); ++i)
		{
			int ParentIdx = JointData[i].ParentIdx;
			JointData[i].ComponentTransform = ParentIdx == INDEX_NONE ? JointData[i].LocalTransform : // No Parent - Root Space
				JointData[i].LocalTransform * JointData[ParentIdx].ComponentTransform;
		}
	}
};

/**
 * @brief A template struct for Oculus XR retarget skeleton.
 *
 * @tparam T The type of the bone IDs.
 */
template <typename T>
struct OCULUSXRRETARGETING_API TOculusXRRetargetSkeleton : FAbstractRetargetSkeleton
{
protected:
	/**
	 * @brief Construct a new TOculusXRRetargetSkeleton object. Empty Constructor
	 */

	TOculusXRRetargetSkeleton() {}

	/**
	 * @brief Construct a new TOculusXRRetargetSkeleton object.
	 *
	 * @param _JointData The array of joint data.
	 * @param InvalidJointID Value of an Invalid Joint ID
	 */
	TOculusXRRetargetSkeleton(
		const TArray<TOculusXRRetargetSkeletonJoint<T>>& _JointData, const T InvalidJointID)
		: JointData(_JointData)
	{
		for (int iBoneIdx = 0; iBoneIdx < JointData.Num(); ++iBoneIdx)
		{
			if (JointData[iBoneIdx].BoneId != InvalidJointID)
			{
				BoneIdToJointIdxMap.Add(JointData[iBoneIdx].BoneId, iBoneIdx);
			}
		}
	}

	/**
	 * @brief Copy Constructor - Construct a new TOculusXRRetargetSkeleton object.
	 *
	 * @param other - Instance of class to copy from
	 */
	TOculusXRRetargetSkeleton(const TOculusXRRetargetSkeleton<T>& other)
		: JointData(other.JointData)
		, BoneIdToJointIdxMap(other.BoneIdToJointIdxMap)
	{
	}

	// Assignment Operator
	TOculusXRRetargetSkeleton<T>& operator=(const TOculusXRRetargetSkeleton<T>& other)
	{
		this->JointData = other.JointData;
		this->BoneIdToJointIdxMap = other.BoneIdToJointIdxMap;
		return *this;
	}

	virtual const T& InvalidBoneID() const = 0;

private:
	TArray<TOculusXRRetargetSkeletonJoint<T>> JointData;
	TMap<T, int> BoneIdToJointIdxMap;

public:
	const TArray<TOculusXRRetargetSkeletonJoint<T>>& GetJointDataArray() const { return JointData; }

	/**
	 * @brief Get the number of bones.
	 *
	 * @return int The number of bones.
	 */
	virtual int GetNumBones() const override
	{
		return JointData.Num();
	}

	/**
	 * @brief Check if the bone index is valid.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return bool True if the bone index is valid, false otherwise.
	 */
	virtual bool IsValidIndex(const int BoneIndex) const override
	{
		return BoneIndex >= 0 && BoneIndex < JointData.Num();
	}

	/**
	 * @brief Get the bone index by bone ID.
	 *
	 * @param BoneId The bone ID.
	 * @return int The bone index.
	 */
	int GetBoneIndex(const T& BoneId) const
	{
		return (BoneIdToJointIdxMap.Contains(BoneId) ? BoneIdToJointIdxMap[BoneId] : INDEX_NONE);
	}

	/**
	 * @brief Returns true if the Skeleton Structure has no data.
	 */
	virtual bool IsEmpty() const override
	{
		return JointData.IsEmpty();
	}

	/**
	 * @brief Check if the bone ID is valid.
	 *
	 * @param BoneId The ID of the bone.
	 * @return bool True if the bone ID is valid, false otherwise.
	 */
	bool IsValid(const T& BoneId) const
	{
		return BoneId != InvalidBoneID();
	}

	/**
	 * @brief Check if the bone at the given index is valid.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return bool True if the bone at that index is valid, false otherwise.
	 */
	/*
	bool IsValid(const int BoneIndex) const
	{
		return IsValidIndex(BoneIndex) && IsValid(GetBoneId(BoneIndex));
	}
	*/

	/**
	 * @brief Get the bone ID by index.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return T The bone ID.
	 */
	T GetBoneId(const int BoneIndex) const
	{
		return IsValidIndex(BoneIndex) ? JointData[BoneIndex].BoneId : InvalidBoneID();
	}

	/**
	 * @brief Checks if the bone at the given index has a valid parent.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return bool True if the bone at the given index has a valid parent, false otherwise.
	 */
	bool HasParent(const int BoneIndex) const
	{
		if (!IsValidIndex(BoneIndex))
			return false;
		return IsValidIndex(GetParentBoneIndex(BoneIndex));
	}

	/**
	 * @brief Checks if the bone with the given ID has a parent.
	 *
	 * @param BoneId The ID of the bone.
	 * @return bool True if the bone with the given ID has a parent, false otherwise.
	 */
	bool HasParentBoneId(const T& BoneId) const
	{
		return HasParent(GetBoneIndex(BoneId));
	}

	/**
	 * @brief Get the parent bone index by bone index.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return int The parent bone index.
	 */
	virtual int GetParentBoneIndex(const int BoneIndex) const override
	{
		return IsValidIndex(BoneIndex) ? JointData[BoneIndex].ParentIdx : INDEX_NONE;
	}

	/**
	 * @brief Get the parent bone ID by bone ID.
	 *
	 * @param BoneId The bone ID.
	 * @return T The parent bone ID.
	 */
	T GetParentBoneId(const T& BoneId) const
	{
		const auto BoneIndex = GetBoneIndex(BoneId);
		const auto ParentBoneIndex = GetParentBoneIndex(BoneIndex);
		return GetBoneId(ParentBoneIndex);
	}

	/**
	 * @brief Get the local transform by bone index.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return FTransform The local transform.
	 */
	virtual const FTransform& GetLocalTransform(const int BoneIndex) const override
	{
		return IsValidIndex(BoneIndex) ? JointData[BoneIndex].LocalTransform : FTransform::Identity;
	}

	/**
	 * @brief Get the component transform by bone index.
	 *
	 * @param BoneIndex The index of the bone.
	 * @return FTransform The component transform.
	 */
	virtual const FTransform& GetComponentTransform(const int BoneIndex) const override
	{
		return IsValidIndex(BoneIndex) ? JointData[BoneIndex].ComponentTransform : FTransform::Identity;
	}
};

// Explicit instantiation of the TOculusXRRetargetSkeleton template for the relevant types.
// This is needed in order for the compiler to properly make sense of method overloads and the like

template struct OCULUSXRRETARGETING_API TOculusXRRetargetSkeleton<EOculusXRBoneID>;
struct OCULUSXRRETARGETING_API FOculusXRRetargetSkeletonEOculusXRBoneID final : TOculusXRRetargetSkeleton<EOculusXRBoneID>
{
	FOculusXRRetargetSkeletonEOculusXRBoneID() {}

	FOculusXRRetargetSkeletonEOculusXRBoneID(
		const TArray<TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>>& JointData)
		: TOculusXRRetargetSkeleton<EOculusXRBoneID>(JointData, EOculusXRBoneID::None)
	{
		// All arrays should have at least one element
		check(!IsEmpty());
	}

	// Copy Constructor
	FOculusXRRetargetSkeletonEOculusXRBoneID(const FOculusXRRetargetSkeletonEOculusXRBoneID& other)
		: TOculusXRRetargetSkeleton<EOculusXRBoneID>(other)
	{
	}

	// Assignment Operator
	FOculusXRRetargetSkeletonEOculusXRBoneID& operator=(const FOculusXRRetargetSkeletonEOculusXRBoneID& other)
	{
		TOculusXRRetargetSkeleton<EOculusXRBoneID>::operator=(other);
		return *this;
	}

	static const EOculusXRBoneID kINVALID_BONE_ID;
	virtual const EOculusXRBoneID& InvalidBoneID() const override { return kINVALID_BONE_ID; }
};

template struct OCULUSXRRETARGETING_API TOculusXRRetargetSkeleton<FCompactPoseBoneIndex>;
struct OCULUSXRRETARGETING_API FOculusXRRetargetSkeletonFCompactPoseBoneIndex final : TOculusXRRetargetSkeleton<FCompactPoseBoneIndex>
{
	FOculusXRRetargetSkeletonFCompactPoseBoneIndex(
		const TArray<TOculusXRRetargetSkeletonJoint<FCompactPoseBoneIndex>>& JointData)
		: TOculusXRRetargetSkeleton<FCompactPoseBoneIndex>(JointData, FCompactPoseBoneIndex(INDEX_NONE))
	{
		// All arrays should have at least one element
		check(!IsEmpty());
	}

	// Copy Constructor
	FOculusXRRetargetSkeletonFCompactPoseBoneIndex(const FOculusXRRetargetSkeletonFCompactPoseBoneIndex& other)
		: TOculusXRRetargetSkeleton<FCompactPoseBoneIndex>(other)
	{
	}

	// Assignment Operator
	FOculusXRRetargetSkeletonFCompactPoseBoneIndex& operator=(const FOculusXRRetargetSkeletonFCompactPoseBoneIndex& other)
	{
		TOculusXRRetargetSkeleton<FCompactPoseBoneIndex>::operator=(other);
		return *this;
	}

	static const FCompactPoseBoneIndex kINVALID_BONE_ID;
	virtual const FCompactPoseBoneIndex& InvalidBoneID() const override { return kINVALID_BONE_ID; }
};

namespace Factory
{
	/**
	 * @brief Create a TOculusXRRetargetSkeleton object from Oculus XR body skeleton.
	 *
	 * @param SourceReferenceSkeleton The source reference skeleton.
	 * @param TrackingSpaceToComponentSpace The transform from tracking space to component space.
	 * @return FOculusXRRetargetSkeletonEOculusXRBoneID A TOculusXRRetargetSkeleton object.
	 */
	FOculusXRRetargetSkeletonEOculusXRBoneID FromOculusXRBodySkeleton(
		const FOculusXRBodySkeleton& SourceReferenceSkeleton,
		const FTransform& TrackingSpaceToComponentSpace);

	/**
	 * @brief Create a TOculusXRRetargetSkeleton object from Oculus XR body state.
	 *
	 * @param SourceFrameSkeleton The current frame of the source skeleton.
	 * @param SourceReferenceSkeleton The reference skeleton of the source skeleton.
	 * @param TrackingSpaceToComponentSpace The transform from tracking space to component space.
	 * @param rootMotionBehavior The root motion behavior to be applied when caching the pose.
	 * @return FOculusXRRetargetSkeletonEOculusXRBoneID A TOculusXRRetargetSkeleton object.
	 */
	FOculusXRRetargetSkeletonEOculusXRBoneID FromOculusXRBodyState(
		const FOculusXRBodyState& SourceFrameSkeleton,
		const FOculusXRBodySkeleton& SourceReferenceSkeleton,
		const FTransform& TrackingSpaceToComponentSpace,
		const EOculusXRBodyRetargetingRootMotionBehavior rootMotionBehavior);

	/**
	 * @brief Create a TOculusXRRetargetSkeleton object from a reference skeleton.
	 *
	 * @param TargetBoneContainer The bone container of the target skeleton from which to create the TOculusXRRetargetSkeleton object.
	 * @return FOculusXRRetargetSkeletonFCompactPoseBoneIndex A TOculusXRRetargetSkeleton object.
	 */
	FOculusXRRetargetSkeletonFCompactPoseBoneIndex FromBoneContainer(
		const FBoneContainer& TargetBoneContainer);

	/**
	 * @brief Create a TOculusXRRetargetSkeleton object from an Array of BoneId/ComponentSpace Transform pairs and origin Skeleton.
	 *
	 * @param BaseSkeleton The skeleton the component transform Array is relative to
	 * @param ComponentSpaceTransforms An Array of Compoenent Space Transform Data matching the BaseSkeleton
	 * @return FOculusXRRetargetSkeletonFCompactPoseBoneIndex A FOculusXRRetargetSkeletonFCompactPoseBoneIndex object.
	 */
	FOculusXRRetargetSkeletonFCompactPoseBoneIndex FromComponentSpaceTransformArray(
		const FAbstractRetargetSkeleton& BaseSkeleton,
		const TArray<TTuple<FCompactPoseBoneIndex, FTransform, float>>& ComponentSpaceTransformPairs);

} // namespace Factory
