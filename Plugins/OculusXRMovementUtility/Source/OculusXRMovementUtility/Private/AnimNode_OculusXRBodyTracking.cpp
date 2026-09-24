/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "AnimNode_OculusXRBodyTracking.h"
#include "OculusXRMovement.h"
#include "OculusXRMovementUtility.h"
#include "Animation/AnimInstanceProxy.h"
#include <MetaMovementSDK_UEUtility.h>

#include "Kismet/GameplayStatics.h"

const FCompactPoseBoneIndex FAnimNode_OculusXRBodyTracking::kInvalidBoneIndex(INDEX_NONE);

FAnimNode_OculusXRBodyTracking::~FAnimNode_OculusXRBodyTracking()
{
	ConfigInfo.Clear();
}

void FAnimNode_OculusXRBodyTracking::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	InputPose.Initialize(Context);

	// This animation node is executed during the packaging step.
	// During that time, the MetaXR plugin is not available and any calls to it will crash the editor,
	// preventing the packaging process from completing.
	// To avoid this, we check if the plugin is available before calling any of its functions.
	if (!GEngine || !GEngine->XRSystem.IsValid())
	{
		UE_LOG(LogOculusXRMovementUtility, Warning, TEXT("XR tracking is not loaded and available. Cannot retarget body at this time."));
		return;
	}

	SkeletalMeshComponent = Context.AnimInstanceProxy->GetSkelMeshComponent();
	if (SkeletalMeshComponent == nullptr)
	{
		UE_LOG(LogOculusXRMovementUtility, Warning, TEXT("SkeletalMeshComponent is null"));
	}

	// Initialize the skeleton cache
	CachedUSkeleton = Context.AnimInstanceProxy->GetSkeleton();
	if (CachedUSkeleton == nullptr)
	{
		UE_LOG(LogOculusXRMovementUtility, Warning, TEXT("Skeleton is null"));
	}
}

void FAnimNode_OculusXRBodyTracking::PreUpdate(const UAnimInstance* InAnimInstance)
{
	if (IsDebugDrawEnabled())
	{
		APawn* owningPawn = InAnimInstance->TryGetPawnOwner();
		if (owningPawn)
		{
			APlayerController* playerController =
				UGameplayStatics::GetPlayerController(owningPawn->GetWorld(), 0);
			if (playerController && playerController->PlayerCameraManager)
			{
				IsUserPlayer = (playerController->PlayerCameraManager->GetViewTarget() == owningPawn);
			}
		}
	}
}

void FAnimNode_OculusXRBodyTracking::Evaluate_AnyThread(FPoseContext& Output)
{
	InputPose.Evaluate(Output);
	// This animation node is executed during the packaging step.
	// During that time, the MetaXR plugin is not available and any calls to it will crash the editor,
	// preventing the packaging process from completing.
	// To avoid this, we check if the plugin is available before calling any of its functions.
	if (!GEngine || !GEngine->XRSystem.IsValid())
	{
		UE_LOG(LogOculusXRMovementUtility, Warning, TEXT("XR tracking is not loaded and available. Cannot retarget body at this time."));
		return;
	}

	if (SkeletalMeshComponent == nullptr)
	{
		return;
	}

	UpdateRetargeting(Output.Pose);
}

void FAnimNode_OculusXRBodyTracking::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	InputPose.Update(Context);
	// Evaluate pin inputs
	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FAnimNode_OculusXRBodyTracking::UpdateRetargeting(FCompactPose& Pose)
{
	if (!ConfigInfo.IsValid())
	{
		const FBoneContainer& targetSkeletonBoneContainer = Pose.GetBoneContainer();
		CreateRetargetingHandle(targetSkeletonBoneContainer);
	}

	if (ConfigInfo.IsValid())
	{
		// This will reinitialize if the bone container serial has changed
		if (ConfigInfo.ReInitializeBoneContainerIfNecessary(Pose.GetBoneContainer()))
		{
			SkeletonInfo.Initialize();
		}

		bool retargetingCalculatedSuccessfully = ProcessMovementSDKBodyPose();

		TArray<metaMovementSDK_Transform>& TargetTransformCache = ConfigInfo.GetTransformCache(metaMovementSDK_SkeletonType::TargetSkeleton);

		// Check to see if we need to override and use the RestPose
		if (DebugPoseMode == EOculusXRBodyDebugPoseMode::RestPose && MetaMovementSDK_UEUtility::GetSkeletonTPose(ConfigInfo.GetHandle(), metaMovementSDK_SkeletonType::TargetSkeleton, metaMovementSDK_SkeletonTPoseType::CurrentTPose, TargetTransformCache))
		{
			retargetingCalculatedSuccessfully = true;
		}

		// Retargeting could fail for several reasons (many valid).
		// If this happens, use the last frame data
		if (!retargetingCalculatedSuccessfully)
		{
			int targetJointCount = TargetTransformCache.Num();
			retargetingCalculatedSuccessfully =
				metaMovementSDK_getLastProcessedFramePose(
					ConfigInfo.GetHandle(),
					metaMovementSDK_SkeletonType::TargetSkeleton,
					kMSDKJointSpaceOutputType,
					TargetTransformCache.GetData(),
					&targetJointCount,
					nullptr)
				== metaMovementSDK_Result::Success;
		}

		// Fallback - grab the unscaled Target rest pose for the frame
		if (!retargetingCalculatedSuccessfully && ShouldUseFallbackPose())
		{
			int targetJointCount = TargetTransformCache.Num();
			if (metaMovementSDK_getSkeletonTPose(
					ConfigInfo.GetHandle(),
					metaMovementSDK_SkeletonType::TargetSkeleton,
					metaMovementSDK_SkeletonTPoseType::UnscaledTPose,
					kMSDKJointSpaceOutputType,
					TargetTransformCache.GetData(),
					&targetJointCount)
				== metaMovementSDK_Result::Success)
			{
				retargetingCalculatedSuccessfully = true;
			}
		}

		if (retargetingCalculatedSuccessfully)
		{
			// Apply the Pose
			FCSPose<FCompactPose> MeshPoses;
			MeshPoses.InitPose(Pose);

			// Populate the MeshPoses structure with our output
			const TArray<FCompactPoseBoneIndex>& targetIndexArray = ConfigInfo.GetTargetIndexArray();
			const FTransform& ConfigToComponentSpace = ConfigInfo.GetConfigToComponentSpaceTransform();
			for (int i = 0; i < TargetTransformCache.Num(); ++i)
			{
				if (targetIndexArray[i].IsValid())
				{
					// Multiply the Transform against ConfigInfo.GetConfigToComponentSpaceTransform()
					// To transform back to component space.
					MeshPoses.SetComponentSpaceTransform(
						targetIndexArray[i],
						MetaMovementSDK_UEUtility::ToFTransform(TargetTransformCache[i]) * ConfigToComponentSpace);
				}
			}

			// Apply back to the Compact Pose for the frame
			FCSPose<FCompactPose>::ConvertComponentPosesToLocalPosesSafe(MeshPoses, Pose);

			UpdateDebugDraw();
		}
	}
}

void FAnimNode_OculusXRBodyTracking::UpdateDebugDraw()
{
	// Debug Draw
	if (!DebugDraw)
	{
		DebugDraw = TSharedPtr<OculusXRMovementUtility_DebugDraw>(reinterpret_cast<OculusXRMovementUtility_DebugDraw*>(new OculusXRMovementUtility_DebugDraw()));
	}

	// SkeletalMeshComponent could only be null if this private function is called from the wrong place.
	check(SkeletalMeshComponent != nullptr);
	if (IsDebugDrawEnabled() && SkeletalMeshComponent != nullptr)
	{
		TArray<metaMovementSDK_Transform>& SourceTransformCache = ConfigInfo.GetTransformCache(metaMovementSDK_SkeletonType::SourceSkeleton);
		TArray<metaMovementSDK_Transform>& TargetTransformCache = ConfigInfo.GetTransformCache(metaMovementSDK_SkeletonType::TargetSkeleton);

		bool validSource = false;
		bool validTarget = false;

		TSet<metaMovementSDK_JointIndex> SourceJointsToSkipDebugDrawRender;
		TSet<metaMovementSDK_JointIndex> TargetJointsToSkipDebugDrawRender(ConfigInfo.GetTargetJointsMissingFromCurrentSkeletonLOD());

		switch (DebugDrawMode)
		{
			case EOculusXRBodyDebugDrawMode::RestPose:
			case EOculusXRBodyDebugDrawMode::RestPoseWithMapping:
				validSource = MetaMovementSDK_UEUtility::GetSkeletonTPose(ConfigInfo.GetHandle(), metaMovementSDK_SkeletonType::SourceSkeleton, metaMovementSDK_SkeletonTPoseType::CurrentTPose, SourceTransformCache);
				validTarget = MetaMovementSDK_UEUtility::GetSkeletonTPose(ConfigInfo.GetHandle(), metaMovementSDK_SkeletonType::TargetSkeleton, metaMovementSDK_SkeletonTPoseType::CurrentTPose, TargetTransformCache);
				break;
			case EOculusXRBodyDebugDrawMode::FramePose:
			case EOculusXRBodyDebugDrawMode::FramePoseWithMapping:
				validSource = MetaMovementSDK_UEUtility::GetLastProcessedFramePose(ConfigInfo.GetHandle(), metaMovementSDK_SkeletonType::SourceSkeleton, SourceTransformCache);
				validTarget = MetaMovementSDK_UEUtility::GetLastProcessedFramePose(ConfigInfo.GetHandle(), metaMovementSDK_SkeletonType::TargetSkeleton, TargetTransformCache);

				if (IsUserPlayer)
				{
					// If this is the user player (1P Camera), add the head and face joints to
					// the SkipDebugDrawSets so they don't interfere with the camera
					TArray<metaMovementSDK_JointIndex> sourceHeadAndFaceJoints;
					MetaMovementSDK_UEUtility::GetJointIndexesInHumanoidLimb(
						ConfigInfo.GetHandle(),
						metaMovementSDK_SkeletonType::SourceSkeleton,
						metaMovementSDK_HumanoidLimbType::HeadAndFaceLimb,
						sourceHeadAndFaceJoints);

					for (metaMovementSDK_JointIndex jointIndex : sourceHeadAndFaceJoints)
					{
						SourceJointsToSkipDebugDrawRender.Add(jointIndex);
					}

					TArray<metaMovementSDK_JointIndex> targetHeadAndFaceJoints;
					MetaMovementSDK_UEUtility::GetJointIndexesInHumanoidLimb(
						ConfigInfo.GetHandle(),
						metaMovementSDK_SkeletonType::TargetSkeleton,
						metaMovementSDK_HumanoidLimbType::HeadAndFaceLimb,
						targetHeadAndFaceJoints);

					for (metaMovementSDK_JointIndex jointIndex : targetHeadAndFaceJoints)
					{
						TargetJointsToSkipDebugDrawRender.Add(jointIndex);
					}
				}
				break;
			default:
				break;
		}

		const FTransform componentAndConfigTransformToApply = ConfigInfo.GetConfigToComponentSpaceTransform() * SkeletalMeshComponent->GetComponentTransform();
		if (validSource)
		{
			DebugDraw->AddSkeleton(
				ConfigInfo.GetHandle(),
				metaMovementSDK_SkeletonType::SourceSkeleton,
				SourceTransformCache,
				SourceJointsToSkipDebugDrawRender,
				componentAndConfigTransformToApply,
				FColor::Yellow);
		}
		if (validTarget)
		{
			DebugDraw->AddSkeleton(
				ConfigInfo.GetHandle(),
				metaMovementSDK_SkeletonType::TargetSkeleton,
				TargetTransformCache,
				TargetJointsToSkipDebugDrawRender,
				componentAndConfigTransformToApply,
				FColor::Green);
		}

		if ((DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPoseWithMapping || DebugDrawMode == EOculusXRBodyDebugDrawMode::FramePoseWithMapping) && validSource && validTarget)
		{
			DebugDraw->AddSkeletonMappings(
				ConfigInfo.GetHandle(),
				SourceTransformCache,
				TargetTransformCache,
				componentAndConfigTransformToApply,
				FColor::White,
				DebugDrawMode == EOculusXRBodyDebugDrawMode::RestPoseWithMapping);
		}
	}
}

bool FAnimNode_OculusXRBodyTracking::ProcessMovementSDKBodyPose()
{
	// Use the Body State to Retarget using our retargeting config handle
	FOculusXRBodyState BodyState;
	OculusXRMovement::GetBodyState(BodyState);
	if (BodyState.IsActive)
	{
		TArray<metaMovementSDK_Transform>& SourceTransformCache = ConfigInfo.GetTransformCache(metaMovementSDK_SkeletonType::SourceSkeleton);

		// Extract the BodyState joints to the transform cache
		MsdkTransformArrayFromXRBodyState(BodyState, SourceTransformCache);
		FOculusXRBodySkeleton frameSkeleton;
		bool bodySkeletonRetrieved = OculusXRMovement::GetBodySkeleton(frameSkeleton);
		if (bodySkeletonRetrieved && frameSkeleton.NumBones == SourceTransformCache.Num() && SkeletonInfo.UpdateFrameCounts(BodyState.SkeletonChangedCount, frameSkeleton.NumBones))
		{
			// Update the Skeleton Data
			// NOTE: Use a different Transform Cache since we're comparing counts to our BodyState
			TArray<metaMovementSDK_Transform>& SourceTPoseTransformCache = ConfigInfo.GetTPoseTransformCache();
			MsdkTransformArrayFromXRBodySkeleton(frameSkeleton, SourceTPoseTransformCache);
			if (SourceTPoseTransformCache.Num() > 0)
			{
				// Pattern to ensure the string cast stays in scope on Android
				std::string convertedManifestationName;
				if (ConfigInfo.GetSourceJointCountToManifestationTable().Contains(SourceTPoseTransformCache.Num()))
				{
					convertedManifestationName = TCHAR_TO_ANSI(*(ConfigInfo.GetSourceJointCountToManifestationTable()[SourceTPoseTransformCache.Num()]));
				}
				const char* manifestation = convertedManifestationName.empty() ? nullptr : convertedManifestationName.c_str();

				if (metaMovementSDK_updateSourceReferenceTPose(
						ConfigInfo.GetHandle(),
						SourceTPoseTransformCache.GetData(),
						SourceTPoseTransformCache.Num(),
						manifestation)
					!= metaMovementSDK_Result::Success)
				{
					UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Unable to update source reference T Pose with %d joints"), SourceTPoseTransformCache.Num());
				}
			}
		}
		else if (!bodySkeletonRetrieved)
		{
			UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Could not retrieve Body Skeleton from Source Tracking Data"));
			SkeletonInfo.ResetSourceChangeCounter();
		}

		// Use the size of the SourceTransformCache
		if (SourceTransformCache.Num() > 0)
		{
			// Ensure we have the correct input manifestation
			const int JointCount = SourceTransformCache.Num();
			TArray<metaMovementSDK_Transform>& TargetTransformCache = ConfigInfo.GetTransformCache(metaMovementSDK_SkeletonType::TargetSkeleton);
			check(!TargetTransformCache.IsEmpty());

			// Pattern to ensure the string cast stays in scope on Android
			std::string convertedManifestationName;
			if (ConfigInfo.GetSourceJointCountToManifestationTable().Contains(JointCount))
			{
				convertedManifestationName = TCHAR_TO_ANSI(*(ConfigInfo.GetSourceJointCountToManifestationTable()[JointCount]));
			}

			const char* sourceManifestation = convertedManifestationName.empty() ? nullptr : convertedManifestationName.c_str();

			int targetJointCount = TargetTransformCache.Num();
			metaMovementSDK_RetargetingBehaviorInfo retargetingSettings = META_MOVEMENTSDK_DEFAULT_RETARGETING_SETTINGS;
			retargetingSettings.retargetingBehavior = static_cast<metaMovementSDK_RetargetingBehavior>(RetargetingMode);
			retargetingSettings.rootMotionBehavior = static_cast<metaMovementSDK_RetargetingRootMotionBehavior>(RootMotionBehavior);
			retargetingSettings.targetOutputJointSpaceType = kMSDKJointSpaceOutputType;

			// Process the retargeting for the frame
			if (metaMovementSDK_retargetFromSourceFrameData(
					ConfigInfo.GetHandle(),
					retargetingSettings,
					SourceTransformCache.GetData(),
					SourceTransformCache.Num(),
					TargetTransformCache.GetData(),
					&targetJointCount,
					sourceManifestation,
					nullptr)
				== metaMovementSDK_Result::Success)
			{
				return true;
			}
			else
			{
				UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Unable to Retarget from Body State Data"));
			}
		}
	}
	return false;
}

bool FAnimNode_OculusXRBodyTracking::CreateRetargetingHandle(const FBoneContainer& targetSkeletonBoneContainer)
{
	ConfigInfo.Clear();
	SkeletonInfo.Initialize();
	metaMovementSDK_Handle retargetingHandle = META_MOVEMENTSDK_INVALID_HANDLE;

	// Load the soft object pointer if it hasn't been loaded yet
	UJsonDataAsset* LoadedJsonConfigAsset = JsonConfigAsset.LoadSynchronous();

	if (LoadedJsonConfigAsset != nullptr)
	{
		if (LoadedJsonConfigAsset->JsonText == "")
		{
			UE_LOG(LogOculusXRMovementUtility, Warning, TEXT("JsonConfigAsset %s is set but contains empty JsonText. Falling back to auto-generated retargeting config."), *LoadedJsonConfigAsset->GetName());
		}
		else if (metaMovementSDK_createOrUpdateHandle(TCHAR_TO_ANSI(*LoadedJsonConfigAsset->JsonText), &retargetingHandle) != metaMovementSDK_Result::Success)
		{
			// Intialize with JSON Asset
			UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to load JsonConfigAsset %s. File has errors or is invalid. Falling back to auto-generated retargeting config."), *LoadedJsonConfigAsset->GetName());
		}
	}

	if (!IS_VALID_META_MOVEMENTSDK_HANDLE(retargetingHandle))
	{
		retargetingHandle = GenerateRetargetingConfig(CachedUSkeleton);
	}

	if (IS_VALID_META_MOVEMENTSDK_HANDLE(retargetingHandle))
	{
		// TODO:
		// Validate the source and target joint counts/manifestations.
		// Cache joint index -> index map (in case target joint order is different from our update)

		// Calculate the rotation needed to maintain forward facing of this character (ie - if the config rotates the target skeleton)
		ConfigInfo.Initialize(retargetingHandle, targetSkeletonBoneContainer);
	}
	else
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to create valid retargeting handle. Body tracking will not function correctly."));
	}

	// Reset the Skeleton Changed Tracking variables
	return ConfigInfo.IsValid();
}

metaMovementSDK_Handle FAnimNode_OculusXRBodyTracking::GenerateRetargetingConfig(USkeleton* Skeleton) const
{
	// Load the soft object pointer if it hasn't been loaded yet
	UJsonDataAsset* LoadedTrackingSourceAsset = TrackingSourceConfigAsset.LoadSynchronous();

	// Initialize with Source Config and Skeleton Data
	if (LoadedTrackingSourceAsset == nullptr || LoadedTrackingSourceAsset->JsonText == "")
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("TrackingSourceConfigAsset is null or contains empty JsonText. Cannot generate retargeting config."));
		return META_MOVEMENTSDK_INVALID_HANDLE;
	}

	// First, Load our Source Config
	metaMovementSDK_Handle sourceConfigHandle = META_MOVEMENTSDK_INVALID_HANDLE;
	if (metaMovementSDK_createOrUpdateHandle(
			TCHAR_TO_ANSI(*LoadedTrackingSourceAsset->JsonText), &sourceConfigHandle)
			!= metaMovementSDK_Result::Success
		|| !IS_VALID_META_MOVEMENTSDK_HANDLE(sourceConfigHandle))
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to create or update source config handle from TrackingSourceConfigAsset JsonText."));
		return META_MOVEMENTSDK_INVALID_HANDLE;
	}

	// Next, Create our Target Config
	metaMovementSDK_Handle targetConfigHandle = CreateTargetConfigFromSkeletonJoints(Skeleton);
	if (!IS_VALID_META_MOVEMENTSDK_HANDLE(targetConfigHandle))
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to create target config handle from skeleton joints."));
		metaMovementSDK_destroy(sourceConfigHandle);
		return META_MOVEMENTSDK_INVALID_HANDLE;
	}

	// Retrieve our Target config Name
	FString targetConfigName;
	if (!MetaMovementSDK_UEUtility::GetConfigName(targetConfigHandle, targetConfigName))
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to retrieve target config name."));
		metaMovementSDK_destroy(sourceConfigHandle);
		metaMovementSDK_destroy(targetConfigHandle);
		return META_MOVEMENTSDK_INVALID_HANDLE;
	}

	// Now, align the Source and Target Config
	if (metaMovementSDK_alignTargetToSource(
			TCHAR_TO_ANSI(*targetConfigName),
			META_MOVEMENTSDK_FULL_ALIGNMENT_WITH_DEFORMATION,
			sourceConfigHandle,
			metaMovementSDK_SkeletonType::SourceSkeleton,
			targetConfigHandle,
			&targetConfigHandle)
		!= metaMovementSDK_Result::Success)
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to align source"));
		metaMovementSDK_destroy(sourceConfigHandle);
		metaMovementSDK_destroy(targetConfigHandle);
		return META_MOVEMENTSDK_INVALID_HANDLE;
	}

	// Destroy the sourceConfigHandle since we no longer need it
	metaMovementSDK_destroy(sourceConfigHandle);

	// Check and cache whether we have any valid KnownJoints, Exclude Joints set
	// If both are untouched, let's ensure that we populate the exclude joints when we create
	// the mapping.
	TAutoMappingJointData autoMappingExcludeJointData;
	ConvertExcludeAndTwistJointsToANSISets(autoMappingExcludeJointData, targetConfigHandle);

	// Now, create the structures needed to generate the mapping
	if (metaMovementSDK_generateMappings(
			targetConfigHandle,
			metaMovementSDK_AutoMappingFlags::AutoMapEmptyFlag,
			autoMappingExcludeJointData.additionalJointData.GetData(),
			autoMappingExcludeJointData.additionalJointData.Num())
		!= metaMovementSDK_Result::Success)
	{
		UE_LOG(LogOculusXRMovementUtility, Error, TEXT("Failed to generate Mappings for Target Config"));
		metaMovementSDK_destroy(targetConfigHandle);
		return META_MOVEMENTSDK_INVALID_HANDLE;
	}

	// Successfully created Mappings for the source to target skeletons
	return targetConfigHandle;
}

metaMovementSDK_Handle FAnimNode_OculusXRBodyTracking::CreateTargetConfigFromSkeletonJoints(USkeleton* Skeleton) const
{
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	metaMovementSDK_SkeletonInitParams msdkSkeletonParams = META_MOVEMENTSDK_EMPTY_SKELETON_INIT_PARAMS;

	msdkSkeletonParams.jointCount = RefSkeleton.GetRawBoneNum();
	TArray<std::string> jointNamesStrings;
	TArray<const char*> jointNames;
	TArray<const char*> parentJointNames;
	TArray<metaMovementSDK_Transform> unscaledTPose;
	jointNamesStrings.SetNum(msdkSkeletonParams.jointCount);
	jointNames.SetNum(msdkSkeletonParams.jointCount);
	parentJointNames.SetNum(msdkSkeletonParams.jointCount);
	unscaledTPose.SetNum(msdkSkeletonParams.jointCount);

	// Initialize a vector of const char pointers to the known joint table size and nullptr
	std::vector<const char*> knownJointNamesByType(metaMovementSDK_KnownJointType::KnownJointCount, nullptr);
	msdkSkeletonParams.optional_knownSourceJointNamesById = knownJointNamesByType.data();

	// Stack Array to hold the conversions from FName to FString
	// so our pointers don't go out of scope
	TArray<std::string> knownJointStringStorage;

	// Iterate our table caching the strings and setting the pointers
	// in our const char* vector at the correct index.
	knownJointStringStorage.Reserve(metaMovementSDK_KnownJointType::KnownJointCount);

	for (const auto& iter : KnownJoints)
	{
		if (!iter.Value.IsEmpty())
		{
			// Create an entry in our knownJointTempStorage
			knownJointStringStorage.Emplace(TCHAR_TO_ANSI(*iter.Value.BoneName.ToString()));

			// Cache the pointer to our string into our table
			knownJointNamesByType[static_cast<int>(iter.Key)] = knownJointStringStorage.Last().c_str();
		}
	}

	for (int i = 0; i < msdkSkeletonParams.jointCount; ++i)
	{
		const FMeshBoneInfo& BoneInfo = RefSkeleton.GetRefBoneInfo()[i];
		jointNamesStrings[i] = TCHAR_TO_ANSI(*BoneInfo.Name.ToString());
	}

	// Compute component space transforms (relative to root)
	TArray<FTransform> ComponentSpaceTransforms;
	ComponentSpaceTransforms.SetNum(msdkSkeletonParams.jointCount);

	for (int i = 0; i < msdkSkeletonParams.jointCount; ++i)
	{
		const FMeshBoneInfo& BoneInfo = RefSkeleton.GetRefBoneInfo()[i];
		const FTransform& LocalTransform = RefSkeleton.GetRefBonePose()[i];

		if (BoneInfo.ParentIndex == INDEX_NONE)
		{
			// Root bone - component space is same as local space
			ComponentSpaceTransforms[i] = LocalTransform;
		}
		else
		{
			// Accumulate parent transforms to get component space
			ComponentSpaceTransforms[i] = LocalTransform * ComponentSpaceTransforms[BoneInfo.ParentIndex];
		}
	}

	for (int i = 0; i < msdkSkeletonParams.jointCount; ++i)
	{
		const FMeshBoneInfo& BoneInfo = RefSkeleton.GetRefBoneInfo()[i];
		const FTransform& ComponentSpaceTransform = ComponentSpaceTransforms[i];

		jointNames[i] = jointNamesStrings[i].c_str();
		if (BoneInfo.ParentIndex == INDEX_NONE)
		{
			parentJointNames[i] = nullptr;
		}
		else
		{
			parentJointNames[i] = jointNamesStrings[BoneInfo.ParentIndex].c_str();
		}

		// Now in component space (relative to root)
		unscaledTPose[i] = MetaMovementSDK_UEUtility::ToMsdkTransform(
			ComponentSpaceTransform.GetRotation(),
			ComponentSpaceTransform.GetLocation(),
			ComponentSpaceTransform.GetScale3D());
	}

	// Make sure the known exclude joints are written to the config
	TAutoMappingJointData autoMappingExcludeJointData;
	ConvertExcludeAndTwistJointsToANSISets(autoMappingExcludeJointData);

	msdkSkeletonParams.jointNames = jointNames.GetData();
	msdkSkeletonParams.parentJointNames = parentJointNames.GetData();
	msdkSkeletonParams.unscaledTPose = unscaledTPose.GetData();
	msdkSkeletonParams.optional_autoMapJointData = autoMappingExcludeJointData.additionalJointData.GetData();
	msdkSkeletonParams.optional_autoMapJointDataCount = autoMappingExcludeJointData.additionalJointData.Num();

	const std::string skeletonName(TCHAR_TO_ANSI(*Skeleton->GetName()));
	metaMovementSDK_Handle referenceSkeletonHandle = META_MOVEMENTSDK_INVALID_HANDLE;
	return metaMovementSDK_createOrUpdateSimpleUtilityConfig(
			   skeletonName.c_str(),
			   metaMovementSDK_SkeletonType::TargetSkeleton,
			   &msdkSkeletonParams,
			   &referenceSkeletonHandle)
			== metaMovementSDK_Result::Success
		? referenceSkeletonHandle
		: META_MOVEMENTSDK_INVALID_HANDLE;
}

void FAnimNode_OculusXRBodyTracking::ConvertExcludeAndTwistJointsToANSISets(
	TAutoMappingJointData& autoMappingExcludeJointData,
	const metaMovementSDK_Handle targetConfigHandle) const
{
	// Now that we have both our Source and Target handles, Create our Mapping
	std::unordered_set<std::string> ansiExcludeJoints = BoneNameSetToANSIStringSet(AutoMappingExcludeJoints);
	std::unordered_set<std::string> ansiTwistExcludeJoints = BoneNameSetToANSIStringSet(AutoMappingExcludeTwistBehaviorOnly);

	if (IS_VALID_META_MOVEMENTSDK_HANDLE(targetConfigHandle))
	{
		bool hasValidKnownOrExcludedJoint = false;
		for (const auto& iter : KnownJoints)
		{
			if (!iter.Value.IsEmpty())
			{
				hasValidKnownOrExcludedJoint = true;
				break;
			}
		}

		hasValidKnownOrExcludedJoint |= !ansiExcludeJoints.empty();

		// If we haven't set anything on this character, have the library
		// identify joints we should ignore and populate the structure we
		// pass to the system.
		if (!hasValidKnownOrExcludedJoint)
		{
			ansiExcludeJoints = BoneNameSetToANSIStringSet(IdentifyPossibleExcludeJointsFromConfig(targetConfigHandle));
		}
	}

	autoMappingExcludeJointData.jointNames.clear();
	autoMappingExcludeJointData.jointNames.reserve(ansiExcludeJoints.size() + ansiTwistExcludeJoints.size());
	for (const std::string& jointName : ansiExcludeJoints)
	{
		autoMappingExcludeJointData.jointNames.insert(jointName);
	}
	for (const std::string& jointName : ansiTwistExcludeJoints)
	{
		autoMappingExcludeJointData.jointNames.insert(jointName);
	}

	autoMappingExcludeJointData.additionalJointData.Empty();
	autoMappingExcludeJointData.additionalJointData.Reserve(autoMappingExcludeJointData.jointNames.size());

	for (const auto& iter : autoMappingExcludeJointData.jointNames)
	{
		metaMovementSDK_AutoMappingJointData jointData = {
			iter.c_str(),
			metaMovementSDK_AutoMappingJointFlags::AutoMapEmptyJointFlag,
		};
		if (ansiExcludeJoints.find(iter) != ansiExcludeJoints.end())
		{
			jointData.flags =
				static_cast<metaMovementSDK_AutoMappingJointFlags>(jointData.flags | metaMovementSDK_AutoMappingJointFlags::AutoMapJointFlagExclude);
		}
		if (ansiTwistExcludeJoints.find(iter) != ansiTwistExcludeJoints.end())
		{
			jointData.flags =
				static_cast<metaMovementSDK_AutoMappingJointFlags>(jointData.flags | metaMovementSDK_AutoMappingJointFlags::AutoMapJointFlagExcludeFromTwistMappings);
		}
		autoMappingExcludeJointData.additionalJointData.Emplace(jointData);
	}
}

TSet<FOculusXRBoneName> FAnimNode_OculusXRBodyTracking::IdentifyPossibleExcludeJointsFromConfig(metaMovementSDK_Handle configHandle)
{
	TSet<FOculusXRBoneName> retVal;
	if (IS_VALID_META_MOVEMENTSDK_HANDLE(configHandle))
	{
		// TODO: Move the string retrieval to MetaMovementSDK_UEUtility.h
		int bufferSize = 0;
		int jointCount = 0;
		if (metaMovementSDK_identifyPossibleExcludeFromMappingUsingRestPose(
				configHandle,
				metaMovementSDK_SkeletonType::TargetSkeleton,
				nullptr,
				&bufferSize,
				nullptr,
				&jointCount)
			!= metaMovementSDK_Result::Success)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to identify possible joints to exclude from Mapping."));
			return retVal;
		}

		std::vector<char> buffer(bufferSize);
		std::vector<const char*> jointNames(jointCount);
		if (metaMovementSDK_identifyPossibleExcludeFromMappingUsingRestPose(
				configHandle,
				metaMovementSDK_SkeletonType::TargetSkeleton,
				buffer.data(),
				&bufferSize,
				jointNames.data(),
				&jointCount)
			!= metaMovementSDK_Result::Success)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to retrieve joint names for possible joints to exclude from Mapping."));
			return retVal;
		}

		for (const char* excludedJointName : jointNames)
		{
			retVal.Emplace(excludedJointName);
		}
	}
	return retVal;
}

std::unordered_set<std::string> FAnimNode_OculusXRBodyTracking::BoneNameSetToANSIStringSet(const TSet<FOculusXRBoneName>& BoneNameSet)
{
	std::unordered_set<std::string> retVal;
	retVal.reserve(BoneNameSet.Num());

	for (const auto& iter : BoneNameSet)
	{
		if (!iter.IsEmpty())
		{
			retVal.insert(TCHAR_TO_ANSI(*iter.BoneName.ToString()));
		}
	}

	return retVal;
}

bool FAnimNode_OculusXRBodyTracking::LoadedConfigInfo::ReInitializeBoneContainerIfNecessary(const FBoneContainer& targetSkeletonBoneContainer)
{
	if (TargetBoneContainerSerialNumber != targetSkeletonBoneContainer.GetSerialNumber())
	{
		// Re-Initialize the Target Index Array (for the LOD of the model)
		InitializeTargetIndexArray(targetSkeletonBoneContainer);
		return true;
	}
	return false;
}

void FAnimNode_OculusXRBodyTracking::LoadedConfigInfo::Initialize(
	const metaMovementSDK_Handle msdkHandle,
	const FBoneContainer& targetSkeletonBoneContainer)
{
	Clear();
	RetargetingHandle = msdkHandle;
	if (IS_VALID_META_MOVEMENTSDK_HANDLE(RetargetingHandle))
	{
		// Set Cache Sizes based on the joint count from the Config:
		metaMovementSDK_SkeletonInfo sourceSkeletonInfo{ metaMovementSDK_SkeletonType::SourceSkeleton, metaMovementSDK_SkeletonFlags::SkeletonFlag_None, 0, 0 };
		MetaMovementSDK_UEUtility::GetSkeletonInfo(RetargetingHandle, sourceSkeletonInfo.skeletonType, sourceSkeletonInfo);

		metaMovementSDK_SkeletonInfo targetSkeletonInfo{ metaMovementSDK_SkeletonType::TargetSkeleton, metaMovementSDK_SkeletonFlags::SkeletonFlag_None, 0, 0 };
		MetaMovementSDK_UEUtility::GetSkeletonInfo(RetargetingHandle, targetSkeletonInfo.skeletonType, targetSkeletonInfo);

		SourceTransformCache.SetNum(sourceSkeletonInfo.jointCount);
		TargetTransformCache.SetNum(targetSkeletonInfo.jointCount);

		// Initialize the Target Index Array (for the LOD of the model)
		InitializeTargetIndexArray(targetSkeletonBoneContainer);

		// Initialize our ConfigToComponentSpace Transform
		InitializeConfigToComponentSpace(targetSkeletonBoneContainer.GetReferenceSkeleton());

		// Populate the source Manifestation data
		TArray<FString> sourceManifestationNames;
		if (MetaMovementSDK_UEUtility::GetManifestationNames(RetargetingHandle, metaMovementSDK_SkeletonType::SourceSkeleton, sourceManifestationNames) && !sourceManifestationNames.IsEmpty())
		{
			SourceJointCountToManifestationTable.Reserve(sourceManifestationNames.Num() + 1);
			for (const FString& manifestation : sourceManifestationNames)
			{
				int manifestationJointCount = 0;
				if (metaMovementSDK_getJointsInManifestation(
						RetargetingHandle,
						metaMovementSDK_SkeletonType::SourceSkeleton,
						TCHAR_TO_ANSI(*manifestation),
						nullptr,
						&manifestationJointCount)
					== metaMovementSDK_Result::Success)
				{
					SourceJointCountToManifestationTable.Emplace(manifestationJointCount, manifestation);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to retrieve joint count for Manifestation named %s"), *manifestation);
				}
			}
		}
	}
}

void FAnimNode_OculusXRBodyTracking::LoadedConfigInfo::InitializeConfigToComponentSpace(const FReferenceSkeleton& RefSkeleton)
{
	if (IsValid() && !TargetIndexArray.IsEmpty())
	{
		// Get the Known Joint Indexes and cache them to our T-Map
		metaMovementSDK_KnownJointIndexData knownJointIndices;
		if (metaMovementSDK_getKnownJointIndexes(RetargetingHandle, metaMovementSDK_SkeletonType::TargetSkeleton, &knownJointIndices) == metaMovementSDK_Result::Success)
		{
			// Populate our table
			for (int i = 0; i < metaMovementSDK_KnownJointType::KnownJointCount; ++i)
			{
				if (IS_VALID_META_MOVEMENTSDK_JOINT(knownJointIndices.jointIndexByType[i]))
				{
					ConfigTargetKnownJointLookupTable.Add(static_cast<EOculusXRBodyKnownJointType>(i), knownJointIndices.jointIndexByType[i]);
				}
			}
		}

		// Get the Unscaled TPose in RootRelativeSpace
		if (MetaMovementSDK_UEUtility::GetSkeletonTPose(
				RetargetingHandle,
				metaMovementSDK_SkeletonType::TargetSkeleton,
				metaMovementSDK_SkeletonTPoseType::UnscaledTPose,
				TargetTransformCache))
		{
			// Use either the root or the first index to calculate the Transform
			const int rootIndex =
				ConfigTargetKnownJointLookupTable.Contains(EOculusXRBodyKnownJointType::Root) ? ConfigTargetKnownJointLookupTable[EOculusXRBodyKnownJointType::Root] : 0;

			if (TargetIndexArray[rootIndex].IsValid())
			{
				const FTransform TPoseRoot = MetaMovementSDK_UEUtility::ToFTransform(TargetTransformCache[rootIndex]);
				const FTransform RefSkeletonRoot = RefSkeleton.GetRefBonePose()[TargetIndexArray[rootIndex].GetInt()];

				// Calculate the transformation that converts TPoseRoot to RefSkeletonRoot
				ConfigToComponentSpace = TPoseRoot.Inverse() * RefSkeletonRoot;
			}
		}
	}
}

void FAnimNode_OculusXRBodyTracking::LoadedConfigInfo::InitializeTargetIndexArray(const FBoneContainer& targetSkeletonBoneContainer)
{
	TargetIndexArray.Empty();
	MSDKJointsMissingFromCurrentSkeletonLOD.Empty();

	// Get the target joint names
	TArray<FString> targetJointNames;
	if (MetaMovementSDK_UEUtility::GetJointNames(RetargetingHandle, metaMovementSDK_SkeletonType::TargetSkeleton, targetJointNames) && !targetJointNames.IsEmpty())
	{

		// Get full skeleton and the bone indices array
		const FReferenceSkeleton& TargetReferenceSkeleton = targetSkeletonBoneContainer.GetReferenceSkeleton();

		// We need to build a joint name to index map so we can reference
		// the index/order of the config to the mesh joint using the names.
		// This ensures that the update works even if the config is in a different
		// index order than the mesh/Unreal interfaces.
		TMap<FString, FCompactPoseBoneIndex> jointNameToFCPBIndex;
		jointNameToFCPBIndex.Reserve(TargetReferenceSkeleton.GetNum());

		for (int i = 0; i < TargetReferenceSkeleton.GetRawBoneNum(); ++i)
		{
			// Get the bone name from the reference skeleton
			// BoneIndex = Index into the full skeleton
			const FMeshPoseBoneIndex meshBoneIndex(i);
			const FCompactPoseBoneIndex CompactPoseIndex = targetSkeletonBoneContainer.MakeCompactPoseIndex(meshBoneIndex);
			const FMeshBoneInfo& BoneInfo = TargetReferenceSkeleton.GetRefBoneInfo()[i];
			FString BoneNameFString = BoneInfo.Name.ToString();
			if (jointNameToFCPBIndex.Contains(BoneNameFString))
			{
				UE_LOG(LogTemp, Error, TEXT("Skeleton contains multiple joints named %s!"), *BoneNameFString);
			}
			jointNameToFCPBIndex.Add(BoneNameFString, CompactPoseIndex);
		}

		TargetIndexArray.Reserve(targetJointNames.Num());
		for (int i = 0; i < targetJointNames.Num(); ++i)
		{
			if (jointNameToFCPBIndex.Contains(targetJointNames[i]))
			{
				TargetIndexArray.Add(jointNameToFCPBIndex[targetJointNames[i]]);
			}
			else
			{
				TargetIndexArray.Add(kInvalidBoneIndex);
				MSDKJointsMissingFromCurrentSkeletonLOD.Add(static_cast<metaMovementSDK_JointIndex>(i));
			}
		}
	}
}

void FAnimNode_OculusXRBodyTracking::LoadedConfigInfo::Clear()
{
	if (IS_VALID_META_MOVEMENTSDK_HANDLE(RetargetingHandle))
	{
		metaMovementSDK_destroy(RetargetingHandle);
	}
	RetargetingHandle = META_MOVEMENTSDK_INVALID_HANDLE;

	ConfigToComponentSpace = FTransform::Identity;
	ConfigTargetKnownJointLookupTable.Empty();

	TargetIndexArray.Empty();
	SourceJointCountToManifestationTable.Empty();

	SourceTransformCache.Empty();
	TargetTransformCache.Empty();
}

void FAnimNode_OculusXRBodyTracking::CachedSkeletonInfo::Initialize()
{
	ResetSourceChangeCounter();

	// Reset the Serial Number and joint Indice Table
	SourceJointCount = 0;
}

void FAnimNode_OculusXRBodyTracking::MsdkTransformArrayFromXRBodySkeleton(const FOculusXRBodySkeleton& xrBodySkeleton, TArray<metaMovementSDK_Transform>& outTransformArray)
{
	check(xrBodySkeleton.Bones.Num() >= xrBodySkeleton.NumBones);
	outTransformArray.Empty(xrBodySkeleton.NumBones);
	for (int i = 0; i < xrBodySkeleton.NumBones; ++i)
	{
		outTransformArray.Add({ MetaMovementSDK_UEUtility::ToMsdkQuatf(FQuat(xrBodySkeleton.Bones[i].Orientation)),
			MetaMovementSDK_UEUtility::ToMsdkVector3f(xrBodySkeleton.Bones[i].Position),
			META_MOVEMENTSDK_ONE_VECTOR });
	}
}

void FAnimNode_OculusXRBodyTracking::MsdkTransformArrayFromXRBodyState(const FOculusXRBodyState& xrBodyState, TArray<metaMovementSDK_Transform>& outTransformArray)
{
	outTransformArray.Empty(xrBodyState.Joints.Num());
	for (int i = 0;
		i < xrBodyState.Joints.Num() && xrBodyState.Joints[i].bIsValid && (xrBodyState.Joints[i].LocationFlags & (XRSpaceFlags::XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XRSpaceFlags::XR_SPACE_LOCATION_POSITION_VALID_BIT));
		++i)
	{
		outTransformArray.Add({ MetaMovementSDK_UEUtility::ToMsdkQuatf(FQuat(xrBodyState.Joints[i].Orientation)),
			MetaMovementSDK_UEUtility::ToMsdkVector3f(xrBodyState.Joints[i].Position),
			META_MOVEMENTSDK_ONE_VECTOR });
	}
}

bool FAnimNode_OculusXRBodyTracking::ShouldUseFallbackPose() const
{
#if WITH_EDITOR
	if (SkeletalMeshComponent != nullptr)
	{
		UWorld* world = SkeletalMeshComponent->GetWorld();
		if (world != nullptr)
		{
			switch (world->WorldType)
			{
				case EWorldType::PIE:
				case EWorldType::Game:
					return true;
			}
		}
	}
	return false;
#else
	return SkeletalMeshComponent != nullptr;
#endif // WITH_EDITOR
}
