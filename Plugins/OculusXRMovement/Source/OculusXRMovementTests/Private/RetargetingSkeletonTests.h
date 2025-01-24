/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "Misc/AutomationTest.h"
#include "OculusXRMovementTypes.h"
#include "OculusXRRetargetSkeleton.h"

#if UE_VERSION_OLDER_THAN(5, 5, 0)
#define RetargetSkeletonTestFilters EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
#else
#define RetargetSkeletonTestFilters EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter
#endif // UE_VERSION_OLDER_THAN(5, 5, 0)

// These tests check that the simple operations of getting bone IDs, parent bone indices, local transforms, and component transforms work as expected.

inline TOculusXRRetargetSkeleton<EOculusXRBoneID>* CreateSkeleton()
{
	// Instantiate the class you want to test
	const TArray<TOculusXRRetargetSkeletonJoint<EOculusXRBoneID>> JointData = {
		{ EOculusXRBoneID::BodyRoot, INDEX_NONE, FTransform::Identity, FTransform::Identity },
		{ EOculusXRBoneID::BodyHips, 0, FTransform::Identity, FTransform::Identity },
	};

	return new FOculusXRRetargetSkeletonEOculusXRBoneID(JointData);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGetNumBones, "OculusXRRetargetingTests.FGetNumBones", RetargetSkeletonTestFilters)
inline bool FGetNumBones::RunTest(const FString& Parameters)
{
	const auto Skeleton = CreateSkeleton();

	// Test GetNumBones method
	TestEqual("Number of bones should be 2", Skeleton->GetNumBones(), 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIsValidIndex, "OculusXRRetargetingTests.FIsValidIndex", RetargetSkeletonTestFilters)
inline bool FIsValidIndex::RunTest(const FString& Parameters)
{
	const auto Skeleton = CreateSkeleton();

	// Test IsValidIndex method
	TestFalse("Index -1 should be invalid", Skeleton->IsValidIndex(-1));
	TestFalse("INDEX_NONE should be invalid", Skeleton->IsValidIndex(INDEX_NONE));
	TestTrue("Index 0 should be valid", Skeleton->IsValidIndex(0));
	TestTrue("Index 1 should be valid", Skeleton->IsValidIndex(1));
	TestFalse("Index 2 should be invalid", Skeleton->IsValidIndex(2));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHasParentTests, "OculusXRRetargetingTests.FHasParentTests", RetargetSkeletonTestFilters)
inline bool FHasParentTests::RunTest(const FString& Parameters)
{
	const auto Skeleton = CreateSkeleton();

	// Test HasParent method
	TestFalse("Index -1 should not have a parent", Skeleton->HasParent(-1));
	TestFalse("INDEX_NONE should not have a parent", Skeleton->HasParent(INDEX_NONE));
	TestFalse("Bone at index 0 should not have a parent", Skeleton->HasParent(0));
	TestTrue("Bone at index 1 should have a parent", Skeleton->HasParent(1));
	TestFalse("Index 2 should not have a parent", Skeleton->HasParent(2));

	TestFalse("BoneID NONE should not have a parent", Skeleton->HasParentBoneId(EOculusXRBoneID::None));
	TestFalse("BoneID BodyRoot should not have a parent", Skeleton->HasParentBoneId(EOculusXRBoneID::BodyRoot));
	TestTrue("BoneID BodyHips should have a parent", Skeleton->HasParentBoneId(EOculusXRBoneID::BodyHips));
	TestFalse("BoneID that is not present in the dataset should not have a parent", Skeleton->HasParentBoneId(EOculusXRBoneID::BodyHead));

	return true;
}
