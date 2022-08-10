// MIT License - Copyright (c) 2022 Jared Cook

#pragma once
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

#include "MultiSplineMesh.generated.h"

UCLASS()
class AMultiSplineMesh : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SplineMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis = ESplineMeshAxis::Type::X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPhysicalMaterial* PhysicalMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollisionProfile = UCollisionProfile::BlockAll_ProfileName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSmoothInterpRollScale = false;
	UPROPERTY(EditAnywhere, Category = "Spawn Item Along Spline")
	TSubclassOf<AActor> SpawnedItem = nullptr;
	UPROPERTY(EditAnywhere, Category = "Spawn Item Along Spline")
	int NumSpawnedItems = 0;
	UPROPERTY(EditAnywhere, Category = "Spawn Item Along Spline")
	int SpawnedItemStartNode = 0;
	UPROPERTY(EditAnywhere, Category = "Spawn Item Along Spline")
	int SpawnedItemEndNode = 0;
	UPROPERTY(EditAnywhere, Category = "Spawn Item Along Spline")
	FVector SpawnedItemOffset = FVector::UpVector * 75;

private:
	UPROPERTY(EditDefaultsOnly)
	USplineComponent* SplineComponent;
	UPROPERTY()
	TArray<USplineMeshComponent*> GeneratedMeshes;

public:
	AMultiSplineMesh();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// recreates all of the current mesh points, so that they align with the start/end of each mesh component.
	// sometimes useful for tweaking a problem area
	UFUNCTION(CallInEditor)
	void CreatePointsPerMesh();
	// destroy all mesh components and remake them - useful if you updated the mesh asset
	// and you want to clear all the cached spline meshes
	UFUNCTION(CallInEditor)
	void ReconstructMeshes();

private:
	void ConstructMeshes();

	// spawns the given blueprint along the spline, for easy placing of things like pickups
	void SpawnItems();
};
