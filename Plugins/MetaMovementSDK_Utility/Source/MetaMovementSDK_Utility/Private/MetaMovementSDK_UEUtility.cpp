/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "MetaMovementSDK_UEUtility.h"
#include <string>

#define LOCTEXT_NAMESPACE "FMetaMovementSDK_UtilityModule"

DEFINE_LOG_CATEGORY(LogMetaMovementSDK_Utility);

void FMetaMovementSDK_UtilityModule::StartupModule()
{
#if PLATFORM_WINDOWS
	FString DllPath = FPaths::Combine(
		FPaths::ProjectPluginsDir(),
		TEXT("MetaMovementSDK_Utility/Binaries/Win64/MetaMovementSDK_Utility.dll"));
	DllHandle = FPlatformProcess::GetDllHandle(*DllPath);
#elif PLATFORM_ANDROID
	DllHandle = FPlatformProcess::GetDllHandle(TEXT("libMetaMovementSDK_Utility.so"));
#else
	UE_LOG(LogTemp, Error, TEXT("Unsupported platform. Only Winodws and Android are supported for now."));
	return;
#endif
	if (!DllHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load the MSDK library."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("MSDK plugin loaded."));

	// Initialize the plugin to the engine coordinate space.
	if (metaMovementSDK_initialize(&MetaMovementSDK_UEUtility::GetEngineCoordinateSpace()) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Plugin is initialized - skipping setting coordinate space."));
	}
	UE_LOG(LogTemp, Log, TEXT("MSDK plugin initialized."));
}

void FMetaMovementSDK_UtilityModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (DllHandle)
	{
		FPlatformProcess::FreeDllHandle(DllHandle);
		DllHandle = nullptr;
	}
}

bool MetaMovementSDK_UEUtility::GetConfigName(metaMovementSDK_Handle handle, FString& outString)
{
	// Retrieve our Target config Name
	int nameBufferSize = 0;
	if (metaMovementSDK_getConfigName(handle, nullptr, &nameBufferSize) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to retrieve config name string size for handle %d"), handle);
		return false;
	}

	std::string targetConfigName(nameBufferSize, '\0');
	if (metaMovementSDK_getConfigName(handle, targetConfigName.data(), &nameBufferSize) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to retrieve target config name to Buffer for handle %d"), handle);
		return false;
	}

	// Use C-String to handle the termination character
	outString = FString(targetConfigName.c_str());
	return true;
}

bool MetaMovementSDK_UEUtility::GetJointNames(metaMovementSDK_Handle handle, metaMovementSDK_SkeletonType skeletonType, TArray<FString>& outJointNames)
{
	int bufferSize = 0;
	int jointNameCount = 0;
	if (metaMovementSDK_getJointNames(handle, skeletonType, nullptr, &bufferSize, nullptr, &jointNameCount) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to get joint name buffer size from %hs of handle %d"), META_MOVEMENTSDK_SKELETON_TYPES[skeletonType], handle);
		return false;
	}

	if (jointNameCount <= 0)
	{
		outJointNames.Empty();
		return true;
	}

	std::string buffer(bufferSize, '\0');
	std::vector<const char*> jointNames(jointNameCount, nullptr);
	if (metaMovementSDK_getJointNames(handle, skeletonType, buffer.data(), &bufferSize, jointNames.data(), &jointNameCount) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to get joint names from %hs of handle %d"), META_MOVEMENTSDK_SKELETON_TYPES[skeletonType], handle);
		return false;
	}

	// Fill the TArray with the Joint Names
	outJointNames.Empty();
	outJointNames.Reserve(jointNames.size());
	for (const char* jointName : jointNames)
	{
		outJointNames.Emplace(jointName != nullptr ? jointName : "");
	}
	return true;
}

bool MetaMovementSDK_UEUtility::GetJointName(metaMovementSDK_Handle handle, metaMovementSDK_SkeletonType skeletonType, metaMovementSDK_JointIndex jointIndex, FString& outName)
{
	int jointNameSize = 0;
	if (metaMovementSDK_getJointName(handle, skeletonType, jointIndex, nullptr, &jointNameSize) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to get joint name size from joint index %d of %hs"), jointIndex, META_MOVEMENTSDK_SKELETON_TYPES[skeletonType]);
		return false;
	}
	std::string jointName(jointNameSize, '\0');
	if (metaMovementSDK_getJointName(handle, skeletonType, jointIndex, jointName.data(), &jointNameSize) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to get joint name from joint index %d of %hs"), jointIndex, META_MOVEMENTSDK_SKELETON_TYPES[skeletonType]);
		return false;
	}

	// Use C-String to handle the termination character
	outName = FString(jointName.c_str());
	return true;
}

bool MetaMovementSDK_UEUtility::GetManifestationNames(
	metaMovementSDK_Handle handle,
	metaMovementSDK_SkeletonType skeletonType,
	TArray<FString>& outManifestationNames)
{
	int bufferSize = 0;
	int manifestationCount = 0;
	if (metaMovementSDK_getManifestationNames(handle, skeletonType, nullptr, &bufferSize, nullptr, &manifestationCount) != metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to get manifestation buffer size from %hs of handle %d"), META_MOVEMENTSDK_SKELETON_TYPES[skeletonType], handle);
		return false;
	}

	if (manifestationCount <= 0)
	{
		outManifestationNames.Empty();
		return true;
	}

	std::string buffer(bufferSize, '\0');
	std::vector<const char*> manifestationNames(manifestationCount, nullptr);
	if (metaMovementSDK_getManifestationNames(
			handle,
			skeletonType,
			buffer.data(),
			&bufferSize,
			manifestationNames.data(),
			&manifestationCount)
		!= metaMovementSDK_Result::Success)
	{
		UE_LOG(LogMetaMovementSDK_Utility, Error, TEXT("Failed to get manifestation names from %hs of handle %d"), META_MOVEMENTSDK_SKELETON_TYPES[skeletonType], handle);
		return false;
	}

	// Fill the TArray with the Joint Names
	outManifestationNames.Empty();
	outManifestationNames.Reserve(manifestationNames.size());
	for (const char* manifestation : manifestationNames)
	{
		outManifestationNames.Emplace(manifestation != nullptr ? manifestation : "");
	}
	return true;
}

bool MetaMovementSDK_UEUtility::WriteConfigDataToJSON(
	metaMovementSDK_Handle handle,
	FString& outJSONData,
	const metaMovementSDK_JointRelativeSpaceType JointRelativeSpace)
{
	if (IS_VALID_META_MOVEMENTSDK_HANDLE(handle))
	{
		int bufferSize = 0;
		if (metaMovementSDK_writeConfigDataToJSON(handle, nullptr, &JointRelativeSpace, nullptr, &bufferSize) == metaMovementSDK_Result::Success)
		{
			TArray<uint8> buffer;
			buffer.SetNum(bufferSize);
			if (metaMovementSDK_writeConfigDataToJSON(handle, nullptr, &JointRelativeSpace, (char*)buffer.GetData(), &bufferSize) == metaMovementSDK_Result::Success)
			{
				outJSONData = UTF8_TO_TCHAR((const ANSICHAR*)buffer.GetData());
				return true;
			}
		}
	}
	return false;
}

bool MetaMovementSDK_UEUtility::GetSkeletonInfo(
	metaMovementSDK_Handle handle,
	metaMovementSDK_SkeletonType skeletonType,
	metaMovementSDK_SkeletonInfo& out_skeletonInfo)
{
	return IS_VALID_META_MOVEMENTSDK_HANDLE(handle) && metaMovementSDK_getSkeletonInfo(handle, skeletonType, &out_skeletonInfo) == metaMovementSDK_Result::Success;
}

bool MetaMovementSDK_UEUtility::GetSkeletonTPose(
	metaMovementSDK_Handle handle,
	metaMovementSDK_SkeletonType skeletonType,
	metaMovementSDK_SkeletonTPoseType tPoseType,
	TArray<metaMovementSDK_Transform>& outTransformData,
	metaMovementSDK_JointRelativeSpaceType jointSpaceType)
{
	metaMovementSDK_SkeletonInfo skeletonInfo;
	if (GetSkeletonInfo(handle, skeletonType, skeletonInfo))
	{
		outTransformData.SetNum(skeletonInfo.jointCount);
		return metaMovementSDK_getSkeletonTPose(
				   handle,
				   skeletonType,
				   tPoseType,
				   jointSpaceType,
				   outTransformData.GetData(),
				   &skeletonInfo.jointCount)
			== metaMovementSDK_Result::Success;
	}
	return false;
}

bool MetaMovementSDK_UEUtility::GetLastProcessedFramePose(
	metaMovementSDK_Handle handle,
	metaMovementSDK_SkeletonType skeletonType,
	TArray<metaMovementSDK_Transform>& outTransformData,
	metaMovementSDK_JointRelativeSpaceType out_jointSpaceType)
{
	metaMovementSDK_SkeletonInfo skeletonInfo;
	if (GetSkeletonInfo(handle, skeletonType, skeletonInfo))
	{
		outTransformData.SetNum(skeletonInfo.jointCount);
		return metaMovementSDK_getLastProcessedFramePose(
				   handle,
				   skeletonType,
				   out_jointSpaceType,
				   outTransformData.GetData(),
				   &skeletonInfo.jointCount,
				   nullptr)
			== metaMovementSDK_Result::Success;
	}
	return false;
}

bool MetaMovementSDK_UEUtility::GetParentJointIndexes(
	metaMovementSDK_Handle handle,
	metaMovementSDK_SkeletonType skeletonType,
	TArray<metaMovementSDK_JointIndex>& outParentJointIndices)
{
	// NOTE: metaMovementSDK_getParentJointIndexes will update the jointCount
	// to the actual number of joints.  The function will fail if the jointCount
	// is too small, but will succeed and update the jointCount value if the
	// count is larger.  In this case, we want to return false since
	// the wrapper function assumes a correct sized array.
	metaMovementSDK_SkeletonInfo skeletonInfo;
	if (GetSkeletonInfo(handle, skeletonType, skeletonInfo))
	{
		outParentJointIndices.SetNum(skeletonInfo.jointCount);
		return metaMovementSDK_getParentJointIndexes(
				   handle,
				   skeletonType,
				   outParentJointIndices.GetData(),
				   &skeletonInfo.jointCount)
			== metaMovementSDK_Result::Success;
	}
	return false;
}

bool MetaMovementSDK_UEUtility::GetJointIndexesInHumanoidLimb(
	metaMovementSDK_Handle handle,
	metaMovementSDK_SkeletonType skeletonType,
	metaMovementSDK_HumanoidLimbType humanoidLimbType,
	TArray<metaMovementSDK_JointIndex>& outJointIndicesInHumanoidLimb)
{
	int limbJointCount = 0;
	if (metaMovementSDK_getJointIndexesInHumanoidLimb(
			handle,
			skeletonType,
			humanoidLimbType,
			nullptr,
			&limbJointCount)
		== metaMovementSDK_Result::Success)
	{
		outJointIndicesInHumanoidLimb.SetNum(limbJointCount);
		return metaMovementSDK_getJointIndexesInHumanoidLimb(
				   handle,
				   skeletonType,
				   humanoidLimbType,
				   outJointIndicesInHumanoidLimb.GetData(),
				   &limbJointCount)
			== metaMovementSDK_Result::Success;
	}
	return false;
}

bool MetaMovementSDK_UEUtility::GetSkeletonMappingTargetJoints(
	metaMovementSDK_Handle handle,
	TArray<metaMovementSDK_JointIndex>& outTargetJointIndices)
{
	int jointCount = 0;
	if (metaMovementSDK_getSkeletonMappingTargetJoints(handle, nullptr, &jointCount) == metaMovementSDK_Result::Success)
	{
		outTargetJointIndices.SetNum(jointCount);
		return metaMovementSDK_getSkeletonMappingTargetJoints(
				   handle,
				   outTargetJointIndices.GetData(),
				   &jointCount)
			== metaMovementSDK_Result::Success;
	}
	return false;
}

bool MetaMovementSDK_UEUtility::GetLastRetargetedMappingData(
	metaMovementSDK_Handle handle,
	metaMovementSDK_JointIndex targetJointIndex,
	metaMovementSDK_SkeletonType& outSourceSkeletonType,
	metaMovementSDK_Transform& outTPoseBlendedTransform,
	metaMovementSDK_Transform& outLastPoseBlendedTransform,
	TArray<metaMovementSDK_JointIndex> outSourceJointIndexList)
{
	int sourceJointCount = 0;
	if (metaMovementSDK_getLastRetargetedMappingData(
			handle,
			targetJointIndex,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&sourceJointCount)
		== Success)
	{
		outSourceJointIndexList.SetNum(sourceJointCount);
		return metaMovementSDK_getLastRetargetedMappingData(
				   handle,
				   targetJointIndex,
				   &outSourceSkeletonType,
				   &outTPoseBlendedTransform,
				   &outLastPoseBlendedTransform,
				   outSourceJointIndexList.GetData(),
				   &sourceJointCount)
			== Success;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMetaMovementSDK_UtilityModule, MetaMovementSDK_Utility)
