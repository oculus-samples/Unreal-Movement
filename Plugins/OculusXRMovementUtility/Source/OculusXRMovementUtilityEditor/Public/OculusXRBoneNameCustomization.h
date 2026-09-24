/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class USkeleton;

/**
 * Property type customization for FOculusXRBoneName that provides a dropdown
 * of available bone names from the skeleton.
 */
class FOculusXRBoneNameCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	/**
	 * Get the skeleton from the property handle context.
	 */
	USkeleton* GetSkeletonFromPropertyHandle(TSharedRef<IPropertyHandle> PropertyHandle) const;

	/**
	 * Get all bone names from the skeleton as an array of shared strings.
	 */
	TArray<TSharedPtr<FString>> GetBoneNames(USkeleton* Skeleton) const;

	/**
	 * Create the bone name dropdown widget.
	 */
	TSharedRef<SWidget> MakeBoneNameWidget(TSharedRef<IPropertyHandle> BoneNamePropertyHandle);

	/**
	 * Called when a bone name is selected from the dropdown.
	 */
	void OnBoneNameSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> BoneNamePropertyHandle);

	/**
	 * Get the current bone name as text for display in the dropdown.
	 */
	FText GetCurrentBoneNameText(TSharedRef<IPropertyHandle> BoneNamePropertyHandle) const;

	/**
	 * Cached skeleton pointer.
	 */
	TWeakObjectPtr<USkeleton> CachedSkeleton;

	/**
	 * Cached bone names for dropdown.
	 */
	TArray<TSharedPtr<FString>> CachedBoneNames;
};
