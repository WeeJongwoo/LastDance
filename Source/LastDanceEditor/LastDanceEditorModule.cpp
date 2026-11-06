#include "LastDanceEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Data/LDComboDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogLastDanceEditor, Log, All);

void FLastDanceEditorModule::StartupModule()
{
	// LastDance 모듈의 타입을 참조할 수 있는지 확인
	UE_LOG(LogLastDanceEditor, Log, TEXT("LastDanceEditor loaded: %s"), *ULDComboDataAsset::StaticClass()->GetName());
}

void FLastDanceEditorModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FLastDanceEditorModule, LastDanceEditor)
