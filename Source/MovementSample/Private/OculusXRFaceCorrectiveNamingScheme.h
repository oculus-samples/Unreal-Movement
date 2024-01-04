/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.
This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "OculusXRFaceCorrectives.h"

/**
 * The version of the naming scheme to use for parsing corrective shapes.
 */
UENUM(BlueprintType)
enum ENamingSchemeVersion : int
{
	V0 = 0,
	COUNT = 1 UMETA(Hidden)
};

/**
 * A naming scheme describes how to parse the various corrective shapes' ICorrectiveShape
 * representations from a list of shape names.
 */
class NamingScheme
{
public:
	virtual ~NamingScheme() = default;

	/**
	 * Parses all proved shape names looking for known corrective shapes. Shapes that are not
	 * parseable are assumed to be driver shapes, but a warning is surfaced if a driver is
	 * found after one or more correctives has already been found.
	 */
	TArray<TSharedPtr<ICorrectiveShape>> ParseCorrectives(TArray<FName> MorphTargetNames);

protected:
	/**
	 * Attempts to parse a usable Combination corrective out of the shape name at the given index.
	 */
	virtual bool TryParseCombination(TArray<FName>& MorphTargetNames, int Index, TSharedPtr<Combination> Combination) = 0;

	/**
	 * Attempts to parse a partial InBetween corrective out of the shape name at the given index.
	 */
	virtual bool TryParseInBetween(TArray<FName>& MorphTargetNames, int Index, Partial* InBetween) = 0;

	/**
	 * The human-readable name of this parsing version.
	 */
	virtual FString VersionName() = 0;
};

/**
 * A collection of naming schemes to be used for parsing corrective shapes.
 */
class NamingSchemes
{
public:
	/**
	* The V0 corrective naming scheme.
	*/
	static NamingScheme* V0()
	{
		return new NamingSchemeV0();
	}

private:
	/**
	 * V0 implementation of the naming scheme.
	 */
	class NamingSchemeV0 final : public NamingScheme
	{
	public:
		NamingSchemeV0() = default;
		virtual ~NamingSchemeV0() override = default;

	protected:
		virtual bool TryParseCombination(TArray<FName>& MorphTargetNames, int Index, TSharedPtr<Combination> Combination) override;
		virtual bool TryParseInBetween(TArray<FName>& MorphTargetNames, int Index, Partial* InBetween) override;
		virtual FString VersionName() override;

	private:
		/**
		 * Searches a list of shape names for a given shape, trying different combinations of
		 * potential suffixes if the shape is not immediately found.
		 */
		static bool FindDriver(TArray<FName>& MorphTargetNames, FName& DriverName, FString Suffix, FName& Result);
	};
};
