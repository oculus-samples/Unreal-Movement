/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRRetargetingUtils.h"

bool FOculusXRRetargetingUtils::GetUnitScaleFactorFromSettings(UWorld* World, float& OutWorldToMeters)
{
	if (IsValid(World))
	{
		if (const auto* WorldSettings = World->GetWorldSettings(); IsValid(WorldSettings))
		{
			OutWorldToMeters = WorldSettings->WorldToMeters;
			return true;
		}
	}
	return false;
}
