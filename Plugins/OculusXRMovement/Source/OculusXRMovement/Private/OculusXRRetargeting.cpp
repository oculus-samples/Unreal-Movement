/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRRetargeting.h"

#define LOCTEXT_NAMESPACE "FOculusXRRetargetingModule"

DEFINE_LOG_CATEGORY(LogOculusXRRetargeting);

void FOculusXRRetargetingModule::StartupModule()
{
}

void FOculusXRRetargetingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOculusXRRetargetingModule, OculusXRRetargeting)
