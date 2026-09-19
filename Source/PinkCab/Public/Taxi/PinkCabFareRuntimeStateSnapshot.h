#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabFareLoopCoordinator.h"

struct FPinkCabFareSessionSnapshot
{
    FString FareId;
    EPinkCabFareState State = EPinkCabFareState::Boarding;
    bool bPaymentCommitted = false;
    bool bReceiptResolved = false;
    bool bReceiptTaken = false;
    bool bStopoverActive = false;
    bool bRequiresWorkdayEnd = false;
    bool bRequiresRepairRecovery = false;
};

struct FPinkCabTaximeterSnapshot
{
    FPinkCabFarePricingTerms Terms;
    double DistanceKm = 0.0;
    double FareSeconds = 0.0;
    bool bStarted = false;
    bool bRunning = false;
};

struct FPinkCabFarePassengerRecordSnapshot
{
    FString PassengerId;
    EPinkCabPassengerSeat Seat = EPinkCabPassengerSeat::Rear1;
    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

struct FPinkCabFarePassengerManifestSnapshot
{
    FString FareId;
    TArray<FPinkCabFarePassengerRecordSnapshot> Records;
    float TotalPassengerMassKg = 0.0f;
    bool bBoarded = false;
    bool bExited = false;
};

struct FPinkCabFareRuntimeStateSnapshot
{
    FString FareId;
    EPinkCabFareLoopState LoopState = EPinkCabFareLoopState::Offered;
    bool bInitialized = false;
    FPinkCabFareSessionSnapshot Session;
    FPinkCabTaximeterSnapshot Taximeter;
    FPinkCabFarePassengerManifestSnapshot Manifest;
};
