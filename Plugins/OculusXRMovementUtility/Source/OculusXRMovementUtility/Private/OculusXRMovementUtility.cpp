/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRMovementUtility.h"

#define LOCTEXT_NAMESPACE "FOculusXRMovementUtilityModule"

DEFINE_LOG_CATEGORY(LogOculusXRMovementUtility);

void FOculusXRMovementUtilityModule::StartupModule()
{
}

void FOculusXRMovementUtilityModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOculusXRMovementUtilityModule, OculusXRMovementUtility)
