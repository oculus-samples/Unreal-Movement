/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once
#include "OculusXRLiveLinkRetargetBodyAsset.h"

struct OCULUSXRRETARGETING_API FOculusXRRetargetingUtils
{
	/**
	 * Converts from an EOculusXRAxis to an orientation quaternion of that direction
	 */
	static FTransform DirectionTransform(EOculusXRAxis Direction)
	{
		FVector Dir = FVector::ZeroVector;
		const uint8 IndexOfDir = static_cast<uint8>(Direction);
		const double Sign = IndexOfDir < static_cast<uint8>(EOculusXRAxis::NegativeX) ? 1 : -1;
		Dir[IndexOfDir % 3] = Sign * 1.0;
		return FTransform(Dir.ToOrientationQuat());
	}

	/**
	 * Get the transform from the tracking space to the component space
	 * @param ForwardTransform The forward transform of the mesh
	 */
	static FTransform GetTrackingSpaceToComponentSpace(const FTransform& ForwardTransform)
	{
		return DirectionTransform(ForwardTracking).Inverse() * ForwardTransform;
	}

	/**
	 * Compute the up and right axis for a bone based on the forward axis
	 */
	static FQuat LookRotation(FVector Forward)
	{
		Forward.Normalize();

		FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
		if (Right.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			Right = FVector::RightVector;
		}
		else
		{
			Right.Normalize();
		}
		const FVector Up = FVector::CrossProduct(Forward, Right);
		const FMatrix RotMatrix(Forward, Right, Up, FVector::ZeroVector);

		return RotMatrix.Rotator().Quaternion();
	}

	/**
	 * Compute the up axis for a bone based on the forward and right axis
	 */
	static FQuat LookRotation(FVector Forward, FVector Right)
	{
		Forward.Normalize();
		Right.Normalize();

		const FVector Up = FVector::CrossProduct(Forward, Right);
		const FMatrix RotMatrix(Forward, Right, Up, FVector::ZeroVector);

		return RotMatrix.Rotator().Quaternion();
	}

	/**
	 * Returns the scale factor from the world settings
	 */
	static bool GetUnitScaleFactorFromSettings(UWorld* World, float& OutWorldToMeters);

private:
	/**
	 * Oculus tracking space is using +X as its forward direction.
	 */
	inline static EOculusXRAxis ForwardTracking = EOculusXRAxis::X;
};
