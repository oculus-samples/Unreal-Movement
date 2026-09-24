/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "UJsonDataAssetFactory.h"
#include "UJsonDataAsset.h"
#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UJsonDataAssetFactory)

#define LOCTEXT_NAMESPACE "UJsonDataAssetFactory"

UJsonDataAssetFactory::UJsonDataAssetFactory()
{
	// This factory is for importing files, not creating new assets from scratch
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	bText = true;

	SupportedClass = UJsonDataAsset::StaticClass();

	// Standard priority
	ImportPriority = 0;

	// Only support .jsondata extension to avoid conflicts with DataTable factory
	// The DataTable factory has special import dialog handling for .json files
	// that bypasses normal factory selection, so we use .jsondata exclusively
	Formats.Add(TEXT("jsondata;JSON Data Asset File"));
}

UObject* UJsonDataAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	bOutOperationCanceled = false;

	// Load the JSON file content
	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *Filename))
	{
		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load JSON file: %s"), *Filename);
		}
		bOutOperationCanceled = true;
		return nullptr;
	}

	// Create the new UJsonDataAsset
	UJsonDataAsset* NewAsset = NewObject<UJsonDataAsset>(InParent, InClass, InName, Flags);

	if (NewAsset)
	{
		// Store the JSON content
		NewAsset->JsonText = JsonContent;

		// Set up asset import data for reimport support
		NewAsset->AssetImportData->Update(Filename);

		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Log, TEXT("Successfully imported JSON file: %s"), *Filename);
		}
	}

	return NewAsset;
}

bool UJsonDataAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == UJsonDataAsset::StaticClass();
}

bool UJsonDataAssetFactory::FactoryCanImport(const FString& Filename)
{
	// Only support .jsondata extension
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension.Equals(TEXT("jsondata"), ESearchCase::IgnoreCase);
}

FText UJsonDataAssetFactory::GetDisplayName() const
{
	return LOCTEXT("UJsonDataAssetFactoryDescription", "JSON Data Asset");
}

#undef LOCTEXT_NAMESPACE
