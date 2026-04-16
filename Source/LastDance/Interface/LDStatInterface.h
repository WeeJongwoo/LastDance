// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Component/LDStatComponent.h"
#include "LDStatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULDStatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LASTDANCE_API ILDStatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual ULDStatComponent* GetStatComponent() const = 0;
};
