/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "AnimNode_OculusXRBodyTracking.h"
#include "AnimNode_OculusXRFaceTracking.h"
#include "AnimNode_OculusXREyeTracking.h"
#include "AnimGraphNode_Base.h"
#include "OculusXR_TrackingGraphNodes.generated.h"

/**
 * This node is responsible for receiving the body tracking data from the Oculus SDK and applying it to the skeleton.
 */
UCLASS()
class OCULUSXRMOVEMENTUTILITYGRAPH_API UOculusXR_BodyTracking : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	// TODO: Hide the input pose data as that is not being used or valid

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_OculusXRBodyTracking Node;

	UFUNCTION(
		CallInEditor,
		Category = "OculusXR|BodyTracking",
		meta = (DisplayName = "Auto Fill Joints",
			ToolTip = "Automatically populates the Known Joints List and the Excluded Joints List"))
	void AutoFillJointData();

	UFUNCTION(
		CallInEditor,
		Category = "OculusXR|BodyTracking",
		meta = (DisplayName = "Write Config Data",
			ToolTip = "Generates retargeting mapping data and writes the file to JSON"))
	void WriteConfigData();

	UFUNCTION(
		CallInEditor,
		Category = Tools,
		meta = (DisplayName = "Export Skeleton Rest Pose",
			ToolTip = "Generates a retargeting config file with only the skeleton (target) rest pose to JSON"))
	void ExportSkeletonRestPose();

	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;

private:
	bool GetSaveFilePathFromJSONSaveDialog(FString& outFilePath, const FString& suffix = TEXT(""));
};

/**
 * This node is responsible for receiving the body tracking data from the Oculus SDK and applying it to the skeleton.
 */
UCLASS()
class OCULUSXRMOVEMENTUTILITYGRAPH_API UOculusXR_FaceTracking : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	// TODO: Hide the input pose data as that is not being used or valid

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_OculusXRFaceTracking Node;

	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;
};

/**
 * This node is responsible for receiving the body tracking data from the Oculus SDK and applying it to the skeleton.
 */
UCLASS()
class OCULUSXRMOVEMENTUTILITYGRAPH_API UOculusXR_EyeTracking : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	// TODO: Hide the input pose data as that is not being used or valid

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_OculusXREyeTracking Node;

	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;
};
