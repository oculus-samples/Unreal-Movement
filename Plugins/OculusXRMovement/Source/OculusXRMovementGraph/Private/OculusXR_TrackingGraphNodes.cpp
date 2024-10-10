/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXR_TrackingGraphNodes.h"

void UOculusXR_BodyTracking::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UOculusXR_BodyTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("OculusXR Body Tracking");
}

FText UOculusXR_BodyTracking::GetTooltipText() const
{
	return FText::FromString("This node is responsible for receiving the body tracking data from the HMD and applying it to the skeleton.");
}

FString UOculusXR_BodyTracking::GetNodeCategory() const
{
	return FString("OculusXR Body Tracking");
}

void UOculusXR_FaceTracking::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UOculusXR_FaceTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("OculusXR Face Tracking");
}

FText UOculusXR_FaceTracking::GetTooltipText() const
{
	return FText::FromString("This node is responsible for receiving the face tracking data from the HMD and applying it to the skeleton.");
}

FString UOculusXR_FaceTracking::GetNodeCategory() const
{
	return FString("OculusXR Face Tracking");
}

void UOculusXR_EyeTracking::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UOculusXR_EyeTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("OculusXR Eye Tracking");
}

FText UOculusXR_EyeTracking::GetTooltipText() const
{
	return FText::FromString("This node is responsible for receiving the eye tracking data from the HMD and applying it to the skeleton.");
}

FString UOculusXR_EyeTracking::GetNodeCategory() const
{
	return FString("OculusXR Eye Tracking");
}
