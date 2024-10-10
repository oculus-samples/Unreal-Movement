/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "OculusXRRetargetingEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

IMPLEMENT_GAME_MODULE(FOculusXRRetargetingEditor, OculusRetargetingEditor);

DEFINE_LOG_CATEGORY(OculusXRRetargetingEditor)

#define LOCTEXT_NAMESPACE "OculusXRRetargetingEditor"

void FOculusXRRetargetingEditor::StartupModule()
{
	UE_LOG(OculusXRRetargetingEditor, Warning, TEXT("OculusXR Retargeting Editor: Log Started"));
}

void FOculusXRRetargetingEditor::ShutdownModule()
{
	UE_LOG(OculusXRRetargetingEditor, Warning, TEXT("OculusXR Retargeting: Log Ended"));
}

#undef LOCTEXT_NAMESPACE
