#pragma once

/*
browse to add header in project folder
delete saved intermediate binaries
regenerate visual studio files
open uproject in editor
open sln in Visual Studio
set startup project in Visual Studio
#include "ClassName.generated.h" must be last in header include list
*/



UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "AssaultRifle"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};