/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "OculusXRLiveLinkRetargetBodyAsset.h"
#include "OculusXRRetargetSkeleton.h"
#include "Animation/AnimNodeBase.h"
#include "OculusXRMorphTargetsController.h"
#include "AnimNode_OculusXRFaceTracking.generated.h"

USTRUCT(BlueprintType)
struct OCULUSXRRETARGETING_API FOculusXRFaceExpressionModifierNew
{
	GENERATED_BODY()
public:
	FOculusXRFaceExpressionModifierNew()
		: MinValue(0.f)
		, MaxValue(1.f)
		, Multiplier(1.f)
	{
	}

	UPROPERTY(EditAnywhere, Category = "OculusXR|Movement")
	float MinValue;

	UPROPERTY(EditAnywhere, Category = "OculusXR|Movement")
	float MaxValue;

	UPROPERTY(EditAnywhere, Category = "OculusXR|Movement")
	float Multiplier;
};

USTRUCT(BlueprintType)
struct OCULUSXRRETARGETING_API FOculusXRExpressionCurves
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "OculusXR|Movement")
	TArray<FName> CurveNames;
};

USTRUCT(Blueprintable)
struct OCULUSXRRETARGETING_API FAnimNode_OculusXRFaceTracking : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FPoseLink InputPose;

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	/**
	 * Remapping from bone ID to target skeleton's bone name.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "OculusXR|FaceTracking")
	TMap<EOculusXRFaceExpression, FOculusXRExpressionCurves> ExpressionNames = {
		{ EOculusXRFaceExpression::BrowLowererL, FOculusXRExpressionCurves{ { FName("browLowerer_L") } } },
		{ EOculusXRFaceExpression::BrowLowererR, FOculusXRExpressionCurves{ { FName("browLowerer_R") } } },
		{ EOculusXRFaceExpression::CheekPuffL, FOculusXRExpressionCurves{ { FName("cheekPuff_L") } } },
		{ EOculusXRFaceExpression::CheekPuffR, FOculusXRExpressionCurves{ { FName("cheekPuff_R") } } },
		{ EOculusXRFaceExpression::CheekRaiserL, FOculusXRExpressionCurves{ { FName("cheekRaiser_L") } } },
		{ EOculusXRFaceExpression::CheekRaiserR, FOculusXRExpressionCurves{ { FName("cheekRaiser_R") } } },
		{ EOculusXRFaceExpression::CheekSuckL, FOculusXRExpressionCurves{ { FName("cheekSuck_L") } } },
		{ EOculusXRFaceExpression::CheekSuckR, FOculusXRExpressionCurves{ { FName("cheekSuck_R") } } },
		{ EOculusXRFaceExpression::ChinRaiserB, FOculusXRExpressionCurves{ { FName("chinRaiser_B") } } },
		{ EOculusXRFaceExpression::ChinRaiserT, FOculusXRExpressionCurves{ { FName("chinRaiser_T") } } },
		{ EOculusXRFaceExpression::DimplerL, FOculusXRExpressionCurves{ { FName("dimpler_L") } } },
		{ EOculusXRFaceExpression::DimplerR, FOculusXRExpressionCurves{ { FName("dimpler_R") } } },
		{ EOculusXRFaceExpression::EyesClosedL, FOculusXRExpressionCurves{ { FName("eyesClosed_L") } } },
		{ EOculusXRFaceExpression::EyesClosedR, FOculusXRExpressionCurves{ { FName("eyesClosed_R") } } },
		{ EOculusXRFaceExpression::EyesLookDownL, FOculusXRExpressionCurves{ { FName("eyesLookDown_L") } } },
		{ EOculusXRFaceExpression::EyesLookDownR, FOculusXRExpressionCurves{ { FName("eyesLookDown_R") } } },
		{ EOculusXRFaceExpression::EyesLookLeftL, FOculusXRExpressionCurves{ { FName("eyesLookLeft_L") } } },
		{ EOculusXRFaceExpression::EyesLookLeftR, FOculusXRExpressionCurves{ { FName("eyesLookLeft_R") } } },
		{ EOculusXRFaceExpression::EyesLookRightL, FOculusXRExpressionCurves{ { FName("eyesLookRight_L") } } },
		{ EOculusXRFaceExpression::EyesLookRightR, FOculusXRExpressionCurves{ { FName("eyesLookRight_R") } } },
		{ EOculusXRFaceExpression::EyesLookUpL, FOculusXRExpressionCurves{ { FName("eyesLookUp_L") } } },
		{ EOculusXRFaceExpression::EyesLookUpR, FOculusXRExpressionCurves{ { FName("eyesLookUp_R") } } },
		{ EOculusXRFaceExpression::InnerBrowRaiserL, FOculusXRExpressionCurves{ { FName("innerBrowRaiser_L") } } },
		{ EOculusXRFaceExpression::InnerBrowRaiserR, FOculusXRExpressionCurves{ { FName("innerBrowRaiser_R") } } },
		{ EOculusXRFaceExpression::JawDrop, FOculusXRExpressionCurves{ { FName("jawDrop") } } },
		{ EOculusXRFaceExpression::JawSidewaysLeft, FOculusXRExpressionCurves{ { FName("jawSidewaysLeft") } } },
		{ EOculusXRFaceExpression::JawSidewaysRight, FOculusXRExpressionCurves{ { FName("jawSidewaysRight") } } },
		{ EOculusXRFaceExpression::JawThrust, FOculusXRExpressionCurves{ { FName("jawThrust") } } },
		{ EOculusXRFaceExpression::LidTightenerL, FOculusXRExpressionCurves{ { FName("lidTightener_L") } } },
		{ EOculusXRFaceExpression::LidTightenerR, FOculusXRExpressionCurves{ { FName("lidTightener_R") } } },
		{ EOculusXRFaceExpression::LipCornerDepressorL, FOculusXRExpressionCurves{ { FName("lipCornerDepressor_L") } } },
		{ EOculusXRFaceExpression::LipCornerDepressorR, FOculusXRExpressionCurves{ { FName("lipCornerDepressor_R") } } },
		{ EOculusXRFaceExpression::LipCornerPullerL, FOculusXRExpressionCurves{ { FName("lipCornerPuller_L") } } },
		{ EOculusXRFaceExpression::LipCornerPullerR, FOculusXRExpressionCurves{ { FName("lipCornerPuller_R") } } },
		{ EOculusXRFaceExpression::LipFunnelerLB, FOculusXRExpressionCurves{ { FName("lipFunneler_LB") } } },
		{ EOculusXRFaceExpression::LipFunnelerLT, FOculusXRExpressionCurves{ { FName("lipFunneler_LT") } } },
		{ EOculusXRFaceExpression::LipFunnelerRB, FOculusXRExpressionCurves{ { FName("lipFunneler_RB") } } },
		{ EOculusXRFaceExpression::LipFunnelerRT, FOculusXRExpressionCurves{ { FName("lipFunneler_RT") } } },
		{ EOculusXRFaceExpression::LipPressorL, FOculusXRExpressionCurves{ { FName("lipPressor_L") } } },
		{ EOculusXRFaceExpression::LipPressorR, FOculusXRExpressionCurves{ { FName("lipPressor_R") } } },
		{ EOculusXRFaceExpression::LipPuckerL, FOculusXRExpressionCurves{ { FName("lipPucker_L") } } },
		{ EOculusXRFaceExpression::LipPuckerR, FOculusXRExpressionCurves{ { FName("lipPucker_R") } } },
		{ EOculusXRFaceExpression::LipStretcherL, FOculusXRExpressionCurves{ { FName("lipStretcher_L") } } },
		{ EOculusXRFaceExpression::LipStretcherR, FOculusXRExpressionCurves{ { FName("lipStretcher_R") } } },
		{ EOculusXRFaceExpression::LipSuckLB, FOculusXRExpressionCurves{ { FName("lipSuck_LB") } } },
		{ EOculusXRFaceExpression::LipSuckLT, FOculusXRExpressionCurves{ { FName("lipSuck_LT") } } },
		{ EOculusXRFaceExpression::LipSuckRB, FOculusXRExpressionCurves{ { FName("lipSuck_RB") } } },
		{ EOculusXRFaceExpression::LipSuckRT, FOculusXRExpressionCurves{ { FName("lipSuck_RT") } } },
		{ EOculusXRFaceExpression::LipTightenerL, FOculusXRExpressionCurves{ { FName("lipTightener_L") } } },
		{ EOculusXRFaceExpression::LipTightenerR, FOculusXRExpressionCurves{ { FName("lipTightener_R") } } },
		{ EOculusXRFaceExpression::LipsToward, FOculusXRExpressionCurves{ { FName("lipsToward") } } },
		{ EOculusXRFaceExpression::LowerLipDepressorL, FOculusXRExpressionCurves{ { FName("lowerLipDepressor_L") } } },
		{ EOculusXRFaceExpression::LowerLipDepressorR, FOculusXRExpressionCurves{ { FName("lowerLipDepressor_R") } } },
		{ EOculusXRFaceExpression::MouthLeft, FOculusXRExpressionCurves{ { FName("mouthLeft") } } },
		{ EOculusXRFaceExpression::MouthRight, FOculusXRExpressionCurves{ { FName("mouthRight") } } },
		{ EOculusXRFaceExpression::NoseWrinklerL, FOculusXRExpressionCurves{ { FName("noseWrinkler_L") } } },
		{ EOculusXRFaceExpression::NoseWrinklerR, FOculusXRExpressionCurves{ { FName("noseWrinkler_R") } } },
		{ EOculusXRFaceExpression::OuterBrowRaiserL, FOculusXRExpressionCurves{ { FName("outerBrowRaiser_L") } } },
		{ EOculusXRFaceExpression::OuterBrowRaiserR, FOculusXRExpressionCurves{ { FName("outerBrowRaiser_R") } } },
		{ EOculusXRFaceExpression::UpperLidRaiserL, FOculusXRExpressionCurves{ { FName("upperLidRaiser_L") } } },
		{ EOculusXRFaceExpression::UpperLidRaiserR, FOculusXRExpressionCurves{ { FName("upperLidRaiser_R") } } },
		{ EOculusXRFaceExpression::UpperLipRaiserL, FOculusXRExpressionCurves{ { FName("upperLipRaiser_L") } } },
		{ EOculusXRFaceExpression::UpperLipRaiserR, FOculusXRExpressionCurves{ { FName("upperLipRaiser_R") } } },
		{ EOculusXRFaceExpression::TongueTipInterdental, FOculusXRExpressionCurves{ { FName("tongueTipInterdental") } } },
		{ EOculusXRFaceExpression::TongueTipAlveolar, FOculusXRExpressionCurves{ { FName("tongueTipAlveolar") } } },
		{ EOculusXRFaceExpression::TongueFrontDorsalPalate, FOculusXRExpressionCurves{ { FName("tongueFrontDorsalPalate") } } },
		{ EOculusXRFaceExpression::TongueMidDorsalPalate, FOculusXRExpressionCurves{ { FName("tongueMidDorsalPalate") } } },
		{ EOculusXRFaceExpression::TongueBackDorsalVelar, FOculusXRExpressionCurves{ { FName("tongueBackDorsalVelar") } } },
		{ EOculusXRFaceExpression::TongueOut, FOculusXRExpressionCurves{ { FName("tongueOut") } } }
	};

	UPROPERTY(EditDefaultsOnly, Category = "OculusXR|FaceTracking")
	TMap<EOculusXRFaceExpression, FOculusXRFaceExpressionModifierNew> ExpressionModifiers;

private:
	USkeletalMeshComponent* SkeletalMeshComponent;
};
