#include "MultiSplineMesh.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

AMultiSplineMesh::AMultiSplineMesh()
{
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComponent->SetupAttachment(RootComponent);
	SplineComponent->bSplineHasBeenEdited = true;
	SplineComponent->bInputSplinePointsToConstructionScript = true;
	SplineComponent->bIsEditorOnly = false;
	GeneratedMeshes.Empty();
}

void AMultiSplineMesh::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IsValid(SplineMesh))
	{
		ConstructMeshes();
		SpawnItems();
	}
}

void AMultiSplineMesh::BeginPlay()
{
	Super::BeginPlay();
}

void AMultiSplineMesh::ConstructMeshes()
{
	float SplineLength = SplineComponent->GetSplineLength();
	float LengthPerMesh;
	switch (ForwardAxis)
	{
		default:
		case ESplineMeshAxis::Type::X:
			LengthPerMesh = SplineMesh->GetBoundingBox().GetSize().X;
			break;
		case ESplineMeshAxis::Type::Y:
			LengthPerMesh = SplineMesh->GetBoundingBox().GetSize().Y;
			break;
		case ESplineMeshAxis::Type::Z:
			LengthPerMesh = SplineMesh->GetBoundingBox().GetSize().Z;
			break;
	}

	GeneratedMeshes.Empty();
	TArray<USceneComponent*> ChildrenComponents;
	SplineComponent->GetChildrenComponents(false, ChildrenComponents);
	for (USceneComponent* Child : ChildrenComponents)
	{
		USplineMeshComponent* MeshChild = Cast<USplineMeshComponent>(Child);
		if (IsValid(MeshChild))
		{
			GeneratedMeshes.Add(MeshChild);
		}
	}

	float CurrentPos = 0;
	int MeshIndex = 0;
	while (CurrentPos < SplineLength - LengthPerMesh)
	{
		USplineMeshComponent* MeshSegment;
		if (MeshIndex < GeneratedMeshes.Num())
		{
			MeshSegment = GeneratedMeshes[MeshIndex];
		}
		else
		{
			// EComponentCreationMethod::UserConstructionScript just does not play well with C++, so we are creating them as normal
			// components and keeping track of them manually
			MeshSegment = NewObject<USplineMeshComponent>(this);
			MeshSegment->SetStaticMesh(SplineMesh);
			MeshSegment->SetMobility(EComponentMobility::Movable);
			MeshSegment->SetupAttachment(SplineComponent);
			MeshSegment->RegisterComponent();
			GeneratedMeshes.Add(MeshSegment);
		}

		MeshSegment->SetCollisionEnabled(CollisionEnabled);
		MeshSegment->SetCollisionProfileName(CollisionProfile);
		MeshSegment->SetPhysMaterialOverride(PhysicalMaterial);
		MeshSegment->SetForwardAxis(ForwardAxis);
		MeshSegment->bSmoothInterpRollScale = bSmoothInterpRollScale;
		MeshSegment->SetVisibility(true);
		MeshSegment->bIsEditorOnly = false;

		float StartDist = CurrentPos;
		float MiddleDist = CurrentPos + LengthPerMesh * .5f;
		float EndDist = CurrentPos + LengthPerMesh;

		FVector StartTangent = SplineComponent->GetDirectionAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
		StartTangent *= LengthPerMesh;

		FVector EndTangent = SplineComponent->GetDirectionAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);
		EndTangent *= LengthPerMesh;
		MeshSegment->SetStartAndEnd(SplineComponent->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local),
			StartTangent, SplineComponent->GetLocationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local), EndTangent);

		FVector StartScale = SplineComponent->GetScaleAtDistanceAlongSpline(StartDist);
		MeshSegment->SetStartScale(FVector2D(StartScale.Y, StartScale.Z));
		FVector EndScale = SplineComponent->GetScaleAtDistanceAlongSpline(EndDist);
		MeshSegment->SetEndScale(FVector2D(EndScale.Y, EndScale.Z));

		MeshSegment->SetSplineUpDir(SplineComponent->GetUpVectorAtDistanceAlongSpline(MiddleDist, ESplineCoordinateSpace::Local));
		FRotator StartRotator = SplineComponent->GetRotationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
		FRotator MiddleRotator = SplineComponent->GetRotationAtDistanceAlongSpline(MiddleDist, ESplineCoordinateSpace::Local);
		FRotator EndRotator = SplineComponent->GetRotationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);

		FRotator InverseMiddleRotator = MiddleRotator.GetInverse();
		float StartRoll = FRotator(InverseMiddleRotator.Quaternion() * StartRotator.Quaternion()).Roll;
		float EndRoll = FRotator(InverseMiddleRotator.Quaternion() * EndRotator.Quaternion()).Roll;

		MeshSegment->SetStartRoll(FMath::DegreesToRadians(StartRoll));
		MeshSegment->SetEndRoll(FMath::DegreesToRadians(EndRoll));

		CurrentPos += LengthPerMesh;
		++MeshIndex;
	}

	while (MeshIndex < GeneratedMeshes.Num())
	{
		GeneratedMeshes[GeneratedMeshes.Num() - 1]->DestroyComponent();
		GeneratedMeshes.RemoveAt(GeneratedMeshes.Num() - 1);
	}
}

void AMultiSplineMesh::SpawnItems()
{
	if (IsValid(SpawnedItem))
	{
		int StartNode = FMath::Clamp(SpawnedItemStartNode, 0, SplineComponent->GetNumberOfSplinePoints() - 1);
		int EndNode = FMath::Clamp(SpawnedItemEndNode, 0, SplineComponent->GetNumberOfSplinePoints() - 1);
		if (SpawnedItemStartNode == 0 && SpawnedItemEndNode == 0)
		{
			StartNode = 0;
			EndNode = SplineComponent->GetNumberOfSplinePoints() - 1;
		}
		if (EndNode > StartNode)
		{
			float DistStart = SplineComponent->GetDistanceAlongSplineAtSplinePoint(StartNode);
			float DistEnd = SplineComponent->GetDistanceAlongSplineAtSplinePoint(EndNode);
			float DistDiff = DistEnd - DistStart;
			for (int i = 0; i < NumSpawnedItems; ++i)
			{
				float Dist = (static_cast<float>(i) / static_cast<float>(NumSpawnedItems - 1)) * DistDiff + DistStart;

				UChildActorComponent* Child = NewObject<UChildActorComponent>(this);
				Child->SetChildActorClass(SpawnedItem);
				Child->CreationMethod = EComponentCreationMethod::UserConstructionScript;
				Child->SetupAttachment(SplineComponent);
				Child->RegisterComponent();
				FVector Pos = SplineComponent->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
				FRotator Rotator = SplineComponent->GetRotationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
				FVector ScaledOffset = SpawnedItemOffset * SplineComponent->GetScaleAtDistanceAlongSpline(Dist);
				Child->SetWorldLocation(Pos + Rotator.RotateVector(ScaledOffset));
				Child->SetWorldRotation(Rotator);
			}
		}
	}
}

void AMultiSplineMesh::CreatePointsPerMesh()
{
	TArray<FSplinePoint> GeneratedPoints;

	TArray<USceneComponent*> ChildComponents;
	SplineComponent->GetChildrenComponents(false, ChildComponents);
	float LengthPerMesh = SplineMesh->GetBoundingBox().GetSize().X;
	float SplineLength = SplineComponent->GetSplineLength();
	float CurrentPos = LengthPerMesh;
	int InputKey = SplineComponent->SplineCurves.Position.Points[0].InVal + 1.0f;
	while (CurrentPos < SplineLength)
	{
		// GeneratedPoints.Add(FSplinePoint(SplineComponent->GetInputKeyAtDistanceAlongSpline(CurrentPos),
		GeneratedPoints.Add(FSplinePoint(InputKey++,
			SplineComponent->GetLocationAtDistanceAlongSpline(CurrentPos, ESplineCoordinateSpace::Local), ESplinePointType::Curve,
			SplineComponent->GetRotationAtDistanceAlongSpline(CurrentPos, ESplineCoordinateSpace::Local),
			SplineComponent->GetScaleAtDistanceAlongSpline(CurrentPos)));

		CurrentPos += LengthPerMesh;
	}

	for (int i = SplineComponent->GetNumberOfSplinePoints() - 1; i >= 1; --i)
	{
		SplineComponent->RemoveSplinePoint(i);
	}
	SplineComponent->AddPoints(GeneratedPoints, false);
	SplineComponent->UpdateSpline();
}

void AMultiSplineMesh::ReconstructMeshes()
{
	TArray<USceneComponent*> ChildrenComponents;
	SplineComponent->GetChildrenComponents(false, ChildrenComponents);
	for (USceneComponent* Child : ChildrenComponents)
	{
		USplineMeshComponent* MeshChild = Cast<USplineMeshComponent>(Child);
		if (IsValid(MeshChild))
		{
			MeshChild->DestroyComponent();
		}
	}
	GeneratedMeshes.Empty();
	ConstructMeshes();
}
