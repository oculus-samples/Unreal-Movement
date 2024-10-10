/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRFacialExpressionMapFactory.h"
#include "OculusXRFacialExpressionMap.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OculusXRFacialExpressionMapFactory)

#define LOCTEXT_NAMESPACE "OculusXRFacialExpressionMapFactory"

UOculusXRFacialExpressionMapFactory::UOculusXRFacialExpressionMapFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UOculusXRFacialExpressionMap::StaticClass();
	Struct = FOculusXRFacialExpressionMapRow::StaticStruct();
}

UObject* UOculusXRFacialExpressionMapFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UOculusXRFacialExpressionMap* NewCustomDataTable = NewObject<UOculusXRFacialExpressionMap>(InParent, InClass, InName, Flags | RF_Transactional);
	NewCustomDataTable->RowStruct = const_cast<UScriptStruct*>(ToRawPtr(Struct));
	return NewCustomDataTable;
}

FText UOculusXRFacialExpressionMapFactory::GetDisplayName() const
{
	return LOCTEXT("OculusXRFacialExpressionMapFactoryDescription", "OculusXR Facial Expression Map Factory");
}

#undef LOCTEXT_NAMESPACE
