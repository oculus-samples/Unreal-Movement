/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "UJsonDataAsset.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "EditorFramework/AssetImportData.h"
#endif

UJsonDataAsset::UJsonDataAsset()
{
#if WITH_EDITORONLY_DATA
	AssetImportData = CreateDefaultSubobject<UAssetImportData>(TEXT("AssetImportData"));
#endif
}

#if WITH_EDITOR
void UJsonDataAsset::LoadFromJson()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		// Load last used directory from config, or default to Content directory
		FString LastDirectory = FPaths::ProjectContentDir();
		GConfig->GetString(TEXT("JsonDataAsset"), TEXT("LastLoadDirectory"), LastDirectory, GEditorPerProjectIni);

		void* ParentWindowHandle = nullptr;
		FString FileTypes = TEXT("JSON Files (*.json;*.jsondata)|*.json;*.jsondata|All Files (*.*)|*.*");
		TArray<FString> OutFiles;
		bool bOpened = DesktopPlatform->OpenFileDialog(
			ParentWindowHandle,
			TEXT("Choose JSON File"),
			LastDirectory,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::None,
			OutFiles);

		if (bOpened && OutFiles.Num() > 0)
		{
			FString LoadedText;
			if (FFileHelper::LoadFileToString(LoadedText, *OutFiles[0]))
			{
				JsonText = LoadedText;
#if WITH_EDITORONLY_DATA
				if (AssetImportData)
				{
					AssetImportData->Update(OutFiles[0]);
				}
				MarkPackageDirty();
#endif
				// Save the directory for next time
				FString SelectedDirectory = FPaths::GetPath(OutFiles[0]);
				GConfig->SetString(TEXT("JsonDataAsset"), TEXT("LastLoadDirectory"), *SelectedDirectory, GEditorPerProjectIni);
			}
		}
	}
}

void UJsonDataAsset::SaveToJson()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		// Load last used directory from config, or default to Content directory
		FString LastDirectory = FPaths::ProjectContentDir();
		GConfig->GetString(TEXT("JsonDataAsset"), TEXT("LastSaveDirectory"), LastDirectory, GEditorPerProjectIni);

		void* ParentWindowHandle = nullptr;
		FString FileTypes = TEXT("JSON Data Files (*.jsondata)|*.jsondata|JSON Files (*.json)|*.json|All Files (*.*)|*.*");
		TArray<FString> SaveFilenames;
		FString DefaultFileName = GetName() + TEXT(".jsondata");

		bool bFileSelected = DesktopPlatform->SaveFileDialog(
			ParentWindowHandle,
			TEXT("Save JSON Data"),
			LastDirectory,
			DefaultFileName,
			FileTypes,
			EFileDialogFlags::None,
			SaveFilenames);

		if (bFileSelected && SaveFilenames.Num() > 0)
		{
			if (FFileHelper::SaveStringToFile(JsonText, *SaveFilenames[0]))
			{
				UE_LOG(LogTemp, Log, TEXT("Successfully saved JSON data to: %s"), *SaveFilenames[0]);

				// Save the directory for next time
				FString SelectedDirectory = FPaths::GetPath(SaveFilenames[0]);
				GConfig->SetString(TEXT("JsonDataAsset"), TEXT("LastSaveDirectory"), *SelectedDirectory, GEditorPerProjectIni);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save JSON data to: %s"), *SaveFilenames[0]);
			}
		}
	}
}

void UJsonDataAsset::RefreshFromSource()
{
#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		// Get the source file path from AssetImportData
		FString SourceFilePath = AssetImportData->GetFirstFilename();

		if (SourceFilePath.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("No source file associated with this asset. Use 'Load From Json' to set a source file."));
			return;
		}

		if (!FPaths::FileExists(SourceFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Source file does not exist: %s"), *SourceFilePath);
			return;
		}

		// Load the file content
		FString LoadedText;
		if (FFileHelper::LoadFileToString(LoadedText, *SourceFilePath))
		{
			JsonText = LoadedText;
			MarkPackageDirty();
			UE_LOG(LogTemp, Log, TEXT("Successfully refreshed JSON data from: %s"), *SourceFilePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to read source file: %s"), *SourceFilePath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AssetImportData is null. Cannot refresh from source."));
	}
#endif
}

void UJsonDataAsset::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (!AssetImportData)
		{
			AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
		}
	}
#endif
}

void UJsonDataAsset::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);
#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		Context.AddTag(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif
}
#endif
