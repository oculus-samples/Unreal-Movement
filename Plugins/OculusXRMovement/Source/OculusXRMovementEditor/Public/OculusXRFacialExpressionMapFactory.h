/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"

#include "OculusXRFacialExpressionMapFactory.generated.h"

UCLASS(Category = "OculusXR")
class OCULUSXRRETARGETINGEDITOR_API UOculusXRFacialExpressionMapFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<const class UScriptStruct> Struct;

	UOculusXRFacialExpressionMapFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	virtual FText GetDisplayName() const override;
};
