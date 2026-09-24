/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRBoneNameCustomization.h"
#include "OculusXRBoneName.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Animation/Skeleton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animation/AnimBlueprint.h"

TSharedRef<IPropertyTypeCustomization> FOculusXRBoneNameCustomization::MakeInstance()
{
	return MakeShareable(new FOculusXRBoneNameCustomization());
}

void FOculusXRBoneNameCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Get the skeleton from the property context
	USkeleton* Skeleton = GetSkeletonFromPropertyHandle(PropertyHandle);

	if (Skeleton)
	{
		CachedSkeleton = Skeleton;
		CachedBoneNames = GetBoneNames(Skeleton);
	}

	// Get the BoneName child property
	TSharedPtr<IPropertyHandle> BoneNamePropertyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOculusXRBoneName, BoneName));

	if (BoneNamePropertyHandle.IsValid() && Skeleton)
	{
		// Create custom widget with dropdown
		HeaderRow
			.NameContent()
				[PropertyHandle->CreatePropertyNameWidget()]
			.ValueContent()
				[MakeBoneNameWidget(BoneNamePropertyHandle.ToSharedRef())];
	}
	else
	{
		// Fall back to default widget if no skeleton found
		HeaderRow
			.NameContent()
				[PropertyHandle->CreatePropertyNameWidget()]
			.ValueContent()
				[BoneNamePropertyHandle.IsValid() ? BoneNamePropertyHandle->CreatePropertyValueWidget() : PropertyHandle->CreatePropertyValueWidget()];
	}
}

void FOculusXRBoneNameCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// No children to display - we're showing everything in the header
}

USkeleton* FOculusXRBoneNameCustomization::GetSkeletonFromPropertyHandle(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (UObject* Outer : OuterObjects)
	{
		// Try direct cast to AnimBlueprintGeneratedClass
		if (UAnimBlueprintGeneratedClass* AnimBPClass = Cast<UAnimBlueprintGeneratedClass>(Outer))
		{
			if (USkeleton* Skeleton = AnimBPClass->GetTargetSkeleton())
			{
				return Skeleton;
			}
		}

		// Walk up the outer chain to find AnimBlueprintGeneratedClass or AnimBlueprint
		UObject* CurrentOuter = Outer;
		while (CurrentOuter)
		{
			// Check for AnimBlueprintGeneratedClass
			if (UAnimBlueprintGeneratedClass* AnimBPClass = Cast<UAnimBlueprintGeneratedClass>(CurrentOuter))
			{
				if (USkeleton* Skeleton = AnimBPClass->GetTargetSkeleton())
				{
					return Skeleton;
				}
			}

			// Check for AnimBlueprint and get its generated class
			if (UAnimBlueprint* AnimBP = Cast<UAnimBlueprint>(CurrentOuter))
			{
				if (UAnimBlueprintGeneratedClass* AnimBPClass = AnimBP->GetAnimBlueprintGeneratedClass())
				{
					if (USkeleton* Skeleton = AnimBPClass->GetTargetSkeleton())
					{
						return Skeleton;
					}
				}
			}

			CurrentOuter = CurrentOuter->GetOuter();
		}
	}

	return nullptr;
}

TArray<TSharedPtr<FString>> FOculusXRBoneNameCustomization::GetBoneNames(USkeleton* Skeleton) const
{
	TArray<TSharedPtr<FString>> BoneNames;

	if (!Skeleton)
	{
		return BoneNames;
	}

	// Add "None" option
	BoneNames.Add(MakeShareable(new FString("None")));

	// Get all bone names from the reference skeleton
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	TArray<TSharedPtr<FString>> TempBoneNames;
	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetRawBoneNum(); ++BoneIndex)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
		TempBoneNames.Add(MakeShareable(new FString(BoneName.ToString())));
	}

	// Sort bone names alphabetically
	TempBoneNames.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B) {
		return A->Compare(*B) < 0;
	});

	// Add sorted bone names after "None"
	BoneNames.Append(TempBoneNames);

	return BoneNames;
}

TSharedRef<SWidget> FOculusXRBoneNameCustomization::MakeBoneNameWidget(TSharedRef<IPropertyHandle> BoneNamePropertyHandle)
{
	return SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&CachedBoneNames)
		.OnSelectionChanged_Raw(this, &FOculusXRBoneNameCustomization::OnBoneNameSelected, BoneNamePropertyHandle)
		.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) {
			return SNew(STextBlock).Text(FText::FromString(*InItem));
		})
		.Content()
			[SNew(STextBlock)
					.Text_Raw(this, &FOculusXRBoneNameCustomization::GetCurrentBoneNameText, BoneNamePropertyHandle)];
}

void FOculusXRBoneNameCustomization::OnBoneNameSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> BoneNamePropertyHandle)
{
	if (NewSelection.IsValid())
	{
		FName NewBoneName = (*NewSelection == "None") ? NAME_None : FName(**NewSelection);
		BoneNamePropertyHandle->SetValue(NewBoneName);
	}
}

FText FOculusXRBoneNameCustomization::GetCurrentBoneNameText(TSharedRef<IPropertyHandle> BoneNamePropertyHandle) const
{
	FName CurrentBoneName;
	if (BoneNamePropertyHandle->GetValue(CurrentBoneName) == FPropertyAccess::Success)
	{
		if (CurrentBoneName == NAME_None)
		{
			return FText::FromString("None");
		}
		return FText::FromName(CurrentBoneName);
	}

	return FText::FromString("None");
}
