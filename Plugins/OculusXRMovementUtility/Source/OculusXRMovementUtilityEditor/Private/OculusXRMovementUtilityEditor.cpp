/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRMovementUtilityEditor.h"
#include "OculusXRBoneNameCustomization.h"
#include "OculusXRBoneName.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "PropertyEditorModule.h"

IMPLEMENT_MODULE(FOculusXRMovementUtilityEditor, OculusXRMovementUtilityEditor);

DEFINE_LOG_CATEGORY(OculusXRMovementUtilityEditor)

#define LOCTEXT_NAMESPACE "OculusXRMovementEditor"

void FOculusXRMovementUtilityEditor::StartupModule()
{
	UE_LOG(OculusXRMovementUtilityEditor, Warning, TEXT("OculusXR Movement Editor: Log Started"));
	UE_LOG(OculusXRMovementUtilityEditor, Log, TEXT("UJsonDataAssetFactory is available for .json and .jsondata file imports"));

	// Register custom property type customization for bone name wrapper
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FOculusXRBoneName::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FOculusXRBoneNameCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FOculusXRMovementUtilityEditor::ShutdownModule()
{
	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FOculusXRBoneName::StaticStruct()->GetFName());
	}

	UE_LOG(OculusXRMovementUtilityEditor, Warning, TEXT("OculusXR Movement: Log Ended"));
}

#undef LOCTEXT_NAMESPACE
