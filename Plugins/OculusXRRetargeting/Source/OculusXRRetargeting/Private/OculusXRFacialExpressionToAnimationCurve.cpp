/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRFacialExpressionToAnimationCurve.h"
#include "Animation/AnimInstanceProxy.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OculusXRFacialExpressionToAnimationCurve)

void FOculusXRFacialExpressionToAnimationCurve::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)

	FAnimNode_Base::Initialize_AnyThread(Context);

	BasePose.Initialize(Context);

	const USkeleton* Skeleton = Context.AnimInstanceProxy->GetSkeleton();
	if (IsValid(OculusExpressionMapping) && IsValid(Skeleton))
	{
		const TMap<FName, uint8*>& RowMap = OculusExpressionMapping->GetRowMap();

		for (const auto& RowPair : RowMap)
		{
			FOculusXRFacialExpressionMapRow* Row = reinterpret_cast<FOculusXRFacialExpressionMapRow*>(RowPair.Value);

			FName CurveName = Row->CurveName;
			FSmartName CurveSmartName;
			Skeleton->GetSmartNameByName(USkeleton::AnimCurveMappingName, CurveName, CurveSmartName);
			NameToSmartName.Add(CurveName, CurveSmartName);

			CurveValues.Add(CurveName, 0.0f);
		}
	}
}

void FOculusXRFacialExpressionToAnimationCurve::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)

	BasePose.CacheBones(Context);
}

void FOculusXRFacialExpressionToAnimationCurve::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)

	BasePose.Update(Context);

	FAnimNode_Base::Update_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FOculusXRFacialExpressionToAnimationCurve::PreUpdate(const UAnimInstance* InAnimInstance)
{
	FaceTrackingComponent = Cast<UOculusXRFaceTrackingComponent>(InAnimInstance->GetOwningComponent()->GetOwner()->FindComponentByClass(UOculusXRFaceTrackingComponent::StaticClass()));
	if (IsValid(FaceTrackingComponent) && IsValid(OculusExpressionMapping))
	{
		const TMap<FName, uint8*>& RowMap = OculusExpressionMapping->GetRowMap();

		for (const auto& RowPair : RowMap)
		{
			FOculusXRFacialExpressionMapRow* Row = reinterpret_cast<FOculusXRFacialExpressionMapRow*>(RowPair.Value);

			EOculusXRFaceExpression Expression = Row->OculusXRFaceExpression;
			FName CurveName = Row->CurveName;
			if (NameToSmartName.Contains(CurveName))
				CurveValues.Add(CurveName, FaceTrackingComponent->GetExpressionValue(Expression));
		}
	}
}

void FOculusXRFacialExpressionToAnimationCurve::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)

	// Evaluate the base pose
	BasePose.Evaluate(Output);

	const USkeleton* Skeleton = Output.AnimInstanceProxy->GetSkeleton();
	if (IsValid(Skeleton))
	{
		for (const auto& Pair : CurveValues)
		{
			FName CurveName = Pair.Key;
			if (NameToSmartName.Contains(CurveName))
				Output.Curve.Set(NameToSmartName[CurveName].UID, Pair.Value);
		}
	}
}
