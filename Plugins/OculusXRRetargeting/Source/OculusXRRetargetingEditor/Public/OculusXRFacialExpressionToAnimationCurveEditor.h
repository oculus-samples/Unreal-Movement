/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraphNode_Base.h"
#include "OculusXRFacialExpressionToAnimationCurve.h"
#include "OculusXRFacialExpressionToAnimationCurveEditor.generated.h"

class FPrimitiveDrawInterface;
class USkeletalMeshComponent;

UCLASS()
class OCULUSXRRETARGETINGEDITOR_API UOculusXRFacialExpressionToAnimationCurveEditor : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FOculusXRFacialExpressionToAnimationCurve Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FEditorModeID GetEditorMode() const override;

	static const FName AnimModeName;
};
