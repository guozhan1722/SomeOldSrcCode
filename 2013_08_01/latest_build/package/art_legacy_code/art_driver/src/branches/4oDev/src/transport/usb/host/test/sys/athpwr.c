/*++

Module Name:

    athpwr.c

Abstract:

    The power management related processing.

    The Power Manager uses IRPs to direct drivers to change system
    and device power levels, to respond to system wake-up events,
    and to query drivers about their devices. All power IRPs have
    the major function code IRP_MJ_POWER.

    Most function and filter drivers perform some processing for
    each power IRP, then pass the IRP down to the next lower driver
    without completing it. Eventually the IRP reaches the bus driver,
    which physically changes the power state of the device and completes
    the IRP.

    When the IRP has been completed, the I/O Manager calls any
    IoCompletion routines set by drivers as the IRP traveled
    down the device stack. Whether a driver needs to set a completion
    routine depends upon the type of IRP and the driver's individual
    requirements.

    This code is not USB specific. It is essential for every WDM driver
    to handle power irps.

Environment:

    Kernel mode

Notes:
    

--*/

#include "precomp.h"

NTSTATUS
AthUsb_DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
 
Routine Description:

    The power dispatch routine.

Arguments:

    DeviceObject - pointer to a device object.

    Irp - pointer to an I/O Request Packet.

Return Value:

    NT status code

--*/
{
    NTSTATUS           ntStatus;
    PIO_STACK_LOCATION irpStack;
    PUNICODE_STRING    tagString;
    PDEVICE_EXTENSION  deviceExtension;
	
    //
    // initialize the variables
    //
	
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    //
    // We don't queue power Irps, we'll only check if the
    // device was removed, otherwise we'll take appropriate
    // action and send it to the next lower driver. In general
    // drivers should not cause long delays while handling power
    // IRPs. If a driver cannot handle a power IRP in a brief time,
    // it should return STATUS_PENDING and queue all incoming
    // IRPs until the IRP completes.
    //

    if(Removed == deviceExtension->DeviceState) {

        //
        // Even if a driver fails the IRP, it must nevertheless call
        // PoStartNextPowerIrp to inform the Power Manager that it
        // is ready to handle another power IRP.
        //

        PoStartNextPowerIrp(Irp);

        Irp->IoStatus.Status = ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return ntStatus;
    }

    if(NotStarted == deviceExtension->DeviceState) {

        //
        // if the device is not started yet, pass it down
        //

        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        return PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    }

    AthUsb_DbgPrint(3, ("AthUsb_DispatchPower::"));
    AthUsb_IoIncrement(deviceExtension);
    
    switch(irpStack->MinorFunction) {
    
    case IRP_MN_SET_POWER:

        //
        // The Power Manager sends this IRP for one of the
        // following reasons:
        // 1) To notify drivers of a change to the system power state.
        // 2) To change the power state of a device for which
        //    the Power Manager is performing idle detection.
        // A driver sends IRP_MN_SET_POWER to change the power
        // state of its device if it's a power policy owner for the
        // device.
        //

        IoMarkIrpPending(Irp);

        switch(irpStack->Parameters.Power.Type) {

        case SystemPowerState:

            HandleSystemSetPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;

        case DevicePowerState:

            HandleDeviceSetPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;
        }

        break;

    case IRP_MN_QUERY_POWER:

        //
        // The Power Manager sends a power IRP with the minor
        // IRP code IRP_MN_QUERY_POWER to determine whether it
        // can safely change to the specified system power state
        // (S1-S5) and to allow drivers to prepare for such a change.
        // If a driver can put its device in the requested state,
        // it sets status to STATUS_SUCCESS and passes the IRP down.
        //

        IoMarkIrpPending(Irp);
    
        switch(irpStack->Parameters.Power.Type) {

        case SystemPowerState:
            
            HandleSystemQueryPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;

        case DevicePowerState:

            HandleDeviceQueryPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;
        }

        break;

    case IRP_MN_WAIT_WAKE:

        //
        // The minor power IRP code IRP_MN_WAIT_WAKE provides
        // for waking a device or waking the system. Drivers
        // of devices that can wake themselves or the system
        // send IRP_MN_WAIT_WAKE. The system sends IRP_MN_WAIT_WAKE
        // only to devices that always wake the system, such as
        // the power-on switch.
        //

        IoMarkIrpPending(Irp);

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                        Irp,
                        (PIO_COMPLETION_ROUTINE)WaitWakeCompletionRoutine,
                        deviceExtension, 
                        TRUE, 
                        TRUE, 
                        TRUE);        

        PoStartNextPowerIrp(Irp);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(!NT_SUCCESS(ntStatus)) {

            AthUsb_DbgPrint(1, ("Lower drivers failed the wait-wake Irp\n"));            
        }

        ntStatus = STATUS_PENDING;

        //
        // push back the count HERE and NOT in completion routine
        // a pending Wait Wake Irp should not impede stopping the device
        //

        AthUsb_DbgPrint(3, ("IRP_MN_WAIT_WAKE::"));
        AthUsb_IoDecrement(deviceExtension);

        break;

    case IRP_MN_POWER_SEQUENCE:

        //
        // A driver sends this IRP as an optimization to determine
        // whether its device actually entered a specific power state.
        // This IRP is optional. Power Manager cannot send this IRP.
        //

    default:

        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(!NT_SUCCESS(ntStatus)) {

            AthUsb_DbgPrint(1, ("Lower drivers failed default power Irp\n"));
        }
        
        AthUsb_DbgPrint(3, ("AthUsb_DispatchPower::"));
        AthUsb_IoDecrement(deviceExtension);

        break;
    }

    return ntStatus;
}

NTSTATUS
HandleSystemQueryPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
 
Routine Description:

    This routine handles the irp with minor function of type IRP_MN_QUERY_POWER
    for the system power states.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the power manager.

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    SYSTEM_POWER_STATE systemState;
    PIO_STACK_LOCATION irpStack;
    
    AthUsb_DbgPrint(3, ("HandleSystemQueryPower - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    systemState = irpStack->Parameters.Power.State.SystemState;

    AthUsb_DbgPrint(3, ("Query for system power state S%X\n"
                         "Current system power state S%X\n",
                         systemState - 1,
                         deviceExtension->SysPower - 1));

    //
    // if querying for a lower S-state, issue a wait-wake
    //

    if((systemState > deviceExtension->SysPower) &&
       (deviceExtension->WaitWakeEnable)) {

        IssueWaitWake(deviceExtension);
    }

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
            Irp, 
            (PIO_COMPLETION_ROUTINE)SysPoCompletionRoutine,
            deviceExtension, 
            TRUE, 
            TRUE, 
            TRUE);

    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    AthUsb_DbgPrint(3, ("HandleSystemQueryPower - ends\n"));

    return STATUS_PENDING;
}

NTSTATUS
HandleSystemSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
 
Routine Description:

    This routine services irps of minor type IRP_MN_SET_POWER
    for the system power state

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the power manager

Return Value:

    NT status value:

--*/
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    SYSTEM_POWER_STATE systemState;
    PIO_STACK_LOCATION irpStack;
    
    AthUsb_DbgPrint(3, ("HandleSystemSetPower - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    systemState = irpStack->Parameters.Power.State.SystemState;

    AthUsb_DbgPrint(3, ("Set request for system power state S%X\n"
                         "Current system power state S%X\n",
                         systemState - 1,
                         deviceExtension->SysPower - 1));

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
            Irp, 
            (PIO_COMPLETION_ROUTINE)SysPoCompletionRoutine,
            deviceExtension, 
            TRUE, 
            TRUE, 
            TRUE);

    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    AthUsb_DbgPrint(3, ("HandleSystemSetPower - ends\n"));

    return STATUS_PENDING;
}

NTSTATUS
HandleDeviceQueryPower(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    )
/*++
 
Routine Description:

    This routine services irps of minor type IRP_MN_QUERY_POWER
    for the device power state

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the power manager

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;
    DEVICE_POWER_STATE deviceState;

    AthUsb_DbgPrint(3, ("HandleDeviceQueryPower - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceState = irpStack->Parameters.Power.State.DeviceState;

    AthUsb_DbgPrint(3, ("Query for device power state D%X\n"
                         "Current device power state D%X\n",
                         deviceState - 1,
                         deviceExtension->DevPower - 1));

    if(deviceState < deviceExtension->DevPower) {

        ntStatus = STATUS_SUCCESS;
    }
    else {

        ntStatus = HoldIoRequests(DeviceObject, Irp);

        if(STATUS_PENDING == ntStatus) {

            return ntStatus;
        }
    }

    //
    // on error complete the Irp.
    // on success pass it to the lower layers
    //

    PoStartNextPowerIrp(Irp);

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    if(!NT_SUCCESS(ntStatus)) {

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    }

    AthUsb_DbgPrint(3, ("HandleDeviceQueryPower::"));
    AthUsb_IoDecrement(deviceExtension);

    AthUsb_DbgPrint(3, ("HandleDeviceQueryPower - ends\n"));

    return ntStatus;
}


NTSTATUS
SysPoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++
 
Routine Description:

    This is the completion routine for the system power irps of minor
    function types IRP_MN_QUERY_POWER and IRP_MN_SET_POWER.
    This completion routine sends the corresponding device power irp and
    returns STATUS_MORE_PROCESSING_REQUIRED. The system irp is passed as a
    context to the device power irp completion routine and is completed in
    the device power irp completion routine.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
 	PIO_STACK_LOCATION irpStack;

    //
    // initialize variables
    //
    ntStatus = Irp->IoStatus.Status;
    irpStack = IoGetCurrentIrpStackLocation(Irp);


    AthUsb_DbgPrint(3, ("SysPoCompletionRoutine - begins\n"));

    //
    // lower drivers failed this Irp
    //

    if(!NT_SUCCESS(ntStatus)) {

        PoStartNextPowerIrp(Irp);

        AthUsb_DbgPrint(3, ("SysPoCompletionRoutine::"));
        AthUsb_IoDecrement(DeviceExtension);

        return STATUS_SUCCESS;
    }

    //
    // ..otherwise update the cached system power state (IRP_MN_SET_POWER)
    //

    if(irpStack->MinorFunction == IRP_MN_SET_POWER) {

        DeviceExtension->SysPower = irpStack->Parameters.Power.State.SystemState;
    }

    //
    // queue device irp and return STATUS_MORE_PROCESSING_REQUIRED
    //
	
    SendDeviceIrp(DeviceObject, Irp);    

    AthUsb_DbgPrint(3, ("SysPoCompletionRoutine - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SendDeviceIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP SIrp
    )
/*++
 
Routine Description:

    This routine is invoked from the completion routine of the system power
    irp. This routine will PoRequest a device power irp. The system irp is 
    passed as a context to the the device power irp.

Arguments:

    DeviceObject - pointer to device object
    SIrp - system power irp.

Return Value:

    None

--*/
{
    NTSTATUS                  ntStatus;
    POWER_STATE               powState;
    PDEVICE_EXTENSION         deviceExtension;
    PIO_STACK_LOCATION        irpStack;
    SYSTEM_POWER_STATE        systemState;
    DEVICE_POWER_STATE        devState;
    PPOWER_COMPLETION_CONTEXT powerContext;
    
    //
    // initialize variables
    //

    irpStack = IoGetCurrentIrpStackLocation(SIrp);
    systemState = irpStack->Parameters.Power.State.SystemState;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    AthUsb_DbgPrint(3, ("SendDeviceIrp - begins\n"));

    //
    // Read out the D-IRP out of the S->D mapping array captured in QueryCap's.
    // we can choose deeper sleep states than our mapping but never choose
    // lighter ones.
    //

    devState = deviceExtension->DeviceCapabilities.DeviceState[systemState];
    powState.DeviceState = devState;
    
    powerContext = (PPOWER_COMPLETION_CONTEXT) 
                   ExAllocatePool(NonPagedPool,
                                  sizeof(POWER_COMPLETION_CONTEXT));

    if(!powerContext) {

        AthUsb_DbgPrint(1, ("Failed to alloc memory for powerContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {

        powerContext->DeviceObject = DeviceObject;
        powerContext->SIrp = SIrp;
   
        //
        // in win2k PoRequestPowerIrp can take fdo or pdo.
        //

        ntStatus = PoRequestPowerIrp(
                            deviceExtension->PhysicalDeviceObject, 
                            irpStack->MinorFunction,
                            powState,
                            (PREQUEST_POWER_COMPLETE)DevPoCompletionRoutine,
                            powerContext, 
                            NULL);
    }

    if(!NT_SUCCESS(ntStatus)) {

        if(powerContext) {

            ExFreePool(powerContext);
        }

        PoStartNextPowerIrp(SIrp);

        SIrp->IoStatus.Status = ntStatus;
        SIrp->IoStatus.Information = 0;
        
        IoCompleteRequest(SIrp, IO_NO_INCREMENT);

        AthUsb_DbgPrint(3, ("SendDeviceIrp::"));
        AthUsb_IoDecrement(deviceExtension);

    }

    AthUsb_DbgPrint(3, ("SendDeviceIrp - ends\n"));

    return ntStatus;
}


VOID
DevPoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject, 
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
/*++
 
Routine Description:

    This is the PoRequest - completion routine for the device power irp.
    This routine is responsible for completing the system power irp, 
    received as a context.

Arguments:

    DeviceObject - pointer to device object
    MinorFunction - minor function of the irp.
    PowerState - power state of the irp.
    Context - context passed to the completion routine.
    IoStatus - status of the device power irp.

Return Value:

    None

--*/
{
    PIRP                      sIrp;
    PDEVICE_EXTENSION         deviceExtension;
    PPOWER_COMPLETION_CONTEXT powerContext;
    
    //
    // initialize variables
    //

    powerContext = (PPOWER_COMPLETION_CONTEXT) Context;
    sIrp = powerContext->SIrp;
    deviceExtension = powerContext->DeviceObject->DeviceExtension;

    AthUsb_DbgPrint(3, ("DevPoCompletionRoutine - begins\n"));

    //
    // copy the D-Irp status into S-Irp
    //

    sIrp->IoStatus.Status = IoStatus->Status;

    //
    // complete the system Irp
    //
    
    PoStartNextPowerIrp(sIrp);

    sIrp->IoStatus.Information = 0;

    IoCompleteRequest(sIrp, IO_NO_INCREMENT);

    //
    // cleanup
    //
    
    AthUsb_DbgPrint(3, ("DevPoCompletionRoutine::"));
    AthUsb_IoDecrement(deviceExtension);

    ExFreePool(powerContext);

    AthUsb_DbgPrint(3, ("DevPoCompletionRoutine - ends\n"));

}

NTSTATUS
HandleDeviceSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
 
Routine Description:

    This routine services irps of minor type IRP_MN_SET_POWER
    for the device power state

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the power manager

Return Value:

    NT status value

--*/
{
    KIRQL              oldIrql;
    NTSTATUS           ntStatus;
    POWER_STATE        newState;    
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION  deviceExtension;
    DEVICE_POWER_STATE newDevState,
                       oldDevState;

    AthUsb_DbgPrint(3, ("HandleDeviceSetPower - begins\n"));
	
    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    oldDevState = deviceExtension->DevPower;
    newState = irpStack->Parameters.Power.State;
    newDevState = newState.DeviceState;

    AthUsb_DbgPrint(3, ("Set request for device power state D%X\n"
                         "Current device power state D%X\n",
                         newDevState - 1,
                         deviceExtension->DevPower - 1));

    if(newDevState < oldDevState) {

        //
        // adding power
        //
        AthUsb_DbgPrint(3, ("Adding power to the device\n"));

        //
        // send the power IRP to the next driver in the stack
        //
        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                Irp, 
                (PIO_COMPLETION_ROUTINE)FinishDevPoUpIrp,
                deviceExtension, 
                TRUE, 
                TRUE, 
                TRUE);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

	}
    else {

        //
        // newDevState >= oldDevState 
        //
        // hold I/O if transition from D0 -> DX (X = 1, 2, 3)
        // if transition from D1 or D2 to deeper sleep states, 
        // I/O queue is already on hold.
        //

        if(PowerDeviceD0 == oldDevState && newDevState > oldDevState) {

            //
            // D0 -> DX transition
            //

            AthUsb_DbgPrint(3, ("Removing power from the device\n"));

            ntStatus = HoldIoRequests(DeviceObject, Irp);

            if(!NT_SUCCESS(ntStatus)) {

                PoStartNextPowerIrp(Irp);

                Irp->IoStatus.Status = ntStatus;
                Irp->IoStatus.Information = 0;

                IoCompleteRequest(Irp, IO_NO_INCREMENT);

                AthUsb_DbgPrint(3, ("HandleDeviceSetPower::"));
                AthUsb_IoDecrement(deviceExtension);

                return ntStatus;
            }
            else {

                goto HandleDeviceSetPower_Exit;
            }

        }
        else if(PowerDeviceD0 == oldDevState && PowerDeviceD0 == newDevState) {

            //
            // D0 -> D0
            // unblock the queue which may have been blocked processing
            // query irp
            //

            AthUsb_DbgPrint(3, ("A SetD0 request\n"));

            KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);
              
            deviceExtension->QueueState = AllowRequests;

            KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

            ProcessQueuedRequests(deviceExtension);
        }   

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                Irp, 
                (PIO_COMPLETION_ROUTINE) FinishDevPoDnIrp,
                deviceExtension, 
                TRUE, 
                TRUE, 
                TRUE);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(!NT_SUCCESS(ntStatus)) {

            AthUsb_DbgPrint(1, ("Lower drivers failed a power Irp\n"));
        }

    }

HandleDeviceSetPower_Exit:

    AthUsb_DbgPrint(3, ("HandleDeviceSetPower - ends\n"));

    return STATUS_PENDING;
}

NTSTATUS
FinishDevPoUpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++
 
Routine Description:

    completion routine for the device power UP irp with minor function
    IRP_MN_SET_POWER.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
                        
    //
    // initialize variables
    //

    ntStatus = Irp->IoStatus.Status;

    AthUsb_DbgPrint(3, ("FinishDevPoUpIrp - begins\n"));

    if(Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

    if(!NT_SUCCESS(ntStatus)) {

        PoStartNextPowerIrp(Irp);

        AthUsb_DbgPrint(3, ("FinishDevPoUpIrp::"));
        AthUsb_IoDecrement(DeviceExtension);

        return STATUS_SUCCESS;
    }

    SetDeviceFunctional(DeviceObject, Irp, DeviceExtension);

    AthUsb_DbgPrint(3, ("FinishDevPoUpIrp - ends\n"));
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SetDeviceFunctional(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++
 
Routine Description:

    This routine processes queue of pending irps.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    KIRQL              oldIrql;
    NTSTATUS           ntStatus;
    POWER_STATE        newState;
    PIO_STACK_LOCATION irpStack;
    DEVICE_POWER_STATE newDevState,
                       oldDevState;

    //
    // initialize variables
    //

    ntStatus = Irp->IoStatus.Status;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    newState = irpStack->Parameters.Power.State;
    newDevState = newState.DeviceState;
    oldDevState = DeviceExtension->DevPower;

    AthUsb_DbgPrint(3, ("SetDeviceFunctional - begins\n"));

    //
    // update the cached state
    //
    DeviceExtension->DevPower = newDevState;

    //
    // restore appropriate amount of state to our h/w
    // this driver does not implement partial context
    // save/restore.
    //

    PoSetPowerState(DeviceObject, DevicePowerState, newState);

    if(PowerDeviceD0 == newDevState) {

    //
    // empty existing queue of all pending irps.
    //

        KeAcquireSpinLock(&DeviceExtension->DevStateLock, &oldIrql);

        DeviceExtension->QueueState = AllowRequests;
        
        KeReleaseSpinLock(&DeviceExtension->DevStateLock, oldIrql);

        ProcessQueuedRequests(DeviceExtension);
    }

    PoStartNextPowerIrp(Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("SetDeviceFunctional::"));
    AthUsb_IoDecrement(DeviceExtension);

    AthUsb_DbgPrint(3, ("SetDeviceFunctional - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
FinishDevPoDnIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++
 
Routine Description:

    This routine is the completion routine for device power DOWN irp.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
    POWER_STATE        newState;
    PIO_STACK_LOCATION irpStack;

    //
    // initialize variables
    //
    ntStatus = Irp->IoStatus.Status;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    newState = irpStack->Parameters.Power.State;

    AthUsb_DbgPrint(3, ("FinishDevPoDnIrp - begins\n"));

    if(NT_SUCCESS(ntStatus) && irpStack->MinorFunction == IRP_MN_SET_POWER) {

        //
        // update the cache;
        //

        AthUsb_DbgPrint(3, ("updating cache..\n"));

        DeviceExtension->DevPower = newState.DeviceState;

        PoSetPowerState(DeviceObject, DevicePowerState, newState);
    }

    PoStartNextPowerIrp(Irp);

    AthUsb_DbgPrint(3, ("FinishDevPoDnIrp::"));
    AthUsb_IoDecrement(DeviceExtension);

    AthUsb_DbgPrint(3, ("FinishDevPoDnIrp - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
HoldIoRequests(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++
 
Routine Description:

    This routine is called on query or set power DOWN irp for the device.
    This routine queues a workitem.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    NTSTATUS               ntStatus;
    PIO_WORKITEM           item;
    PDEVICE_EXTENSION      deviceExtension;
    PWORKER_THREAD_CONTEXT context;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    AthUsb_DbgPrint(3, ("HoldIoRequests - begins\n"));

    deviceExtension->QueueState = HoldRequests;

    context = ExAllocatePool(NonPagedPool, sizeof(WORKER_THREAD_CONTEXT));

    if(context) {

        item = IoAllocateWorkItem(DeviceObject);

        context->Irp = Irp;
        context->DeviceObject = DeviceObject;
        context->WorkItem = item;

        if(item) {

            IoMarkIrpPending(Irp);
            
            IoQueueWorkItem(item, HoldIoRequestsWorkerRoutine,
                            DelayedWorkQueue, context);
            
            ntStatus = STATUS_PENDING;
        }
        else {

            AthUsb_DbgPrint(3, ("Failed to allocate memory for workitem\n"));
            ExFreePool(context);
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {

        AthUsb_DbgPrint(1, ("Failed to alloc memory for worker thread context\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    AthUsb_DbgPrint(3, ("HoldIoRequests - ends\n"));

    return ntStatus;
}

VOID
HoldIoRequestsWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
/*++
 
Routine Description:

    This routine waits for the I/O in progress to finish and then
    sends the device power irp (query/set) down the stack.

Arguments:

    DeviceObject - pointer to device object
    Context - context passed to the work-item.

Return Value:

    None

--*/
{
    PIRP                   irp;
    NTSTATUS               ntStatus;
    PDEVICE_EXTENSION      deviceExtension;
    PWORKER_THREAD_CONTEXT context;

    AthUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine - begins\n"));

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    context = (PWORKER_THREAD_CONTEXT) Context;
    irp = (PIRP) context->Irp;


    //
    // wait for I/O in progress to finish.
    // the stop event is signalled when the counter drops to 1.
    // invoke AthUsb_IoDecrement twice: once each for the S-Irp and D-Irp.
    //
    AthUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    AthUsb_IoDecrement(deviceExtension);
    AthUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    AthUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->StopEvent, Executive,
                          KernelMode, FALSE, NULL);

    //
    // Increment twice to restore the count
    //
    AthUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    AthUsb_IoIncrement(deviceExtension);
    AthUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    AthUsb_IoIncrement(deviceExtension);

    // 
    // now send the Irp down
    //

    IoCopyCurrentIrpStackLocationToNext(irp);

    IoSetCompletionRoutine(irp, (PIO_COMPLETION_ROUTINE) FinishDevPoDnIrp,
                           deviceExtension, TRUE, TRUE, TRUE);

    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(!NT_SUCCESS(ntStatus)) {

        AthUsb_DbgPrint(1, ("Lower driver fail a power Irp\n"));
    }

    IoFreeWorkItem(context->WorkItem);
    ExFreePool((PVOID)context);

    AthUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine - ends\n"));

}

NTSTATUS
QueueRequest(
    IN OUT PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
/*++
 
Routine Description:

    Queue the Irp in the device queue

Arguments:

    DeviceExtension - pointer to device extension
    Irp - I/O request packet.

Return Value:

    NT status value

--*/
{
    KIRQL    oldIrql;
    NTSTATUS ntStatus;

    //
    // initialize variables
    //
    ntStatus = STATUS_PENDING;

    AthUsb_DbgPrint(3, ("QueueRequests - begins\n"));

    ASSERT(HoldRequests == DeviceExtension->QueueState);

    KeAcquireSpinLock(&DeviceExtension->QueueLock, &oldIrql);

    InsertTailList(&DeviceExtension->NewRequestsQueue, 
                   &Irp->Tail.Overlay.ListEntry);

    IoMarkIrpPending(Irp);

    //
    // Set the cancel routine
    //

    IoSetCancelRoutine(Irp, CancelQueued);

    KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);

    AthUsb_DbgPrint(3, ("QueueRequests - ends\n"));

    return ntStatus;
}

VOID
CancelQueued(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++
 
Routine Description:

    This routine removes the irp from the queue and completes it with
    STATUS_CANCELLED

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    None.

--*/
{
    PDEVICE_EXTENSION deviceExtension;
    KIRQL             oldIrql;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    oldIrql = Irp->CancelIrql;

    AthUsb_DbgPrint(3, ("CancelQueued - begins\n"));

    //
    // Release the cancel spin lock
    //

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    //
    // Acquire the queue lock
    //

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->QueueLock);

    //
    // Remove the cancelled Irp from queue and release the lock
    //
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

    //
    // complete with STATUS_CANCELLED
    //

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    AthUsb_DbgPrint(3, ("CancelQueued - ends\n"));

    return;
}


NTSTATUS
WaitWakeCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++
 
Routine Description:

    This is the IoSet completion routine for the wait wake irp.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    AthUsb_DbgPrint(3, ("WaitWakeCompletionRoutine - begins\n"));

    if(Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

    //
    // Nullify the WaitWakeIrp pointer-the Irp is released 
    // as part of the completion process. If it's already NULL, 
    // avoid race with the CancelWaitWake routine.
    //

    if(InterlockedExchangePointer(&DeviceExtension->WaitWakeIrp, NULL)) {

        PoStartNextPowerIrp(Irp);

        return STATUS_SUCCESS;
    }

    //
    // CancelWaitWake has run. 
    // If FlagWWCancel != 0, complete the Irp.
    // If FlagWWCancel == 0, CancelWaitWake completes it.
    //
    if(InterlockedExchange(&DeviceExtension->FlagWWCancel, 1)) {

        PoStartNextPowerIrp(Irp);

        return STATUS_CANCELLED;
    }

    AthUsb_DbgPrint(3, ("WaitWakeCompletionRoutine - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

PCHAR
PowerMinorFunctionString (
    IN UCHAR MinorFunction
    )
/*++
 
Routine Description:

Arguments:

Return Value:

--*/
{
    switch (MinorFunction) {

        case IRP_MN_SET_POWER:
            return "IRP_MN_SET_POWER\n";

        case IRP_MN_QUERY_POWER:
            return "IRP_MN_QUERY_POWER\n";

        case IRP_MN_POWER_SEQUENCE:
            return "IRP_MN_POWER_SEQUENCE\n";

        case IRP_MN_WAIT_WAKE:
            return "IRP_MN_WAIT_WAKE\n";

        default:
            return "IRP_MN_?????\n";
    }
}
