/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXR_TrackingGraphNodes.h"
#include "OculusXRMovementTypes.h"
#include "Misc/EnumRange.h"
#include <cctype>

ENUM_RANGE_BY_COUNT(EOculusXRBoneID, EOculusXRBoneID::COUNT);

void UOculusXR_BodyTracking::GenerateBoneMapping()
{
	FString SourceName;
	TArray<FName> BoneNames;
	const FReferenceSkeleton& RefSkeleton = GetAnimBlueprint()->TargetSkeleton->GetReferenceSkeleton();

	int32 NumBones = RefSkeleton.GetNum();
	for (int i = 0; i < NumBones; ++i)
	{
		BoneNames.Add(RefSkeleton.GetBoneName(i));
	}

	TMap<EOculusXRBoneID, FName> BoneMap;
	for (EOculusXRBoneID BoneID : TEnumRange<EOculusXRBoneID>())
	{
		int MaxMatches = 0;
		int NumSubstrings = 0;
		TSet<FString> Substrings;
		SourceName = StaticEnum<EOculusXRBoneID>()->GetNameStringByValue(static_cast<int64>(BoneID));

		for (FName TargetName : BoneNames)
		{
			Substrings = GetSubstringsBetweenJointNames(SourceName, TargetName.ToString(), MIN_SUBSTRING_LENGTH, FilterJoints);
			NumSubstrings = Substrings.Num();

			if (NumSubstrings > MaxMatches && CountSubstringsCharLength(Substrings) > (TargetName.ToString().Len() * FilterSensitivity))
			{
				MaxMatches = NumSubstrings;
				BoneMap.Add(BoneID, TargetName);
			}
		}
	}
	Node.BoneRemapping = BoneMap;
	MarkPackageDirty();
}

int UOculusXR_BodyTracking::CountSubstringsCharLength(const TSet<FString>& Substrings)
{
	int Result = 0;
	for (const FString& s : Substrings)
	{
		Result += s.Len();
	}
	return Result;
}

TSet<FString> UOculusXR_BodyTracking::GetSubstringsBetweenJointNames(FString sourceJointName, FString targetJointName, int minSubstringLength, TSet<FString> filterList)
{
	TSet<FString> commonSubstrings;

	int sourceIdx = sourceJointName.Len() - minSubstringLength;
	int targetIdx = 0;

	while (targetIdx < targetJointName.Len() - minSubstringLength)
	{
		int charsToCompare = std::min(sourceJointName.Len() - sourceIdx, targetJointName.Len() - targetIdx);
		int substringStart = -1;

		for (int i = 0; i < charsToCompare; i++)
		{
			if (std::tolower(sourceJointName[sourceIdx + i]) == std::tolower(targetJointName[targetIdx + i]))
			{
				// Check for an Uppercase termination of a word
				if (substringStart >= 0 && i > substringStart && ((std::isupper(sourceJointName[sourceIdx + i]) && !std::isupper(sourceJointName[sourceIdx + i - 1])) || (std::isupper(targetJointName[targetIdx + i]) && !std::isupper(targetJointName[targetIdx + i - 1]))))
				{
					if (i - substringStart >= minSubstringLength)
					{
						// Use the target space for capitalization
						FString foundSubstring = targetJointName.Mid(targetIdx + substringStart,
							i - substringStart);

						if (filterList.Contains(foundSubstring))
						{
							commonSubstrings.Add(foundSubstring);
						}
					}
					// Start a new subString
					substringStart = i;
				}
				else if (substringStart < 0)
				{
					substringStart = i;
				}
			}
			else if (substringStart >= 0)
			{
				if (i - substringStart >= minSubstringLength)
				{
					// Use the target space for capitalization
					FString foundSubstring = targetJointName.Mid(targetIdx + substringStart,
						i - substringStart);

					if (filterList.Contains(foundSubstring))
					{
						commonSubstrings.Add(foundSubstring);
					}
				}
				substringStart = -1;
			}
		}
		// We ended with a substring - store it
		if (substringStart >= 0 && charsToCompare - substringStart >= minSubstringLength)
		{
			FString foundSubstring = targetJointName.Mid(targetIdx + substringStart,
				charsToCompare - substringStart);

			if (filterList.Contains(foundSubstring))
			{
				commonSubstrings.Add(foundSubstring);
			}
		}
		if (sourceIdx > 0)
		{
			sourceIdx--;
		}
		else
		{
			targetIdx++;
		}
	}
	return commonSubstrings;
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
