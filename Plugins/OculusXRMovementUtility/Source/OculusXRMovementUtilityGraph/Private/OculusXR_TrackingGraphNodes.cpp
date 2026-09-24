/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXR_TrackingGraphNodes.h"
#include "MetaMovementSDK_UEUtility.h"
#include "Misc/EnumRange.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "UJsonDataAsset.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UOculusXR_BodyTracking::AutoFillJointData()
{
	// First, Create a configuration in memory so we can process the skeleton/Pose
	metaMovementSDK_Handle referenceSkeletonHandle = Node.CreateTargetConfigFromSkeletonJoints(GetAnimBlueprint()->TargetSkeleton);

	if (!IS_VALID_META_MOVEMENTSDK_HANDLE(referenceSkeletonHandle))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create target config from skeleton data"));
		return;
	}

	metaMovementSDK_KnownJointIndexData knownJointData;
	if (metaMovementSDK_identifyKnownJointIndexesFromRestPose(
			referenceSkeletonHandle,
			metaMovementSDK_SkeletonType::TargetSkeleton,
			&knownJointData)
		== metaMovementSDK_Result::Success)
	{
		int bufferSize = 0;
		if (metaMovementSDK_getKnownJointNamesFromIndexData(
				referenceSkeletonHandle,
				metaMovementSDK_SkeletonType::TargetSkeleton,
				&knownJointData,
				nullptr,
				&bufferSize,
				nullptr)
			== metaMovementSDK_Result::Success)
		{
			metaMovementSDK_KnownJointNameData jointNames;
			std::vector<char> buffer(bufferSize);
			if (metaMovementSDK_getKnownJointNamesFromIndexData(
					referenceSkeletonHandle,
					metaMovementSDK_SkeletonType::TargetSkeleton,
					&knownJointData,
					buffer.data(),
					&bufferSize,
					&jointNames)
				== metaMovementSDK_Result::Success)
			{
				// Populate the Known Joint TMap in the editor with the retrieved strings
				for (int i = 0; i < static_cast<int>(metaMovementSDK_KnownJointType::KnownJointCount); ++i)
				{
					EOculusXRBodyKnownJointType mapKey = static_cast<EOculusXRBodyKnownJointType>(i);
					const FOculusXRBoneName* mapJointName = Node.KnownJoints.Find(mapKey);
					// Only overwrite a value if it doesn't exist in the map or is set
					// to an empty string
					if (jointNames.jointNameByType[i] != nullptr && *(jointNames.jointNameByType[i]) != '\0' && (mapJointName == nullptr || *mapJointName == NAME_None))
					{
						Node.KnownJoints.Emplace(mapKey, FOculusXRBoneName(jointNames.jointNameByType[i]));
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to get the joint names for the captured known joint index data."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to calculate the string buffer size for the known joint data."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to identify known joint indexes from RestPose"));
	}

	// Last, capture the likely ignore joints and populate the array
	TSet<FOculusXRBoneName> foundExcludeJoints = FAnimNode_OculusXRBodyTracking::IdentifyPossibleExcludeJointsFromConfig(referenceSkeletonHandle);
	for (const auto& iter : foundExcludeJoints)
	{
		Node.AutoMappingExcludeJoints.Emplace(iter);
	}

	// Make sure we don't leak the handle
	metaMovementSDK_destroy(referenceSkeletonHandle);

	MarkPackageDirty();

	// Mark the AnimBlueprint as modified and reconstruct the node
	// This ensures runtime AnimNode instances see the changes immediately
	if (UBlueprint* Blueprint = GetAnimBlueprint())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	}
	ReconstructNode();
}

void UOculusXR_BodyTracking::WriteConfigData()
{
	FString SaveFilePath;
	if (!GetSaveFilePathFromJSONSaveDialog(SaveFilePath))
	{
		return;
	}

	metaMovementSDK_Handle configHandle = Node.GenerateRetargetingConfig(GetAnimBlueprint()->TargetSkeleton);
	FString JsonString;
	if (MetaMovementSDK_UEUtility::WriteConfigDataToJSON(configHandle, JsonString) && FFileHelper::SaveStringToFile(JsonString, *SaveFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully exported retargeting config to: %s"), *SaveFilePath);

		// Auto-import the saved .jsondata file as a UAsset
		if (FPaths::IsUnderDirectory(SaveFilePath, FPaths::ProjectContentDir()))
		{
			// Get the asset tools module
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

			// Convert the absolute file path to a package path
			FString PackagePath;
			if (FPackageName::TryConvertFilenameToLongPackageName(SaveFilePath, PackagePath))
			{
				// Get the destination directory path and asset name
				FString DestinationPath = FPaths::GetPath(PackagePath);
				FString AssetName = FPaths::GetBaseFilename(SaveFilePath);

				// Create import task
				UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
				ImportTask->Filename = SaveFilePath;
				ImportTask->DestinationPath = DestinationPath;
				ImportTask->DestinationName = AssetName;
				ImportTask->bAutomated = true;
				ImportTask->bReplaceExisting = true;
				ImportTask->bSave = true;

				// Execute import
				TArray<UAssetImportTask*> ImportTasks;
				ImportTasks.Add(ImportTask);
				AssetTools.ImportAssetTasks(ImportTasks);

				if (ImportTask->GetObjects().Num() > 0)
				{
					UE_LOG(LogTemp, Log, TEXT("Successfully auto-imported asset: %s"), *ImportTask->GetObjects()[0]->GetPathName());

					// Populate the JsonConfigAsset field with the imported asset
					UJsonDataAsset* ImportedJsonAsset = Cast<UJsonDataAsset>(ImportTask->GetObjects()[0]);
					if (ImportedJsonAsset)
					{
						// Set the soft object pointer to the imported asset
						Node.JsonConfigAsset = TSoftObjectPtr<UJsonDataAsset>(ImportedJsonAsset);

						MarkPackageDirty();

						// Mark the AnimBlueprint as modified and reconstruct the node
						// This ensures runtime AnimNode instances see the change immediately
						if (UBlueprint* Blueprint = GetAnimBlueprint())
						{
							FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
						}
						ReconstructNode();
						UE_LOG(LogTemp, Log, TEXT("Successfully set JsonConfigAsset to: %s"), *ImportedJsonAsset->GetName());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Imported asset is not a UJsonDataAsset"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Auto-import failed for: %s. You can manually import it by dragging it into the Content Browser."), *SaveFilePath);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Could not convert file path to package path: %s"), *SaveFilePath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("File saved outside Content directory: %s. Auto-import only works for files in the Content folder."), *SaveFilePath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save file to: %s"), *SaveFilePath);
	}

	// Make sure we don't leak the handle
	metaMovementSDK_destroy(configHandle);
}

void UOculusXR_BodyTracking::ExportSkeletonRestPose()
{
	FString SaveFilePath;
	if (!GetSaveFilePathFromJSONSaveDialog(SaveFilePath, "_RestPose"))
	{
		return;
	}

	metaMovementSDK_Handle referenceSkeletonHandle = Node.CreateTargetConfigFromSkeletonJoints(GetAnimBlueprint()->TargetSkeleton);
	FString JsonString;
	if (MetaMovementSDK_UEUtility::WriteConfigDataToJSON(referenceSkeletonHandle, JsonString) && FFileHelper::SaveStringToFile(JsonString, *SaveFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully exported skeleton rest pose to: %s"), *SaveFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save file to: %s"), *SaveFilePath);
	}

	// Make sure we don't leak the handle
	metaMovementSDK_destroy(referenceSkeletonHandle);
}

bool UOculusXR_BodyTracking::GetSaveFilePathFromJSONSaveDialog(FString& outFilePath, const FString& suffix)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Desktop Platform"));
		return false;
	}

	// Load last used directory from config, or default to Saved directory
	FString LastDirectory = FPaths::ProjectSavedDir();
	GConfig->GetString(TEXT("OculusXRBodyTracking"), TEXT("LastExportDirectory"), LastDirectory, GEditorPerProjectIni);

	TArray<FString> SaveFilenames;
	const FString DefaultFileName = GetAnimBlueprint()->TargetSkeleton->GetName() + suffix + ".jsondata";
	const FString FileTypes = TEXT("JSON Data Files (*.jsondata)|*.jsondata");

	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	bool bFileSelected = DesktopPlatform->SaveFileDialog(
		ParentWindowHandle,
		TEXT("Export Skeleton Rest Pose"),
		LastDirectory,
		DefaultFileName,
		FileTypes,
		EFileDialogFlags::None,
		SaveFilenames);

	if (!bFileSelected || SaveFilenames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No file selected for export"));
		return false;
	}

	outFilePath = SaveFilenames[0];

	// Save the directory for next time
	FString SelectedDirectory = FPaths::GetPath(SaveFilenames[0]);
	GConfig->SetString(TEXT("OculusXRBodyTracking"), TEXT("LastExportDirectory"), *SelectedDirectory, GEditorPerProjectIni);

	return true;
}

void UOculusXR_BodyTracking::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UOculusXR_BodyTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("OculusXR Body Tracking");
}

FText UOculusXR_BodyTracking::GetTooltipText() const
{
	return FText::FromString("This node is responsible for receiving the body tracking data from the HMD and applying it to the skeleton.");
}

FString UOculusXR_BodyTracking::GetNodeCategory() const
{
	return FString("OculusXR Body Tracking");
}

void UOculusXR_FaceTracking::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UOculusXR_FaceTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("OculusXR Face Tracking");
}

FText UOculusXR_FaceTracking::GetTooltipText() const
{
	return FText::FromString("This node is responsible for receiving the face tracking data from the HMD and applying it to the skeleton.");
}

FString UOculusXR_FaceTracking::GetNodeCategory() const
{
	return FString("OculusXR Face Tracking");
}

void UOculusXR_EyeTracking::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UOculusXR_EyeTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString("OculusXR Eye Tracking");
}

FText UOculusXR_EyeTracking::GetTooltipText() const
{
	return FText::FromString("This node is responsible for receiving the eye tracking data from the HMD and applying it to the skeleton.");
}

FString UOculusXR_EyeTracking::GetNodeCategory() const
{
	return FString("OculusXR Eye Tracking");
}
