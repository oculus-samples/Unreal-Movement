/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OculusXRMovementTypes.h"
#include "OculusXRFacialExpressionMap.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct OCULUSXRRETARGETING_API FOculusXRFacialExpressionMapRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXRFacialExpressionMapRow")
	EOculusXRFaceExpression OculusXRFaceExpression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OculusXRFacialExpressionMapRow")
	FName CurveName;
};

UCLASS(BlueprintType, Category = "OculusXR")
class OCULUSXRRETARGETING_API UOculusXRFacialExpressionMap : public UDataTable
{
	GENERATED_BODY()
public:
};
