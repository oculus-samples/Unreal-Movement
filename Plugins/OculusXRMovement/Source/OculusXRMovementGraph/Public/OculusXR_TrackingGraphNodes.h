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
class OCULUSXRRETARGETINGGRAPH_API UOculusXR_BodyTracking : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	// TODO: Hide the input pose data as that is not being used or valid

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_OculusXRBodyTracking Node;

	UFUNCTION(CallInEditor, Category = Tools)
	void GenerateBoneMapping();

	UPROPERTY(EditAnywhere, Category = Tools, meta = (ClampMin = "0", ClampMax = "1"))
	float FilterSensitivity = 0.75;

	static const int MIN_SUBSTRING_LENGTH = 3;

	int CountSubstringsCharLength(const TSet<FString>& Substrings);

	TSet<FString> GetSubstringsBetweenJointNames(FString sourceJointName, FString targetJointName, int minSubstringLength = MIN_SUBSTRING_LENGTH, TSet<FString> filterList = {});

	UPROPERTY(EditAnywhere, Category = Tools)
	TSet<FString> FilterJoints = { "Root", "Hips", "Body", "Spine", "Lower", "Middle", "Upper", "Chest", "Neck", "Head", "Shoulder", "Arm", "Hand", "Leg", "Foot", "Left", "Right", "Thumb", "Index", "Ring", "Pinky", "Little", "Ball" };

	virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;
};

/**
 * This node is responsible for receiving the body tracking data from the Oculus SDK and applying it to the skeleton.
 */
UCLASS()
class OCULUSXRRETARGETINGGRAPH_API UOculusXR_FaceTracking : public UAnimGraphNode_Base
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
class OCULUSXRRETARGETINGGRAPH_API UOculusXR_EyeTracking : public UAnimGraphNode_Base
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
