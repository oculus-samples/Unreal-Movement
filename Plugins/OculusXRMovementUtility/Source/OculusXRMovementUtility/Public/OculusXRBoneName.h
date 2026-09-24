/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "OculusXRBoneName.generated.h"

/**
 * Wrapper struct for bone names that enables dropdown selection in the editor.
 * This struct wraps an FName and provides implicit conversion operators
 * for seamless usage with existing code.
 */
USTRUCT(BlueprintType)
struct OCULUSXRMOVEMENTUTILITY_API FOculusXRBoneName
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName = NAME_None;

	// Default constructor
	FOculusXRBoneName() : BoneName(NAME_None) {}

	// Constructor from FName
	FOculusXRBoneName(FName InName) : BoneName(InName) {}

	// Constructor from string
	FOculusXRBoneName(const TCHAR* InName) : BoneName(InName) {}

	// Constructor from Ansi string
	FOculusXRBoneName(const char* InName) : BoneName(ANSI_TO_TCHAR(InName)) {}

	// Static Const for comparison
	static const FOculusXRBoneName& Empty()
	{
		static const FOculusXRBoneName emptyBoneName;
		return emptyBoneName;
	}

	bool IsEmpty() const
	{
		return *this == Empty();
	}

	// Implicit conversion to FName (allows using wrapper where FName is expected)
	operator FName() const { return BoneName; }

	// Assignment from FName
	FOculusXRBoneName& operator=(FName InName)
	{
		BoneName = InName;
		return *this;
	}

	// Comparison operators
	bool operator==(const FOculusXRBoneName& Other) const { return BoneName == Other.BoneName; }
	bool operator!=(const FOculusXRBoneName& Other) const { return BoneName != Other.BoneName; }
	bool operator==(FName Other) const { return BoneName == Other; }
	bool operator!=(FName Other) const { return BoneName != Other; }

	// For use in maps
	friend uint32 GetTypeHash(const FOculusXRBoneName& Wrapper)
	{
		return GetTypeHash(Wrapper.BoneName);
	}
};
