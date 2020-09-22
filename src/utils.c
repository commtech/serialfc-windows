/*++

Copyright (c) Microsoft Corporation

Module Name:

    utils.c

Abstract:

    This module contains code that perform queueing and completion
    manipulation on requests.  Also module generic functions such
    as error logging.

Environment:

    Kernel mode

--*/

#include "precomp.h"

#if defined(EVENT_TRACING)
#include "utils.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0,SerialMemCompare)
#pragma alloc_text(PAGESRP0,SerialLogError)
#pragma alloc_text(PAGESRP0,SerialMarkHardwareBroken)
#endif // ALLOC_PRAGMA


VOID
SerialRundownIrpRefs(
    IN WDFREQUEST *CurrentOpRequest,
    IN WDFTIMER IntervalTimer,
    IN WDFTIMER TotalTimer,
    IN PSERIAL_DEVICE_EXTENSION PDevExt,
    IN LONG RefType
    );

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};
void FastcomGetTxFifoFillFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, UCHAR *value);

VOID
SerialPurgeRequests(
    IN WDFQUEUE QueueToClean,
    IN WDFREQUEST *CurrentOpRequest
    )

/*++

Routine Description:

    This function is used to cancel all queued and the current irps
    for reads or for writes. Called at DPC level.

Arguments:

    QueueToClean - A pointer to the queue which we're going to clean out.

    CurrentOpRequest - Pointer to a pointer to the current request.

Return Value:

    None.

--*/

{
    NTSTATUS status;
    PREQUEST_CONTEXT reqContext;

    WdfIoQueuePurge(QueueToClean, WDF_NO_EVENT_CALLBACK, WDF_NO_CONTEXT);

    //
    // The queue is clean.  Now go after the current if
    // it's there.
    //

    if (*CurrentOpRequest) {

        PFN_WDF_REQUEST_CANCEL CancelRoutine;

        reqContext = SerialGetRequestContext(*CurrentOpRequest);
        CancelRoutine = reqContext->CancelRoutine;
        //
        // Clear the common cancel routine but don't clear the reference because the
        // request specific cancel routine called below will clear the reference.
        //
        status = SerialClearCancelRoutine(*CurrentOpRequest, FALSE);
        if (NT_SUCCESS(status)) {
            //
            // Let us just call the CancelRoutine to start the next request.
            //
            if(CancelRoutine) {
                CancelRoutine(*CurrentOpRequest);
            }
        }
    }
}

VOID
SerialFlushRequests(
    IN WDFQUEUE QueueToClean,
    IN WDFREQUEST *CurrentOpRequest
    )

/*++

Routine Description:

    This function is used to cancel all queued and the current irps
    for reads or for writes. Called at DPC level.

Arguments:

    QueueToClean - A pointer to the queue which we're going to clean out.

    CurrentOpRequest - Pointer to a pointer to the current request.

Return Value:

    None.

--*/

{
    SerialPurgeRequests(QueueToClean,  CurrentOpRequest);

    //
    // Since purge puts the queue state to fail requests, we have to explicitly
    // change the queue state to accept requests.
    //
    WdfIoQueueStart(QueueToClean);

}


VOID
SerialGetNextRequest(
    IN WDFREQUEST               * CurrentOpRequest,
    IN WDFQUEUE                   QueueToProcess,
    OUT WDFREQUEST              * NextRequest,
    IN BOOLEAN                    CompleteCurrent,
    IN PSERIAL_DEVICE_EXTENSION   Extension
    )

/*++

Routine Description:

    This function is used to make the head of the particular
    queue the current request.  It also completes the what
    was the old current request if desired.

Arguments:

    CurrentOpRequest - Pointer to a pointer to the currently active
                   request for the particular work list.  Note that
                   this item is not actually part of the list.

    QueueToProcess - The list to pull the new item off of.

    NextIrp - The next Request to process.  Note that CurrentOpRequest
              will be set to this value under protection of the
              cancel spin lock.  However, if *NextIrp is NULL when
              this routine returns, it is not necessaryly true the
              what is pointed to by CurrentOpRequest will also be NULL.
              The reason for this is that if the queue is empty
              when we hold the cancel spin lock, a new request may come
              in immediately after we release the lock.

    CompleteCurrent - If TRUE then this routine will complete the
                      request pointed to by the pointer argument
                      CurrentOpRequest.

Return Value:

    None.

--*/

{
    WDFREQUEST       oldRequest = NULL;
    PREQUEST_CONTEXT reqContext;
    NTSTATUS         status;

    UNREFERENCED_PARAMETER(Extension);

    oldRequest = *CurrentOpRequest;
    *CurrentOpRequest = NULL;

    //
    // Check to see if there is a new request to start up.
    //

    status = WdfIoQueueRetrieveNextRequest(
                 QueueToProcess,
                 CurrentOpRequest
                 );

    if(!NT_SUCCESS(status)) {
        ASSERTMSG("WdfIoQueueRetrieveNextRequest failed",
                  status == STATUS_NO_MORE_ENTRIES);
    }

    *NextRequest = *CurrentOpRequest;

    if (CompleteCurrent) {

        if (oldRequest) {

            reqContext = SerialGetRequestContext(oldRequest);

            SerialCompleteRequest(oldRequest,
                                  reqContext->Status,
                                  reqContext->Information);
        }
    }
}

VOID
SerialTryToCompleteCurrent(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PFN_WDF_INTERRUPT_SYNCHRONIZE  SynchRoutine OPTIONAL,
    IN NTSTATUS StatusToUse,
    IN WDFREQUEST *CurrentOpRequest,
    IN WDFQUEUE QueueToProcess OPTIONAL,
    IN WDFTIMER IntervalTimer OPTIONAL,
    IN WDFTIMER TotalTimer OPTIONAL,
    IN PSERIAL_START_ROUTINE Starter OPTIONAL,
    IN PSERIAL_GET_NEXT_ROUTINE GetNextRequest OPTIONAL,
    IN LONG RefType
    )

/*++

Routine Description:

    This routine attempts to remove all of the reasons there are
    references on the current read/write.  If everything can be completed
    it will complete this read/write and try to start another.

    NOTE: This routine assumes that it is called with the cancel
          spinlock held.

Arguments:

    Extension - Simply a pointer to the device extension.

    SynchRoutine - A routine that will synchronize with the isr
                   and attempt to remove the knowledge of the
                   current request from the isr.  NOTE: This pointer
                   can be null.

    IrqlForRelease - This routine is called with the cancel spinlock held.
                     This is the irql that was current when the cancel
                     spinlock was acquired.

    StatusToUse - The request's status field will be set to this value, if
                  this routine can complete the request.


Return Value:

    None.

--*/

{
    PREQUEST_CONTEXT reqContext;

    ASSERTMSG("SerialTryToCompleteCurrent: CurrentOpRequest is NULL", *CurrentOpRequest);

     reqContext = SerialGetRequestContext(*CurrentOpRequest);

    if(RefType == SERIAL_REF_ISR || RefType == SERIAL_REF_XOFF_REF) {
        //
        // We can decrement the reference to "remove" the fact
        // that the caller no longer will be accessing this request.
        //

        SERIAL_CLEAR_REFERENCE(
            reqContext,
            RefType
            );
    }

    if (SynchRoutine) {

        WdfInterruptSynchronize(
            Extension->WdfInterrupt,
            SynchRoutine,
            Extension
            );

    }

    //
    // Try to run down all other references to this request.
    //

    SerialRundownIrpRefs(
        CurrentOpRequest,
        IntervalTimer,
        TotalTimer,
        Extension,
        RefType
        );

    if(StatusToUse == STATUS_CANCELLED) {
        //
        // This function is called from a cancelroutine. So mark
        // the request as cancelled. We need to do this because
        // we may not complete the request below if somebody
        // else has a reference to it.
        // This state variable was added to avoid calling
        // WdfRequestMarkCancelable second time on a request that
        // has cancelled but wasn't completed in the cancel routine.
        //
        reqContext->Cancelled = TRUE;
    }

    //
    // See if the ref count is zero after trying to complete everybody else.
    //

    if (!SERIAL_REFERENCE_COUNT(reqContext)) {

        WDFREQUEST newRequest;


        //
        // The ref count was zero so we should complete this
        // request.
        //
        // The following call will also cause the current request to be
        // completed.
        //

        reqContext->Status = StatusToUse;

        if (StatusToUse == STATUS_CANCELLED) {

            reqContext->Information = 0;

        }

        if (GetNextRequest) {

            GetNextRequest(
                CurrentOpRequest,
                QueueToProcess,
                &newRequest,
                TRUE,
                Extension
                );

            if (newRequest) {

                Starter(Extension);

            }

        } else {

            WDFREQUEST oldRequest = *CurrentOpRequest;

            //
            // There was no get next routine.  We will simply complete
            // the request.  We should make sure that we null out the
            // pointer to the pointer to this request.
            //

            *CurrentOpRequest = NULL;

            SerialCompleteRequest(oldRequest,
                                  reqContext->Status,
                                  reqContext->Information);
        }

    } else {


    }

}


VOID
SerialEvtIoStop(
    IN WDFQUEUE                 Queue,
    IN WDFREQUEST               Request,
    IN ULONG                    ActionFlags
    )
/*++

Routine Description:

     This callback is invoked for every request pending in the driver (not queue) -
     in-flight request. The Action parameter tells us why the callback is invoked -
     because the device is being stopped, removed or suspended. In this
     driver, we have told the framework not to stop or remove when there
     are pending requests, so only reason for this callback is when the system is
     suspending.

Arguments:

    Queue - Queue the request currently belongs to
    Request - Request that is currently out of queue and being processed by the driver
    Action - Reason for this callback

Return Value:

    None. Acknowledge the request so that framework can contiue suspending the
    device.

--*/
{
    PREQUEST_CONTEXT reqContext;

    UNREFERENCED_PARAMETER(Queue);

    reqContext = SerialGetRequestContext(Request);

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_WRITE,
                    "--> SerialEvtIoStop %x %p\n", ActionFlags, Request);

    //
    // System suspends all the timers before asking the driver to goto
    // sleep. So let us not worry about cancelling the timers. Also the
    // framework will disconnect the interrupt before calling our
    // D0Exit handler so we can be sure that nobody will touch the hardware.
    // So just acknowledge callback to say that we are okay to stop due to
    // system suspend. Please note that since we have taken a power reference
    // we will never idle out when there is an open handle. Also we have told
    // the framework to not stop for resource rebalancing or remove when there are
    // open handles, so let us not worry about that either.
    //
    if (ActionFlags & WdfRequestStopRequestCancelable) {
        PFN_WDF_REQUEST_CANCEL cancelRoutine;

        //
        // Request is in a cancelable state. So unmark cancelable before you
        // acknowledge. We will mark the request cancelable when we resume.
        //
        cancelRoutine = reqContext->CancelRoutine;

        SerialClearCancelRoutine(Request, TRUE);

        //
        // SerialClearCancelRoutine clears the cancel-routine. So set it back
        // in the context. We will need that when we resume.
        //
        reqContext->CancelRoutine = cancelRoutine;

        reqContext->MarkCancelableOnResume = TRUE;

        ActionFlags &= ~WdfRequestStopRequestCancelable;
    }

    ASSERT(ActionFlags == WdfRequestStopActionSuspend);

    WdfRequestStopAcknowledge(Request, FALSE); // Don't requeue the request

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_WRITE,
                        "<-- SerialEvtIoStop \n");
}

VOID
SerialEvtIoResume(
    IN WDFQUEUE   Queue,
    IN WDFREQUEST Request
    )
/*++

Routine Description:

     This callback is invoked for every request pending in the driver - in-flight
     request - to notify that the hardware is ready for contiuing the processing
     of the request.

Arguments:

    Queue - Queue the request currently belongs to
    Request - Request that is currently out of queue and being processed by the driver

Return Value:

    None.

--*/
{
    PREQUEST_CONTEXT reqContext;

    UNREFERENCED_PARAMETER(Queue);

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_WRITE,
            "--> SerialEvtIoResume %p \n", Request);

    reqContext = SerialGetRequestContext(Request);

    //
    // If we unmarked cancelable on suspend, let us mark it cancelable again.
    //
    if (reqContext->MarkCancelableOnResume) {
        SerialSetCancelRoutine(Request, reqContext->CancelRoutine);
        reqContext->MarkCancelableOnResume = FALSE;
    }

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_WRITE,
        "<-- SerialEvtIoResume \n");
}

VOID
SerialRundownIrpRefs(
    IN WDFREQUEST *CurrentOpRequest,
    IN WDFTIMER IntervalTimer OPTIONAL,
    IN WDFTIMER TotalTimer OPTIONAL,
    IN PSERIAL_DEVICE_EXTENSION PDevExt,
    IN LONG RefType
    )

/*++

Routine Description:

    This routine runs through the various items that *could*
    have a reference to the current read/write.  It try's to remove
    the reason.  If it does succeed in removing the reason it
    will decrement the reference count on the request.

    NOTE: This routine assumes that it is called with the cancel
          spin lock held.

Arguments:

    CurrentOpRequest - Pointer to a pointer to current request for the
                   particular operation.

    IntervalTimer - Pointer to the interval timer for the operation.
                    NOTE: This could be null.

    TotalTimer - Pointer to the total timer for the operation.
                 NOTE: This could be null.

    PDevExt - Pointer to device extension

Return Value:

    None.

--*/


{
    PREQUEST_CONTEXT  reqContext;
    WDFREQUEST        request = *CurrentOpRequest;

    reqContext = SerialGetRequestContext(request);

    if(RefType == SERIAL_REF_CANCEL) {
        //
        // Caller is a cancel routine. So just clear the reference.
        //
        SERIAL_CLEAR_REFERENCE( reqContext,  SERIAL_REF_CANCEL );
        reqContext->CancelRoutine = NULL;

    } else {
        //
        // Try to clear the cancelable state.
        //
        SerialClearCancelRoutine(request, TRUE);
    }
    if (IntervalTimer) {

        //
        // Try to cancel the operations interval timer.  If the operation
        // returns true then the timer did have a reference to the
        // request.  Since we've canceled this timer that reference is
        // no longer valid and we can decrement the reference count.
        //
        // If the cancel returns false then this means either of two things:
        //
        // a) The timer has already fired.
        //
        // b) There never was an interval timer.
        //
        // In the case of "b" there is no need to decrement the reference
        // count since the "timer" never had a reference to it.
        //
        // In the case of "a", then the timer itself will be coming
        // along and decrement it's reference.  Note that the caller
        // of this routine might actually be the this timer, so
        // decrement the reference.
        //

        if (SerialCancelTimer(IntervalTimer, PDevExt)) {

            SERIAL_CLEAR_REFERENCE(
                reqContext,
                SERIAL_REF_INT_TIMER
                );

        } else if(RefType == SERIAL_REF_INT_TIMER) { // caller is the timer

            SERIAL_CLEAR_REFERENCE(
                reqContext,
                SERIAL_REF_INT_TIMER
                );
        }

    }

    if (TotalTimer) {

        //
        // Try to cancel the operations total timer.  If the operation
        // returns true then the timer did have a reference to the
        // request.  Since we've canceled this timer that reference is
        // no longer valid and we can decrement the reference count.
        //
        // If the cancel returns false then this means either of two things:
        //
        // a) The timer has already fired.
        //
        // b) There never was an total timer.
        //
        // In the case of "b" there is no need to decrement the reference
        // count since the "timer" never had a reference to it.
        //
        // In the case of "a", then the timer itself will be coming
        // along and decrement it's reference.  Note that the caller
        // of this routine might actually be the this timer, so
        // decrement the reference.
        //

        if (SerialCancelTimer(TotalTimer, PDevExt)) {

            SERIAL_CLEAR_REFERENCE(
                reqContext,
                SERIAL_REF_TOTAL_TIMER
                );

        } else if(RefType == SERIAL_REF_TOTAL_TIMER) { // caller is the timer

            SERIAL_CLEAR_REFERENCE(
                reqContext,
                SERIAL_REF_TOTAL_TIMER
                );
        }
    }
}


VOID
SerialStartOrQueue(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN WDFREQUEST Request,
    IN WDFQUEUE QueueToExamine,
    IN WDFREQUEST *CurrentOpRequest,
    IN PSERIAL_START_ROUTINE Starter
    )

/*++

Routine Description:

    This routine is used to either start or queue any requst
    that can be queued in the driver.

Arguments:

    Extension - Points to the serial device extension.

    Request - The request to either queue or start.  In either
          case the request will be marked pending.

    QueueToExamine - The queue the request will be place on if there
                     is already an operation in progress.

    CurrentOpRequest - Pointer to a pointer to the request the is current
                   for the queue.  The pointer pointed to will be
                   set with to Request if what CurrentOpRequest points to
                   is NULL.

    Starter - The routine to call if the queue is empty.

Return Value:


--*/

{

    NTSTATUS status;
    PREQUEST_CONTEXT reqContext;
    WDF_REQUEST_PARAMETERS  params;

    reqContext = SerialGetRequestContext(Request);

    WDF_REQUEST_PARAMETERS_INIT(&params);

    WdfRequestGetParameters(
             Request,
             &params);

    //
    // If this is a write request then take the amount of characters
    // to write and add it to the count of characters to write.
    //

    if (params.Type == WdfRequestTypeWrite) {

        Extension->TotalCharsQueued += reqContext->Length;

    } else if ((params.Type == WdfRequestTypeDeviceControl) &&
               ((params.Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_IMMEDIATE_CHAR) ||
                (params.Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_XOFF_COUNTER))) {

        reqContext->IoctlCode = params.Parameters.DeviceIoControl.IoControlCode; // We need this in the destroy callback

        Extension->TotalCharsQueued++;

    }

    if (IsQueueEmpty(QueueToExamine) &&  !(*CurrentOpRequest)) {

        //
        // There were no current operation.  Mark this one as
        // current and start it up.
        //

        *CurrentOpRequest = Request;

        Starter(Extension);

        return;

    } else {

        //
        // We don't know how long the request will be in the
        // queue.  If it gets cancelled while waiting in the queue, we will
        // be notified by EvtCanceledOnQueue callback so that we can readjust
        // the lenght or free the buffer.
        //
        reqContext->Extension = Extension; // We need this in the destroy callback

        status = WdfRequestForwardToIoQueue(Request,  QueueToExamine);
        if(!NT_SUCCESS(status)) {
            SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_READ, "WdfRequestForwardToIoQueue failed%X\n", status);
            ASSERTMSG("WdfRequestForwardToIoQueue failed ", FALSE);
            SerialCompleteRequest(Request, status, 0);
        }

        return;
    }
}

VOID
SerialEvtCanceledOnQueue(
    IN WDFQUEUE   Queue,
    IN WDFREQUEST Request
    )

/*++

Routine Description:

    Called when the request is cancelled while it's waiting
    on the queue. This callback is used instead of EvtCleanupCallback
    on the request because this one will be called with the
    presentation lock held.


Arguments:

    Queue - Queue in which the request currently waiting
    Request - Request being cancelled


Return Value:

    None.

--*/

{
    PSERIAL_DEVICE_EXTENSION extension = NULL;
    PREQUEST_CONTEXT reqContext;

    UNREFERENCED_PARAMETER(Queue);

    reqContext = SerialGetRequestContext(Request);

    extension = reqContext->Extension;

    //
    // If this is a write request then take the amount of characters
    // to write and subtract it from the count of characters to write.
    //

    if (reqContext->MajorFunction == IRP_MJ_WRITE) {

        extension->TotalCharsQueued -= reqContext->Length;

    } else if (reqContext->MajorFunction == IRP_MJ_DEVICE_CONTROL) {

        //
        // If it's an immediate then we need to decrement the
        // count of chars queued.  If it's a resize then we
        // need to deallocate the pool that we're passing on
        // to the "resizing" routine.
        //

        if (( reqContext->IoctlCode == IOCTL_SERIAL_IMMEDIATE_CHAR) ||
            (reqContext->IoctlCode ==  IOCTL_SERIAL_XOFF_COUNTER)) {

            extension->TotalCharsQueued--;

        } else if (reqContext->IoctlCode ==  IOCTL_SERIAL_SET_QUEUE_SIZE) {

            //
            // We shoved the pointer to the memory into the
            // the type 3 buffer pointer which we KNOW we
            // never use.
            //

            ASSERT(reqContext->Type3InputBuffer);

            ExFreePool(reqContext->Type3InputBuffer);

            reqContext->Type3InputBuffer = NULL;

        }

    }

    SerialCompleteRequest(Request, WdfRequestGetStatus(Request), 0);
}


NTSTATUS
SerialCompleteIfError(
    PSERIAL_DEVICE_EXTENSION extension,
    WDFREQUEST Request
    )

/*++

Routine Description:

    If the current request is not an IOCTL_SERIAL_GET_COMMSTATUS request and
    there is an error and the application requested abort on errors,
    then cancel the request.

Arguments:

    extension - Pointer to the device context

    Request - Pointer to the WDFREQUEST to test.

Return Value:

    STATUS_SUCCESS or STATUS_CANCELLED.

--*/

{

    WDF_REQUEST_PARAMETERS  params;
    NTSTATUS status = STATUS_SUCCESS;

    if ((extension->HandFlow.ControlHandShake &
         SERIAL_ERROR_ABORT) && extension->ErrorWord) {

        WDF_REQUEST_PARAMETERS_INIT(&params);

        WdfRequestGetParameters(
            Request,
            &params
            );


        //
        // There is a current error in the driver.  No requests should
        // come through except for the GET_COMMSTATUS.
        //

        if ((params.Type != WdfRequestTypeDeviceControl) ||
                    (params.Parameters.DeviceIoControl.IoControlCode !=  IOCTL_SERIAL_GET_COMMSTATUS)) {
            status = STATUS_CANCELLED;
            SerialCompleteRequest(Request, status, 0);
        }

    }

    return status;

}

NTSTATUS
SerialCreateTimersAndDpcs(
    IN PSERIAL_DEVICE_EXTENSION pDevExt
    )
/*++

Routine Description:

   This function creates all the timers and DPC objects. All the objects
   are associated with the WDFDEVICE and the callbacks are serialized
   with the device callbacks. Also these objects will be deleted automatically
   when the device is deleted, so there is no need for the driver to explicitly
   delete the objects.

Arguments:

   PDevExt - Pointer to the device extension for the device

Return Value:

    return NTSTATUS

--*/
{
   WDF_DPC_CONFIG dpcConfig;
   WDF_TIMER_CONFIG timerConfig;
   NTSTATUS status;
   WDF_OBJECT_ATTRIBUTES dpcAttributes;
   WDF_OBJECT_ATTRIBUTES timerAttributes;

   //
   // Initialize all the timers used to timeout operations.
   //
   //
   // This timer dpc is fired off if the timer for the total timeout
   // for the read expires.  It will cause the current read to complete.
   //

   WDF_TIMER_CONFIG_INIT(&timerConfig, SerialReadTimeout);

   timerConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
   timerAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfTimerCreate(&timerConfig,
                           &timerAttributes,
                                    &pDevExt->ReadRequestTotalTimer);

   if (!NT_SUCCESS(status)) {
      SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfTimerCreate(ReadRequestTotalTimer) failed  [%#08lx]\n",   status);
      return status;
   }

   //
   // This dpc is fired off if the timer for the interval timeout
   // expires.  If no more characters have been read then the
   // dpc routine will cause the read to complete.  However, if
   // more characters have been read then the dpc routine will
   // resubmit the timer.
   //
   WDF_TIMER_CONFIG_INIT(&timerConfig,   SerialIntervalReadTimeout);

   timerConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
   timerAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfTimerCreate(&timerConfig,
                           &timerAttributes,
                                        &pDevExt->ReadRequestIntervalTimer);

   if (!NT_SUCCESS(status)) {
      SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfTimerCreate(ReadRequestIntervalTimer) failed  [%#08lx]\n",   status);
      return status;
   }

   //
   // This dpc is fired off if the timer for the total timeout
   // for the write expires.  It will queue a dpc routine that
   // will cause the current write to complete.
   //
   //

   WDF_TIMER_CONFIG_INIT(&timerConfig,    SerialWriteTimeout);

   timerConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
   timerAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfTimerCreate(&timerConfig,
                                &timerAttributes,
                                &pDevExt->WriteRequestTotalTimer);

   if (!NT_SUCCESS(status)) {
      SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfTimerCreate(WriteRequestTotalTimer) failed  [%#08lx]\n",   status);
      return status;
   }

   //
   // This dpc is fired off if the transmit immediate char
   // character times out.  The dpc routine will "grab" the
   // request from the isr and time it out.
   //
   WDF_TIMER_CONFIG_INIT(&timerConfig,   SerialTimeoutImmediate);

   timerConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
   timerAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfTimerCreate(&timerConfig,
                           &timerAttributes,
                                        &pDevExt->ImmediateTotalTimer);

   if (!NT_SUCCESS(status)) {
      SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfTimerCreate(ImmediateTotalTimer) failed  [%#08lx]\n",   status);
      return status;
   }

   //
   // This dpc is fired off if the timer used to "timeout" counting
   // the number of characters received after the Xoff ioctl is started
   // expired.
   //

   WDF_TIMER_CONFIG_INIT(&timerConfig,   SerialTimeoutXoff);

   timerConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
   timerAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfTimerCreate(&timerConfig,
                                    &timerAttributes,
                                    &pDevExt->XoffCountTimer);

    if (!NT_SUCCESS(status)) {
      SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfTimerCreate(XoffCountTimer) failed  [%#08lx]\n",   status);
      return status;
   }

   //
   // This dpc is fired off when a timer expires (after one
   // character time), so that code can be invoked that will
   // check to see if we should lower the RTS line when
   // doing transmit toggling.
   //
   WDF_TIMER_CONFIG_INIT(&timerConfig,  SerialInvokePerhapsLowerRTS);

   timerConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
   timerAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfTimerCreate(&timerConfig,
                           &timerAttributes,
                                    &pDevExt->LowerRTSTimer);
    if (!NT_SUCCESS(status)) {
        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfTimerCreate(LowerRTSTimer) failed  [%#08lx]\n",   status);
        return status;
    }

    //
    // Create a DPC to complete read requests.
    //

   WDF_DPC_CONFIG_INIT(&dpcConfig, SerialCompleteWrite);

   dpcConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
   dpcAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfDpcCreate(&dpcConfig,
                                    &dpcAttributes,
                                    &pDevExt->CompleteWriteDpc);
    if (!NT_SUCCESS(status)) {

        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(CompleteWriteDpc) failed  [%#08lx]\n",   status);
        return status;
    }


    //
    // Create a DPC to complete read requests.
    //

    WDF_DPC_CONFIG_INIT(&dpcConfig, SerialCompleteRead);

    dpcConfig.AutomaticSerialization = TRUE;

    WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
    dpcAttributes.ParentObject = pDevExt->WdfDevice;

    status = WdfDpcCreate(&dpcConfig,
                                &dpcAttributes,
                                &pDevExt->CompleteReadDpc);

    if (!NT_SUCCESS(status)) {
        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(CompleteReadDpc) failed  [%#08lx]\n",   status);
        return status;
    }

    //
    // This dpc is fired off if a comm error occurs.  It will
    // cancel all pending reads and writes.
    //
    WDF_DPC_CONFIG_INIT(&dpcConfig, SerialCommError);

    dpcConfig.AutomaticSerialization = TRUE;

    WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
    dpcAttributes.ParentObject = pDevExt->WdfDevice;

    status = WdfDpcCreate(&dpcConfig,
                                &dpcAttributes,
                                &pDevExt->CommErrorDpc);


    if (!NT_SUCCESS(status)) {

        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(CommErrorDpc) failed  [%#08lx]\n",   status);
        return status;
    }

    //
    // This dpc is fired off when the transmit immediate char
    // character is given to the hardware.  It will simply complete
    // the request.
    //

   WDF_DPC_CONFIG_INIT(&dpcConfig, SerialCompleteImmediate);

   dpcConfig.AutomaticSerialization = TRUE;

   WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
   dpcAttributes.ParentObject = pDevExt->WdfDevice;

   status = WdfDpcCreate(&dpcConfig,
                                    &dpcAttributes,
                                    &pDevExt->CompleteImmediateDpc);
    if (!NT_SUCCESS(status)) {
        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(CompleteImmediateDpc) failed  [%#08lx]\n",   status);
        return status;
    }

    //
    // This dpc is fired off if an event occurs and there was
    // a request waiting on that event.  A dpc routine will execute
    // that completes the request.
    //
    WDF_DPC_CONFIG_INIT(&dpcConfig, SerialCompleteWait);

    dpcConfig.AutomaticSerialization = TRUE;

    WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
    dpcAttributes.ParentObject = pDevExt->WdfDevice;

    status = WdfDpcCreate(&dpcConfig,
                                &dpcAttributes,
                                &pDevExt->CommWaitDpc);
    if (!NT_SUCCESS(status)) {

        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(CommWaitDpc) failed  [%#08lx]\n",   status);
        return status;
    }

    //
    // This dpc is fired off if the xoff counter actually runs down
    // to zero.
    //
    WDF_DPC_CONFIG_INIT(&dpcConfig, SerialCompleteXoff);

    dpcConfig.AutomaticSerialization = TRUE;

    WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
    dpcAttributes.ParentObject = pDevExt->WdfDevice;

    status = WdfDpcCreate(&dpcConfig,
                                &dpcAttributes,
                                &pDevExt->XoffCountCompleteDpc);

    if (!NT_SUCCESS(status)) {
        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(XoffCountCompleteDpc) failed  [%#08lx]\n",   status);
        return status;
    }


    //
    // This dpc is fired off only from device level to start off
    // a timer that will queue a dpc to check if the RTS line
    // should be lowered when we are doing transmit toggling.
    //
    WDF_DPC_CONFIG_INIT(&dpcConfig, SerialStartTimerLowerRTS);

    dpcConfig.AutomaticSerialization = TRUE;

    WDF_OBJECT_ATTRIBUTES_INIT(&dpcAttributes);
    dpcAttributes.ParentObject = pDevExt->WdfDevice;

    status = WdfDpcCreate(&dpcConfig,
                                &dpcAttributes,
                                &pDevExt->StartTimerLowerRTSDpc);
    if (!NT_SUCCESS(status)) {
        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,  "WdfDpcCreate(StartTimerLowerRTSDpc) failed  [%#08lx]\n",   status);
        return status;
    }

    return status;
}




BOOLEAN
SerialInsertQueueDpc(IN WDFDPC PDpc)
/*++

Routine Description:

   This function must be called to queue DPC's for the serial driver.

Arguments:

   PDpc - Pointer to the Dpc object

Return Value:

   Kicks up return value from KeInsertQueueDpc()

--*/
{
    //
    // If the specified DPC object is not currently in the queue, WdfDpcEnqueue
    // queues the DPC and returns TRUE.
    //

    return WdfDpcEnqueue(PDpc);
}



BOOLEAN
SerialSetTimer(IN WDFTIMER Timer, IN LARGE_INTEGER DueTime)
/*++

Routine Description:

   This function must be called to set timers for the serial driver.

Arguments:

   Timer - pointer to timer dispatcher object

   DueTime - time at which the timer should expire


Return Value:

   Kicks up return value from KeSetTimerEx()

--*/
{
    BOOLEAN result;
    //
    // If the timer object was already in the system timer queue, WdfTimerStart returns TRUE
    //
    result = WdfTimerStart(Timer, DueTime.QuadPart);

    return result;

}


VOID
SerialDrainTimersAndDpcs(
    IN PSERIAL_DEVICE_EXTENSION PDevExt
    )
/*++

Routine Description:

   This function cancels all the timers and Dpcs and waits for them
   to run to completion if they are already fired.

Arguments:

   PDevExt - Pointer to the device extension for the device that needs to
             set a timer

Return Value:

--*/
{
    WdfTimerStop(PDevExt->ReadRequestTotalTimer, TRUE);

    WdfTimerStop(PDevExt->ReadRequestIntervalTimer, TRUE);

    WdfTimerStop(PDevExt->WriteRequestTotalTimer, TRUE);

    WdfTimerStop(PDevExt->ImmediateTotalTimer, TRUE);

    WdfTimerStop(PDevExt->XoffCountTimer, TRUE);

    WdfTimerStop(PDevExt->LowerRTSTimer, TRUE);

    WdfDpcCancel(PDevExt->CompleteWriteDpc, TRUE);

    WdfDpcCancel(PDevExt->CompleteReadDpc, TRUE);

    WdfDpcCancel(PDevExt->CommErrorDpc, TRUE);

    WdfDpcCancel(PDevExt->CompleteImmediateDpc, TRUE);

    WdfDpcCancel(PDevExt->CommWaitDpc, TRUE);

    WdfDpcCancel(PDevExt->XoffCountCompleteDpc, TRUE);

    WdfDpcCancel(PDevExt->StartTimerLowerRTSDpc, TRUE);

    return;
}



BOOLEAN
SerialCancelTimer(
    IN WDFTIMER                 Timer,
    IN PSERIAL_DEVICE_EXTENSION PDevExt
    )
/*++

Routine Description:

   This function must be called to cancel timers for the serial driver.

Arguments:

   Timer - pointer to timer dispatcher object

   PDevExt - Pointer to the device extension for the device that needs to
             set a timer

Return Value:

   True if timer was cancelled

--*/
{
    UNREFERENCED_PARAMETER(PDevExt);

    return WdfTimerStop(Timer, FALSE);
}

SERIAL_MEM_COMPARES
SerialMemCompare(
                IN PHYSICAL_ADDRESS A,
                IN ULONG SpanOfA,
                IN PHYSICAL_ADDRESS B,
                IN ULONG SpanOfB
                )
/*++

Routine Description:

    Compare two phsical address.

Arguments:

    A - One half of the comparison.

    SpanOfA - In units of bytes, the span of A.

    B - One half of the comparison.

    SpanOfB - In units of bytes, the span of B.


Return Value:

    The result of the comparison.

--*/
{
   LARGE_INTEGER a;
   LARGE_INTEGER b;

   LARGE_INTEGER lower;
   ULONG lowerSpan;
   LARGE_INTEGER higher;

   PAGED_CODE();

   a = A;
   b = B;

   if (a.QuadPart == b.QuadPart) {

      return AddressesAreEqual;

   }

   if (a.QuadPart > b.QuadPart) {

      higher = a;
      lower = b;
      lowerSpan = SpanOfB;

   } else {

      higher = b;
      lower = a;
      lowerSpan = SpanOfA;

   }

   if ((higher.QuadPart - lower.QuadPart) >= lowerSpan) {

      return AddressesAreDisjoint;

   }

   return AddressesOverlap;

}


VOID
SerialLogError(
    __in                             PDRIVER_OBJECT DriverObject,
    __in_opt                         PDEVICE_OBJECT DeviceObject,
    __in                             PHYSICAL_ADDRESS P1,
    __in                             PHYSICAL_ADDRESS P2,
    __in                             ULONG SequenceNumber,
    __in                             UCHAR MajorFunctionCode,
    __in                             UCHAR RetryCount,
    __in                             ULONG UniqueErrorValue,
    __in                             NTSTATUS FinalStatus,
    __in                             NTSTATUS SpecificIOStatus,
    __in                             ULONG LengthOfInsert1,
    __in_bcount_opt(LengthOfInsert1) PWCHAR Insert1,
    __in                             ULONG LengthOfInsert2,
    __in_bcount_opt(LengthOfInsert2) PWCHAR Insert2
    )
/*++

Routine Description:

    This routine allocates an error log entry, copies the supplied data
    to it, and requests that it be written to the error log file.

Arguments:

    DriverObject - A pointer to the driver object for the device.

    DeviceObject - A pointer to the device object associated with the
    device that had the error, early in initialization, one may not
    yet exist.

    P1,P2 - If phyical addresses for the controller ports involved
    with the error are available, put them through as dump data.

    SequenceNumber - A ulong value that is unique to an WDFREQUEST over the
    life of the request in this driver - 0 generally means an error not
    associated with an request.

    MajorFunctionCode - If there is an error associated with the request,
    this is the major function code of that request.

    RetryCount - The number of times a particular operation has been
    retried.

    UniqueErrorValue - A unique long word that identifies the particular
    call to this function.

    FinalStatus - The final status given to the request that was associated
    with this error.  If this log entry is being made during one of
    the retries this value will be STATUS_SUCCESS.

    SpecificIOStatus - The IO status for a particular error.

    LengthOfInsert1 - The length in bytes (including the terminating NULL)
                      of the first insertion string.

    Insert1 - The first insertion string.

    LengthOfInsert2 - The length in bytes (including the terminating NULL)
                      of the second insertion string.  NOTE, there must
                      be a first insertion string for their to be
                      a second insertion string.

    Insert2 - The second insertion string.

Return Value:

    None.

--*/

{
   PIO_ERROR_LOG_PACKET errorLogEntry;

   PVOID objectToUse;
   SHORT dumpToAllocate = 0;
   PUCHAR ptrToFirstInsert;
   PUCHAR ptrToSecondInsert;

   PAGED_CODE();

   if (Insert1 == NULL) {
      LengthOfInsert1 = 0;
   }

   if (Insert2 == NULL) {
      LengthOfInsert2 = 0;
   }


   if (ARGUMENT_PRESENT(DeviceObject)) {

      objectToUse = DeviceObject;

   } else {

      objectToUse = DriverObject;

   }

   if (SerialMemCompare(
                       P1,
                       (ULONG)1,
                       SerialPhysicalZero,
                       (ULONG)1
                       ) != AddressesAreEqual) {

      dumpToAllocate = (SHORT)sizeof(PHYSICAL_ADDRESS);

   }

   if (SerialMemCompare(
                       P2,
                       (ULONG)1,
                       SerialPhysicalZero,
                       (ULONG)1
                       ) != AddressesAreEqual) {

      dumpToAllocate += (SHORT)sizeof(PHYSICAL_ADDRESS);

   }

   errorLogEntry = IoAllocateErrorLogEntry(
                                          objectToUse,
                                          (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                                  dumpToAllocate
                                                  + LengthOfInsert1 +
                                                  LengthOfInsert2)
                                          );

   if ( errorLogEntry != NULL ) {

      errorLogEntry->ErrorCode = SpecificIOStatus;
      errorLogEntry->SequenceNumber = SequenceNumber;
      errorLogEntry->MajorFunctionCode = MajorFunctionCode;
      errorLogEntry->RetryCount = RetryCount;
      errorLogEntry->UniqueErrorValue = UniqueErrorValue;
      errorLogEntry->FinalStatus = FinalStatus;
      errorLogEntry->DumpDataSize = dumpToAllocate;

      if (dumpToAllocate) {

         RtlCopyMemory(
                      &errorLogEntry->DumpData[0],
                      &P1,
                      sizeof(PHYSICAL_ADDRESS)
                      );

         if (dumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {

            RtlCopyMemory(
                         ((PUCHAR)&errorLogEntry->DumpData[0])
                         +sizeof(PHYSICAL_ADDRESS),
                         &P2,
                         sizeof(PHYSICAL_ADDRESS)
                         );

            ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+(2*sizeof(PHYSICAL_ADDRESS));

         } else {

            ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+sizeof(PHYSICAL_ADDRESS);


         }

      } else {

         ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

      }

      ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

      if (LengthOfInsert1) {

         errorLogEntry->NumberOfStrings = 1;
         errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                (PUCHAR)errorLogEntry);
         RtlCopyMemory(
                      ptrToFirstInsert,
                      Insert1,
                      LengthOfInsert1
                      );

         if (LengthOfInsert2) {

            errorLogEntry->NumberOfStrings = 2;
            RtlCopyMemory(
                         ptrToSecondInsert,
                         Insert2,
                         LengthOfInsert2
                         );

         }

      }

      IoWriteErrorLogEntry(errorLogEntry);

   }

}

VOID
SerialMarkHardwareBroken(IN PSERIAL_DEVICE_EXTENSION PDevExt)
/*++

Routine Description:

   Marks a UART as broken.  This causes the driver stack to stop accepting
   requests and eventually be removed.

Arguments:
   PDevExt - Device extension attached to PDevObj

Return Value:

   None.

--*/
{
   PAGED_CODE();

   //
   // Write a log entry
   //

   SerialLogError(PDevExt->DriverObject, NULL, SerialPhysicalZero,
                  SerialPhysicalZero, 0, 0, 0, 88, STATUS_SUCCESS,
                  SERIAL_HARDWARE_FAILURE, PDevExt->DeviceName.Length
                  + sizeof(WCHAR), PDevExt->DeviceName.Buffer, 0, NULL);

   SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_INIT, "Device is broken. Request a restart...\n");
   WdfDeviceSetFailed(PDevExt->WdfDevice, WdfDeviceFailedAttemptRestart);
}

NTSTATUS
SerialGetDivisorFromBaud(
                        IN PSERIAL_DEVICE_EXTENSION Extension,
                        IN ULONG ClockRate,
                        IN ULONG SampleRate,
                        IN LONG DesiredBaud,
                        OUT PSHORT AppropriateDivisor
                        )

/*++

Routine Description:

    This routine will determine a divisor based on an unvalidated
    baud rate.

Arguments:

    ClockRate - The clock input to the controller.

    DesiredBaud - The baud rate for whose divisor we seek.

    AppropriateDivisor - Given that the DesiredBaud is valid, the
    LONG pointed to by this parameter will be set to the appropriate
    value.  NOTE: The long is undefined if the DesiredBaud is not
    supported.

Return Value:

    This function will return STATUS_SUCCESS if the baud is supported.
    If the value is not supported it will return a status such that
    NT_ERROR(Status) == FALSE.

--*/

{

   NTSTATUS status = STATUS_SUCCESS;
   SHORT calculatedDivisor;
   ULONG denominator;
   ULONG remainder;

   //
   // Allow up to a 1 percent error
   //

   ULONG maxRemain18 = 18432;
   ULONG maxRemain30 = 30720;
   ULONG maxRemain42 = 42336;
   ULONG maxRemain80 = 80000;
   ULONG maxRemain;

   //
   // Reject any non-positive bauds.
   //

   if (FastcomGetCardType(Extension) == CARD_TYPE_PCIe)
      return 1;

   denominator = DesiredBaud*SampleRate;

   if (DesiredBaud <= 0) {

      *AppropriateDivisor = -1;

   } else if ((LONG)denominator < DesiredBaud) {

      //
      // If the desired baud was so huge that it cause the denominator
      // calculation to wrap, don't support it.
      //

      *AppropriateDivisor = -1;

   } else {

      if (ClockRate == 1843200) {
         maxRemain = maxRemain18;
      } else if (ClockRate == 3072000) {
         maxRemain = maxRemain30;
      } else if (ClockRate == 4233600) {
         maxRemain = maxRemain42;
      } else {
         maxRemain = maxRemain80;
      }

      calculatedDivisor = (SHORT)(ClockRate / denominator);
      remainder = ClockRate % denominator;

      //
      // Round up.
      //

      if (((remainder*2) > ClockRate) && (DesiredBaud != 110)) {

         calculatedDivisor++;
      }


      //
      // Only let the remainder calculations effect us if
      // the baud rate is > 9600.
      //

      if (DesiredBaud >= 9600) {

         //
         // If the remainder is less than the maximum remainder (wrt
         // the ClockRate) or the remainder + the maximum remainder is
         // greater than or equal to the ClockRate then assume that the
         // baud is ok.
         //

         if ((remainder >= maxRemain) && ((remainder+maxRemain) < ClockRate)) {
            calculatedDivisor = -1;
         }

      }

      //
      // Don't support a baud that causes the denominator to
      // be larger than the clock.
      //

      if (denominator > ClockRate) {

         calculatedDivisor = -1;

      }

      //
      // Ok, Now do some special casing so that things can actually continue
      // working on all platforms.
      //

      if (ClockRate == 1843200) {

         if (DesiredBaud == 56000) {
            calculatedDivisor = 2;
         }

      } else if (ClockRate == 3072000) {

         if (DesiredBaud == 14400) {
            calculatedDivisor = 13;
         }

      } else if (ClockRate == 4233600) {

         if (DesiredBaud == 9600) {
            calculatedDivisor = 28;
         } else if (DesiredBaud == 14400) {
            calculatedDivisor = 18;
         } else if (DesiredBaud == 19200) {
            calculatedDivisor = 14;
         } else if (DesiredBaud == 38400) {
            calculatedDivisor = 7;
         } else if (DesiredBaud == 56000) {
            calculatedDivisor = 5;
         }

      } else if (ClockRate == 8000000) {

         if (DesiredBaud == 14400) {
            calculatedDivisor = 35;
         } else if (DesiredBaud == 56000) {
            calculatedDivisor = 9;
         }

      }

      *AppropriateDivisor = calculatedDivisor;

   }


   if (*AppropriateDivisor == -1) {

      status = STATUS_INVALID_PARAMETER;

   }

   return status;

}


BOOLEAN
IsQueueEmpty(
    IN WDFQUEUE Queue
    )
{
    WDF_IO_QUEUE_STATE queueStatus;

    queueStatus = WdfIoQueueGetState( Queue, NULL, NULL );

    return (WDF_IO_QUEUE_IDLE(queueStatus)) ? TRUE : FALSE;
}

VOID
SerialSetCancelRoutine(
    IN WDFREQUEST Request,
    IN PFN_WDF_REQUEST_CANCEL CancelRoutine)
{
    PREQUEST_CONTEXT reqContext = SerialGetRequestContext(Request);

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_IOCTLS,
                        "-->SerialSetCancelRoutine %p \n",  Request);

    WdfRequestMarkCancelable(Request, CancelRoutine);
    SERIAL_SET_REFERENCE(reqContext, SERIAL_REF_CANCEL);
    reqContext->CancelRoutine = CancelRoutine;

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_IOCTLS,
                        "<-- SerialSetCancelRoutine \n");

    return;
}

NTSTATUS
SerialClearCancelRoutine(
    IN WDFREQUEST Request,
    IN BOOLEAN    ClearReference
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PREQUEST_CONTEXT reqContext = SerialGetRequestContext(Request);

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_IOCTLS,
                     "-->SerialClearCancelRoutine %p %x\n",
                     Request, ClearReference);

    if(SERIAL_TEST_REFERENCE(reqContext,  SERIAL_REF_CANCEL))
    {
        status = WdfRequestUnmarkCancelable(Request);
        if (NT_SUCCESS(status)) {

            reqContext->CancelRoutine = NULL;
            if(ClearReference) {

               SERIAL_CLEAR_REFERENCE( reqContext,  SERIAL_REF_CANCEL );

              }
        } else {
             ASSERT(status == STATUS_CANCELLED);
        }
    }

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_IOCTLS,
                        "-->SerialClearCancelRoutine %p\n",  Request);

    return status;
}


VOID
SerialCompleteRequest(
    IN WDFREQUEST   Request,
    IN NTSTATUS     Status,
    IN ULONG_PTR    Info
    )
{
    PREQUEST_CONTEXT reqContext;

    reqContext = SerialGetRequestContext(Request);

    ASSERT(reqContext->RefCount == 0);

    SerialDbgPrintEx(TRACE_LEVEL_VERBOSE, DBG_PNP,
                     "Complete Request: %p %X 0x%I64x\n",
                     (Request), (Status), (Info));

    WdfRequestCompleteWithInformation((Request), (Status), (Info));

}


/*****************************************************************************
 * Direct R/W from config space.
 *****************************************************************************/
INT
PCIReadConfigWord(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          Offset,
    IN PVOID          Value
    )
{
    PDEVICE_OBJECT TargetObject;
    PIRP pIrp;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION IrpStack;
    KEVENT ConfigReadWordEvent;
    INT error = 0;

    TargetObject = IoGetAttachedDeviceReference(DeviceObject);
    KeInitializeEvent(&ConfigReadWordEvent, NotificationEvent, FALSE);
    
    pIrp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP, TargetObject, NULL,
        0, NULL, &ConfigReadWordEvent, &IoStatusBlock );

    if (pIrp) {
        /* Create the config space read IRP */
        IrpStack = IoGetNextIrpStackLocation(pIrp);
        IrpStack->MinorFunction = IRP_MN_READ_CONFIG;
        IrpStack->Parameters.ReadWriteConfig.WhichSpace = \
            PCI_WHICHSPACE_CONFIG;
        IrpStack->Parameters.ReadWriteConfig.Offset = Offset;
        IrpStack->Parameters.ReadWriteConfig.Length = 0x2;
        IrpStack->Parameters.ReadWriteConfig.Buffer = Value;
        pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
     
        /* Send the IRP */
        if (IoCallDriver(TargetObject, pIrp)==STATUS_PENDING) {
            KeWaitForSingleObject(&ConfigReadWordEvent, Executive, \
                KernelMode, FALSE, NULL);
        }
    } else {
        error = -1;
    }

    ObDereferenceObject(TargetObject);
    return error;
}

enum FASTCOM_CARD_TYPE FastcomGetCardType(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    switch (pDevExt->DeviceID) {
    case FC_422_2_PCI_335_ID:
    case FC_422_4_PCI_335_ID:
    case FC_232_4_PCI_335_ID:
    case FC_232_8_PCI_335_ID:
        return CARD_TYPE_PCI;

    case FC_422_4_PCIe_ID:
    case FC_422_8_PCIe_ID:
        return CARD_TYPE_PCIe;
    }

    if (pDevExt->DeviceID == 0x0f ||
        (pDevExt->DeviceID >= 0x14 && pDevExt->DeviceID <= 0x1F) ||
        (pDevExt->DeviceID >= 0x22 && pDevExt->DeviceID <= 0x27)
       ) {
        return CARD_TYPE_FSCC;
    }

    return CARD_TYPE_UNKNOWN;
}

NTSTATUS FastcomSetSampleRatePCI(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    UCHAR current_8x_mode, new_8x_mode;

    if (value != 8 && value != 16)
        return STATUS_INVALID_PARAMETER;

    current_8x_mode = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_8XMODE);

    switch (value) {
    case 8:
        new_8x_mode = current_8x_mode | (1 << pDevExt->Channel);
        break;

    case 16:
        new_8x_mode = current_8x_mode & ~(1 << pDevExt->Channel);
        break;
    }
    
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_8XMODE, new_8x_mode);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetSampleRatePCIe(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    UCHAR current_8x_mode, new_8x_mode;
    UCHAR current_4x_mode, new_4x_mode;

    if (value != 4 && value != 8 && value != 16)
        return STATUS_INVALID_PARAMETER;

    current_4x_mode = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_4XMODE);
    current_8x_mode = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_8XMODE);

    switch (value) {
    case 4:
        new_4x_mode = current_4x_mode | (1 << pDevExt->Channel);
        new_8x_mode = current_8x_mode & ~(1 << pDevExt->Channel);
        break;

    case 8:
        new_4x_mode = current_4x_mode & ~(1 << pDevExt->Channel);
        new_8x_mode = current_8x_mode | (1 << pDevExt->Channel); 
        break;

    case 16:
        new_4x_mode = current_4x_mode & ~(1 << pDevExt->Channel);
        new_8x_mode = current_8x_mode & ~(1 << pDevExt->Channel);
        break;
    }
    
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_4XMODE, new_4x_mode); 
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_8XMODE, new_8x_mode);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetSampleRateFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    UCHAR orig_lcr;

    if (value < 4 || value > 16)
        return STATUS_INVALID_PARAMETER;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, TCR_OFFSET); /* To allow access to TCR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, (UCHAR)value); /* Actually writing to TCR through ICR */

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetSampleRate(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        status = FastcomSetSampleRatePCI(pDevExt, value);
        break;

    case CARD_TYPE_PCIe:
        status = FastcomSetSampleRatePCIe(pDevExt, value);
        break;

    case CARD_TYPE_FSCC:
        status = FastcomSetSampleRateFSCC(pDevExt, value);
        break;

	default:
		/* This makes sure the sample rate value is set for the non Fastcom ports. */
		status = STATUS_SUCCESS;
		break;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Sample rate = %i\n", value);

        pDevExt->SampleRate = value;
    }

    return status;
}

void FastcomGetSampleRate(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *value)
{
    *value = pDevExt->SampleRate;
}

NTSTATUS FastcomSetTxTriggerPCI(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    if (value > 64)
        return STATUS_INVALID_PARAMETER;

    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_TXTRG, (UCHAR)value);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetTxTriggerPCIe(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    if (value > 255)
        return STATUS_INVALID_PARAMETER;
    
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_TXTRG, (UCHAR)value);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetTxTriggerFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    UCHAR orig_lcr;

    if (value > 127)
        return STATUS_INVALID_PARAMETER;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, TTL_OFFSET); /* To allow access to TTL */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, (UCHAR)value); /* Set the trigger level to TTL through ICR */

    /* TODO: When 950 trigger levels are fixed, use the TTLH value also */

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetTxTrigger(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    NTSTATUS status;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        status = FastcomSetTxTriggerPCI(pDevExt, value);
        break;

    case CARD_TYPE_PCIe:
        status = FastcomSetTxTriggerPCIe(pDevExt, value);
        break;

    case CARD_TYPE_FSCC:
        status = FastcomSetTxTriggerFSCC(pDevExt, value);
        break;


    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Transmit trigger level = %i\n", value); 

        pDevExt->TxTrigger = value;
    }

    return status;
}

void FastcomGetTxTriggerFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *value)
{
    UCHAR orig_lcr;
    UCHAR ttl;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x40); /* Enable ICR read enable */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, TTL_OFFSET); /* To allow access to TTL */

    ttl = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get TTL through ICR */

    *value = ttl & 0x7F;

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

NTSTATUS FastcomGetTxTrigger(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *value)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
    case CARD_TYPE_PCIe:
        *value = pDevExt->TxTrigger; // The register is write-only so we have to use this variable
        break;

    case CARD_TYPE_FSCC:
        FastcomGetTxTriggerFSCC(pDevExt, value);
        break;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetRxTriggerPCI(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    if (value > 64)
        return STATUS_INVALID_PARAMETER;

    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_RXTRG, (UCHAR)value);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetRxTriggerPCIe(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    if (value > 255)
        return STATUS_INVALID_PARAMETER;
    
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_RXTRG, (UCHAR)value);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetRxTriggerFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    UCHAR orig_lcr;

    if (value > 127)
        return STATUS_INVALID_PARAMETER;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, RTL_OFFSET); /* To allow access to RTL */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, (UCHAR)value); /* Set the trigger level to RTL through ICR */

    /* TODO: When 950 trigger levels are fixed, use the RTLH value also */

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

void FastcomGetRxTriggerFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *value)
{
    UCHAR orig_lcr;
    UCHAR rtl;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x40); /* Enable ICR read enable */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, RTL_OFFSET); /* To allow access to RTL */

    rtl = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get RTL through ICR */

    *value = rtl & 0x7F;

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

NTSTATUS FastcomSetRxTrigger(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    NTSTATUS status;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        status = FastcomSetRxTriggerPCI(pDevExt, value);
        break;

    case CARD_TYPE_PCIe:
        status = FastcomSetRxTriggerPCIe(pDevExt, value);
        break;

    case CARD_TYPE_FSCC:
        status = FastcomSetRxTriggerFSCC(pDevExt, value);
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Receive trigger level = %i\n", value); 

        pDevExt->RxTrigger = value;
    }

    return status;
}

NTSTATUS FastcomGetRxTrigger(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *value)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
    case CARD_TYPE_PCIe:
        *value = pDevExt->RxTrigger; // The register is write-only so we have to use this variable
        break;

    case CARD_TYPE_FSCC:
        FastcomGetRxTriggerFSCC(pDevExt, value);
        break;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS FastcomGetTxFifoSpace(SERIAL_DEVICE_EXTENSION *pDevExt, ULONG *value)
{
    UCHAR fill_level;
    int room_left = 0;
    
    // We want to make sure 
    *value = 0;
    switch (FastcomGetCardType(pDevExt)) {
        case CARD_TYPE_PCI:
            fill_level = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_TXTRG);
            room_left = PCI_FIFO_SIZE - fill_level;
            break;
        case CARD_TYPE_PCIe:
            fill_level = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_TXTRG);
            room_left = PCIE_FIFO_SIZE - fill_level;
            break;
        case CARD_TYPE_FSCC:
            FastcomGetTxFifoFillFSCC(pDevExt, &fill_level);
            room_left = FSCC_FIFO_SIZE - fill_level;
            break;
        default:
            return STATUS_NOT_SUPPORTED;
    }
    if(room_left > 0) *value = room_left;
    
    return STATUS_SUCCESS;
}

void FastcomGetTxFifoFillFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, UCHAR *value)
{
    UCHAR orig_lcr;
    
    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x80); /* Enable TFL read enable */
    *value = pDevExt->SerialReadUChar(pDevExt->Controller + 0x4);
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

void FastcomSetRS485PCI(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR current_mcr, new_mcr;
    UCHAR current_fctr, new_fctr;
    UCHAR current_mpio_lvl, new_mpio_lvl;
    
    current_mcr = READ_MODEM_CONTROL(pDevExt, pDevExt->Controller);
    current_fctr = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_FCTR);
    current_mpio_lvl = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVL_OFFSET);

    if (enable) {
        new_mcr = current_mcr | 0x3;  /* Force RTS/DTS to low (not sure why yet) */
        new_fctr = current_fctr | 0x20; /* Enable Auto 485 on UART */
        new_mpio_lvl = current_mpio_lvl & ~(0x8 << pDevExt->Channel); /* Enable 485 on transmitters */
    }
    else {
        new_mcr = current_mcr & ~0x3;  /* Force RTS/DTS to high (not sure why yet) */
        new_fctr = current_fctr & ~0x20; /* Disable Auto 485 on UART */
        new_mpio_lvl = current_mpio_lvl | (0x8 << pDevExt->Channel); /* Disable 485 on transmitters */
    }

    WRITE_MODEM_CONTROL(pDevExt, pDevExt->Controller, new_mcr);
    pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOLVL_OFFSET, new_mpio_lvl);
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_FCTR, new_fctr);
}

void FastcomSetRS485PCIe(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR current_mcr, new_mcr;
    UCHAR current_fctr, new_fctr;
    
    current_mcr = READ_MODEM_CONTROL(pDevExt, pDevExt->Controller);
    current_fctr = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_FCTR);

    if (enable) {
        new_mcr = current_mcr | 0x04;  /* Use DTR for Auto 485 */
        new_fctr = current_fctr | 0x20; /* Enable Auto 485 on UART */
    }
    else {
        new_mcr = current_mcr & ~0x04;  /* Disable using DTR for Auto 485 */
        new_fctr = current_fctr & ~0x20; /* Disable Auto 485 on UART */
    }

    WRITE_MODEM_CONTROL(pDevExt, pDevExt->Controller, new_mcr);
    pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_FCTR, new_fctr);
}

void FastcomSetRS485FSCC(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR orig_lcr;
    UINT32 current_fcr, new_fcr;
    UINT32 bit_mask;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);
    current_fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */

    switch (pDevExt->Channel) {
    case 0:
        bit_mask = 0x00040000;
        break;

    case 1:
        bit_mask = 0x00400000;
        break;
    }

    if (enable) {
        pDevExt->ACR |= 0x10; /* DTR is active during transmission to turn on drivers */
        new_fcr = current_fcr | bit_mask;
    }
    else {
        pDevExt->ACR &= ~0x10;
        new_fcr = current_fcr & ~bit_mask;
    }

    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR);
    WRITE_PORT_ULONG(ULongToPtr(pDevExt->Bar2), new_fcr);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

void FastcomSetRS485(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        FastcomSetRS485PCI(pDevExt, enable);
        break;

    case CARD_TYPE_PCIe:
        FastcomSetRS485PCIe(pDevExt, enable);
        break;

    case CARD_TYPE_FSCC:
        FastcomSetRS485FSCC(pDevExt, enable);
        break;

    default:
        break; // TODO
    }

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                     "RS485 = %i\n", enable);

    pDevExt->RS485 = enable;
}

void FastcomGetRS485PCI(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UCHAR current_fctr;

    current_fctr = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_FCTR);

    *enabled = (current_fctr & 0x20) ? TRUE : FALSE;
}

void FastcomGetRS485FSCC(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UINT32 current_fcr;
    BOOLEAN dtr_enable_active, transmitter_485_active;
    UINT32 bit_mask;

    switch (pDevExt->Channel) {
    case 0:
        bit_mask = 0x00040000;
        break;

    case 1:
        bit_mask = 0x00400000;
        break;
    }

    current_fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));
    dtr_enable_active = (pDevExt->ACR & 0x10) ? TRUE : FALSE; /* DTR is active during transmission to turn on drivers */
    transmitter_485_active = (current_fcr & bit_mask) ? TRUE : FALSE;

    *enabled = (dtr_enable_active && transmitter_485_active) ? TRUE : FALSE;
}

NTSTATUS FastcomGetRS485(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
    case CARD_TYPE_PCIe:
        FastcomGetRS485PCI(pDevExt, enabled); // Same process for the PCIe card
        return STATUS_SUCCESS;

    case CARD_TYPE_FSCC:
        FastcomGetRS485FSCC(pDevExt, enabled);
        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_UNSUCCESSFUL;
}

void FastcomEnableRS485(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    FastcomSetRS485(pDevExt, TRUE);
}

void FastcomDisableRS485(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    FastcomSetRS485(pDevExt, FALSE);
}

NTSTATUS FastcomSetIsochronousFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, int mode)
{
    UCHAR orig_lcr;
    UCHAR new_cks = 0;
    UCHAR new_mdm = 0;

    if (mode > 10 || mode < -1)
        return STATUS_INVALID_PARAMETER;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */

    switch (mode) {
    /* Enable receive using external DSR# */
    case 2:
    case 3:
    case 4:
    case 10:
        new_cks |= 0x09;
        new_mdm |= 0x02;
        break;

    /* Enable receive using internal BRG */
    case 5:
    case 6:
    case 7:
        new_cks |= 0x0A;
        break;

    /* Enable receive using transmit clock */
    case 8:
        new_cks |= 0x0B;
        break;
    }

    switch (mode) {
    /* Enable transmit using external RI# */
    case 0:
    case 3:
    case 6:
    case 8:
        new_cks |= 0xD0;
        new_mdm |= 0x04;
        break;

    /* Transmit using internal BRG */
    case 1:
    case 4:
    case 7:
        new_cks |= 0x90;
        break;

    case 9:
    case 10:
        new_cks |= 0x10;
        break;
    }

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, MDM_OFFSET); /* To allow access to MDM */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, new_mdm); /* Set interrupts to MDM through ICR */

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, CKS_OFFSET); /* To allow access to CKS */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, new_cks); /* Set clock mode to CKS through ICR */

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

void FastcomGetIsochronousFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, int *mode)
{
    UCHAR orig_lcr;
    UCHAR cks;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x40); /* Enable ICR read enable */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, CKS_OFFSET); /* To allow access to CLK */

    cks = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get CKS through ICR */

    switch (cks) {
    case 0x00:
        *mode = -1;
        break;

    case 0xD0:
        *mode = 0;
        break;

    case 0x90:
        *mode = 1;
        break;

    case 0x09:
        *mode = 2;
        break;

    case 0xD9:
        *mode = 3;
        break;

    case 0x99:
        *mode = 4;
        break;

    case 0x0A:
        *mode = 5;
        break;

    case 0xDA:
        *mode = 6;
        break;

    case 0x9A:
        *mode = 7;
        break;

    case 0xDB:
        *mode = 8;
        break;

    case 0x10:
        *mode = 9;
        break;

    case 0x19:
        *mode = 10;
        break;
    }

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

NTSTATUS FastcomSetIsochronous(SERIAL_DEVICE_EXTENSION *pDevExt, int mode)
{
    NTSTATUS status;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        status =  FastcomSetIsochronousFSCC(pDevExt, mode);
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Isochronous mode = %i\n", mode);

        pDevExt->Isochronous = mode;
    }

    return status;
}

NTSTATUS FastcomGetIsochronous(SERIAL_DEVICE_EXTENSION *pDevExt, int *mode)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        FastcomGetIsochronousFSCC(pDevExt, mode);
        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS FastcomEnableIsochronous(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned mode)
{
    return FastcomSetIsochronous(pDevExt, mode);
}

NTSTATUS FastcomDisableIsochronous(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    return FastcomSetIsochronous(pDevExt, -1);
}

// Active low
void FastcomSetTerminationPCIe(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR current_mpio_lvl, new_mpio_lvl;
    
    current_mpio_lvl = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVL_OFFSET);

    if (enable)
        new_mpio_lvl = current_mpio_lvl & ~(0x1 << pDevExt->Channel);
    else
        new_mpio_lvl = current_mpio_lvl | (0x1 << pDevExt->Channel);

    pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOLVL_OFFSET, new_mpio_lvl);
}

// Active low
void FastcomGetTerminationPCIe(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UCHAR mpio_lvl;
    
    mpio_lvl = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVL_OFFSET);

    *enabled = mpio_lvl & (0x1 << pDevExt->Channel) ? FALSE : TRUE;
}

NTSTATUS FastcomSetTermination(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCIe:
        FastcomSetTerminationPCIe(pDevExt, enable);
        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Termination = %i\n", enable);

        pDevExt->Termination = enable;
    }

    return status;
}

NTSTATUS FastcomGetTermination(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCIe:
        FastcomGetTerminationPCIe(pDevExt, enabled);
        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS FastcomEnableTermination(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    return FastcomSetTermination(pDevExt, TRUE);
}

NTSTATUS FastcomDisableTermination(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    return FastcomSetTermination(pDevExt, FALSE);
}

void FastcomSetEchoCancelPCI(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR current_mpio_lvl, new_mpio_lvl;
    
    current_mpio_lvl = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVL_OFFSET);

    if (enable)
        new_mpio_lvl = current_mpio_lvl | 0x80; /* Enable echo cancel */
    else
        new_mpio_lvl = current_mpio_lvl & ~0x80; /* Disable echo cancel */

    pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOLVL_OFFSET, new_mpio_lvl);
}

void FastcomSetEchoCancelPCIe(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR current_mpio_lvlh, new_mpio_lvlh;
    
    current_mpio_lvlh = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVLH_OFFSET);

    if (enable)
        new_mpio_lvlh = current_mpio_lvlh | (0x1 << pDevExt->Channel);
    else
        new_mpio_lvlh = current_mpio_lvlh & ~(0x1 << pDevExt->Channel);

    pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOLVLH_OFFSET, new_mpio_lvlh);
}

void FastcomSetEchoCancelFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UINT32 current_fcr, new_fcr;
	UINT32 bit_mask;

    current_fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

	switch (pDevExt->Channel) {
	case 0:
		bit_mask = 0x00010000;
		break;

	case 1:
		bit_mask = 0x00100000;
		break;
	}

	if (enable)
		new_fcr = current_fcr | bit_mask;
	else
		new_fcr = current_fcr & ~bit_mask;

    WRITE_PORT_ULONG(ULongToPtr(pDevExt->Bar2), new_fcr);
}

void FastcomSetEchoCancel(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        FastcomSetEchoCancelPCI(pDevExt, enable);
        break;

    case CARD_TYPE_PCIe:
        FastcomSetEchoCancelPCIe(pDevExt, enable);
        break;

    case CARD_TYPE_FSCC:
        FastcomSetEchoCancelFSCC(pDevExt, enable);
        break;

    default:
        break; //TODO
    }

    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                     "Echo cancel = %i\n", enable);

    pDevExt->EchoCancel = enable;
}

void FastcomEnableEchoCancel(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    FastcomSetEchoCancel(pDevExt, TRUE);
}

void FastcomDisableEchoCancel(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    FastcomSetEchoCancel(pDevExt, FALSE);
}

void FastcomGetEchoCancelPCI(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UCHAR mpio_lvl;
    
    mpio_lvl = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVL_OFFSET);

    *enabled = mpio_lvl & 0x80 ? TRUE : FALSE;
}

void FastcomGetEchoCancelPCIe(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UCHAR mpio_lvlh;
    
    mpio_lvlh = pDevExt->SerialReadUChar(pDevExt->Controller + MPIOLVLH_OFFSET);

    *enabled = mpio_lvlh & (0x1 << pDevExt->Channel) ? TRUE : FALSE;
}

void FastcomGetEchoCancelFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UINT32 fcr;
	UINT32 bit_mask;

    fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

	switch (pDevExt->Channel) {
	case 0:
		bit_mask = 0x00010000;
		break;

	case 1:
		bit_mask = 0x00100000;
		break;
	}

	*enabled = (fcr & bit_mask) ? TRUE : FALSE;
}

void FastcomGetEchoCancel(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        FastcomGetEchoCancelPCI(pDevExt, enabled);
        break;

    case CARD_TYPE_PCIe:
        FastcomGetEchoCancelPCIe(pDevExt, enabled);
        break;

    case CARD_TYPE_FSCC:
        FastcomGetEchoCancelFSCC(pDevExt, enabled);
        break;

    default:
        break; //TODO
    }
}

struct ResultStruct {
    double target; 
    double freq;
    double errorPPM;
    int VCO_Div;
    int refDiv;
    int outDiv; 
    int failed;
};

struct IcpRsStruct {
    double pdf;
    double nbw;
    double ratio;
    double df;
    ULONG Rs;
    double icp;
    ULONG icpnum;   //I have to use this in the switch statement because 8.75e-6 becomes 874
};

//WILL: Range 200 Hz - 270MHz
int GetICS30703Data(unsigned long desired, unsigned long ppm, struct ResultStruct *theOne, struct IcpRsStruct *theOther, unsigned char *progdata)
{
//  double inputfreq=18432000.0;
    double inputfreq=24000000.0;

    ULONG od=0; //Output Divider
    unsigned r=0;
    unsigned v=0;
    unsigned V_Divstart=0;
    double freq=0;
    ULONG bestFreq=0;
    ULONG check=0;
    unsigned maxR;
    unsigned minR;
    unsigned max_V=2055;
    unsigned min_V=12;
    double allowable_error;
    double freq_err;
    struct ResultStruct Results;
    ULONG i,j;
    struct IcpRsStruct IRStruct;
    unsigned count;
    ULONG Rs;
    double optimal_ratio=15.0;
    double optimal_df=0.7;
    double best_ratio=0;
    double best_df=0;
    double rule1, rule2;
    int tempint;

    int InputDivider=0;
    int VCODivider=0;
    ULONG ChargePumpCurrent=0;
    ULONG LoopFilterResistor=0;
    ULONG OutputDividerOut1=0;
    ULONG OutputDividerOut2=0;
    ULONG OutputDividerOut3=0;
    ULONG temp=0;
    unsigned long requestedppm;

    requestedppm=ppm;

    if( inputfreq == 18432000.0) 
    {
        maxR = 921;
        minR = 1;
    }
    else if( inputfreq == 24000000.0) 
    {
        maxR = 1200;
        minR = 1;
    }

    ppm=0;
increaseppm:
//  DbgPrint("ICS30703: ppm = %d\n",ppm);
    allowable_error  = ppm * desired/1e6; // * 1e6

    for( r = minR; r <= maxR; r++ )
    {
        rule2 = inputfreq /(double)r;
        if ( (rule2 < 20000.0) || (rule2 > 100000000.0) )
        {
//          DbgPrint("Rule2(r=%d): 20,000<%f<100000000\n",r,rule2);
            continue;   //next r
        }

        od=8232;
        while(od > 1)
        {
            //set starting VCO setting with output freq just below target
            V_Divstart = (int) (((desired - (allowable_error) ) * r * od) / (inputfreq));

            //check if starting VCO setting too low
            if (V_Divstart < min_V)
                V_Divstart = min_V;
            
            //check if starting VCO setting too high
            else if (V_Divstart > max_V)
                V_Divstart = max_V;

            /** Loop thru VCO divide settings**/
            //Loop through all VCO divider ratios
            for( v = V_Divstart; v <= max_V; v++ ) //Check all Vco divider settings
            {
                rule1 = (inputfreq * ((double)v / (double)r) );

                        if(od==2)
                {
                    if( (rule1 < 90000000.0) || (rule1 > 540000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }
                else if(od==3)
                {
                    if( (rule1 < 90000000.0) || (rule1 > 720000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }
                else if( (od>=38) && (od<=1029) )
                {
                    if( (rule1 < 90000000.0) || (rule1 > 570000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }
                else
                {
                if( (rule1 < 90000000.0) || (rule1 > 730000000.0)  )
                {
//                  printf("Rule1: 90MHz<%f<730MHz\n",rule1);
                    continue;   //next VCO_Div
                    }
                }

                freq = (inputfreq * ((double)v / ((double)r * (double)od)));
                freq_err    = fabs(freq - desired) ; //Hz

                if ((freq_err) > allowable_error)
                {
                    continue; //next VCO_Div
                }
                else if((freq_err) <= allowable_error)
                {
                    count=0;
                    for(i=0;i<4;i++)
                    {
                        switch(i)
                        {
                        case 0:
                            Rs = 64000;
                            break;
                        case 1:
                            Rs = 52000;
                            break;
                        case 2:
                            Rs = 16000;
                            break;
                        case 3:
                            Rs = 4000;
                            break;
                        default:
                            return 1;
                        }

                        for(j=0;j<20;j++)
                        {
                            IRStruct.Rs=Rs;
                            switch(j)
                            {
                            case 0:
                                IRStruct.icp=1.25e-6;
                                IRStruct.icpnum=125;
                                break;
                            case 1:
                                IRStruct.icp=2.5e-6;
                                IRStruct.icpnum=250;
                                break;
                            case 2:
                                IRStruct.icp=3.75e-6;
                                IRStruct.icpnum=375;
                                break;
                            case 3:
                                IRStruct.icp=5.0e-6;
                                IRStruct.icpnum=500;
                                break;
                            case 4:
                                IRStruct.icp=6.25e-6;
                                IRStruct.icpnum=625;
                                break;
                            case 5:
                                IRStruct.icp=7.5e-6;
                                IRStruct.icpnum=750;
                                break;
                            case 6:
                                IRStruct.icp=8.75e-6;
                                IRStruct.icpnum=875;
                                break;
                            case 7:
                                IRStruct.icp=10.0e-6;
                                IRStruct.icpnum=1000;
                                break;
                            case 8:
                                IRStruct.icp=11.25e-6;
                                IRStruct.icpnum=1125;
                                break;
                            case 9:
                                IRStruct.icp=12.5e-6;
                                IRStruct.icpnum=1250;
                                break;
                            case 10:
                                IRStruct.icp=15.0e-6;
                                IRStruct.icpnum=1500;
                                break;
                            case 11:
                                IRStruct.icp=17.5e-6;
                                IRStruct.icpnum=1750;
                                break;
                            case 12:
                                IRStruct.icp=18.75e-6;
                                IRStruct.icpnum=1875;
                                break;
                            case 13:
                                IRStruct.icp=20.0e-6;
                                IRStruct.icpnum=2000;
                                break;
                            case 14:
                                IRStruct.icp=22.5e-6;
                                IRStruct.icpnum=2250;
                                break;
                            case 15:
                                IRStruct.icp=25.0e-6;
                                IRStruct.icpnum=2500;
                                break;
                            case 16:
                                IRStruct.icp=26.25e-6;
                                IRStruct.icpnum=2625;
                                break;
                            case 17:
                                IRStruct.icp=30.0e-6;
                                IRStruct.icpnum=3000;
                                break;
                            case 18:
                                IRStruct.icp=35.0e-6;
                                IRStruct.icpnum=3500;
                                break;
                            case 19:
                                IRStruct.icp=40.0e-6;
                                IRStruct.icpnum=4000;
                                break;
                            default:
                                DbgPrint("ICS30703: switch(j:icp) - You shouldn't get here! %d\n",j);
                            }//end switch(j)
//                          printf("Rs=%5d ",IRStruct.Rs);
//                          printf("Icp=%2.2f ",IRStruct.icp*10e5);

                            IRStruct.pdf = (inputfreq / (double)r) ;
//                          printf("pdf=%12.2f ",IRStruct.pdf);

                            IRStruct.nbw = ( ((double)IRStruct.Rs * IRStruct.icp * 310.0e6) / (2.0 * 3.14159 * (double)v) );
//                          printf("nbw=%15.3f ",IRStruct.nbw);

                            IRStruct.ratio = (IRStruct.pdf/IRStruct.nbw);

                            tempint = (int)(IRStruct.ratio*10.0);
                            if((IRStruct.ratio*10.0)-tempint>=0.0) tempint++;
                            IRStruct.ratio = (double)tempint/10.0;

//                          IRStruct.ratio = ceil(IRStruct.ratio*10.0); //these two statements make the
//                          IRStruct.ratio = IRStruct.ratio/10.0;       //ratio a little nicer to compare

//                          printf("ratio=%2.4f ",IRStruct.ratio);

                            IRStruct.df = ( ((double)IRStruct.Rs / 2) * (sqrt( ((IRStruct.icp * 0.093) / (double)v))) );
//                          printf("ndf=%12.3f\n",IRStruct.df);

                            count++;
                            if( (IRStruct.ratio>30) || (IRStruct.ratio<7) || (IRStruct.df>2.0) || (IRStruct.df<0.2) )
                            {
                                continue;
                            }
                            else 
                            {
                                Results.target    = desired; 
                                Results.freq      = freq;
                                Results.errorPPM     = freq_err / desired * 1.0e6 ;
                                Results.VCO_Div   = v;
                                Results.refDiv    = r;
                                Results.outDiv    = od; 
                                Results.failed = FALSE;
                                goto finished;                              
                            }
                        }//end for(j=0;j<20;j++)
                    }//end for(i=0;i<4;i++)
                }
            }//end of for( v = V_Divstart; v < max_V; v++ )

            if(od<=1030)
                od--;
            else if(od<=2060)
                od=od-2;
            else if(od<=4120)
                od=od-4;
            else od=od-8;

        }//end of while(od <= 8232)
    }//end of for( r = maxR, *saved_result_num = 0; r >= minR; r-- )

    ppm++;
    if(ppm>requestedppm)
    {
        return 2;
    }
    else 
    {
//      DbgPrint("ICS30703: increasing ppm to %d\n",ppm);
        goto increaseppm;
    }

finished:
    
    memcpy(theOne,&Results,sizeof(struct ResultStruct));

    memcpy(theOther,&IRStruct,sizeof(struct IcpRsStruct));
/*  
    DbgPrint("ICS30703: Best result is \n");
    DbgPrint("\tRD = %4i,",Results.refDiv);
    DbgPrint(" VD = %4i,",Results.VCO_Div);
    DbgPrint(" OD = %4i,",Results.outDiv);
    DbgPrint(" freq_Hz = %ld,\n",(ULONG)Results.freq);
    
    DbgPrint("\tRs = %5d, ",IRStruct.Rs);
    DbgPrint("Icp = %d, ",(ULONG)(IRStruct.icp*1e6));
    //  DbgPrint("pdf = %d, ",(ULONG)IRStruct.pdf);
    //  DbgPrint("nbw = %d, ",(ULONG)IRStruct.nbw);
    DbgPrint("ratio = %d, ",(ULONG)IRStruct.ratio);
    DbgPrint("df = %d\n",(ULONG)IRStruct.df*1000);
*/  
//  DbgPrint("ICS307-03 freq_Hz = %ld,\n",(ULONG)Results.freq);
/*
first, choose the best dividers (V, R, and OD) with

1st key best accuracy
2nd key lowest reference divide
3rd key highest VCO frequency (OD)

then choose the best loop filter with

1st key best PDF/NBW ratio (between 7 and 30, 15 is optimal)
2nd key best damping factor (between 0.2 and 2, 0.7 is optimal)
*/
/* this is 1MHz
    progdata[19]=0xff;
    progdata[18]=0xff;
    progdata[17]=0xff;
    progdata[16]=0xf0;
    progdata[15]=0x00;
    progdata[14]=0x01;
    progdata[13]=0x43;
    progdata[12]=0x1a;
    progdata[11]=0x9c;
    progdata[10]=0x00;
    progdata[9]=0x00;
    progdata[8]=0x00;
    progdata[7]=0x00;
    progdata[6]=0x00;
    progdata[5]=0x00;
    progdata[4]=0x00;
    progdata[3]=0x00;
    progdata[2]=0x0c;
    progdata[1]=0xdf;
    progdata[0]=0xee;
    goto doitnow;
*/
/* 10 MHz
    progdata[19]=0xff;
    progdata[18]=0xff;
    progdata[17]=0xff;
    progdata[16]=0x00;
    progdata[15]=0x80;
    progdata[14]=0x01;
    progdata[13]=0x00;
    progdata[12]=0x66;
    progdata[11]=0x38;
    progdata[10]=0x00;
    progdata[9]=0x00;
    progdata[8]=0x00;
    progdata[7]=0x00;
    progdata[6]=0x00;
    progdata[5]=0x00;
    progdata[4]=0x00;
    progdata[3]=0x00;
    progdata[2]=0x07;
    progdata[1]=0x20;
    progdata[0]=0x12;
    goto doitnow;
*/

    progdata[19]=0xff;
    progdata[18]=0xff;
    progdata[17]=0xff;
    progdata[16]=0x00;
    progdata[15]=0x00;
    progdata[14]=0x00;
    progdata[13]=0x00;
    progdata[12]=0x00;
    progdata[11]=0x00;
    progdata[10]=0x00;
    progdata[9]=0x00;
    progdata[8]=0x00;
    progdata[7]=0x00;
    progdata[6]=0x00;
    progdata[5]=0x00;
    progdata[4]=0x00;
    progdata[3]=0x00;
    progdata[2]=0x00;
    progdata[1]=0x00;
    progdata[0]=0x00;

//  progdata[16]|=0x02; //enable CLK3
//  progdata[15]&=0xef; //CLK3 source select: 1=CLK1, 0=CLK1 before OD 
//  progdata[15]|=0x08; //CLK2 source select: 1=CLK1, 0=CLK1 before OD
//  progdata[15]|=0x40; //reference source is: 1=crystal, 0=clock
    progdata[14]|=0x01; //1=Power up, 0=power down feedback counter, charge pump and VCO
//  progdata[13]|=0x80; //enable CLK2
    progdata[13]|=0x40; //enable CLK1

    InputDivider = theOne->refDiv;
    VCODivider = theOne->VCO_Div;
    ChargePumpCurrent = theOther->icpnum;
    LoopFilterResistor = theOther->Rs;
    OutputDividerOut1 = theOne->outDiv;

    //InputDivider=2;
    //VCODivider=60;
    //OutputDividerOut1 = 45;
    //LoopFilterResistor=16000;
    //ChargePumpCurrent=3500;

    /* Table 1: Input Divider */

    if( (InputDivider==1)||(InputDivider==2) )
    {
        switch(InputDivider)
        {
        case 1:
            progdata[0]&=0xFC;
            progdata[1]&=0xF0;
            break;
        case 2:
            progdata[0]&=0xFC;
            progdata[0]|=0x01;
            progdata[1]&=0xF0;
            break;
        }
//  printf("1 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);
    }
    else if( (InputDivider>=3) && (InputDivider<=17) )
    {
        temp=~(InputDivider-2);
        temp = (temp << 2);
        progdata[0]=(unsigned char)temp&0xff;

        progdata[0]&=0x3e;  //set bit 0 to a 0
        progdata[0]|=0x02;  //set bit 1 to a 1

//      printf("2 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);
    }
    else if( (InputDivider>=18) && (InputDivider<=2055) )
    {
        temp=InputDivider-8;
        temp = (temp << 2);
        progdata[0]=(unsigned char)temp&0xff;
        progdata[1]=(unsigned char)((temp>>8)&0xff);

        progdata[0]|=0x03;  //set bit 0 and 1 to a 1

//      printf("3 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);

    }
    else 
        return 3;

    /* Table 2 VCO Divider */

    if( (VCODivider >= 12) && (VCODivider <=2055) )
    {
        temp=VCODivider-8;
        temp=(temp << 5);
        progdata[1]|=temp&0xff;
        progdata[2]|=((temp>>8)&0xff);
//      printf("4 0x%2.2X,0x%2.2X\n",progdata[2],progdata[1]);
    }
    else return 4;

    /* Table 4 Loop Filter Resistor */

    switch(LoopFilterResistor)
    {
    case 64000:
        progdata[11]&=0xf9; //bit 89 and 90 = 0
        break;
    case 52000:
        progdata[11]&=0xf9; //bit 89 = 0
        progdata[11]|=0x04; //bit 90 = 1
        break;
    case 16000:
        progdata[11]&=0xf9; //bit 90 = 0
        progdata[11]|=0x02; //bit 89 = 1
        break;
    case 4000:
        progdata[11]|=0x06; //bit 89 and 90 = 1
        break;
    default:
        return 5;
    }
//  printf("5 0x%2.2X\n",progdata[11]);

    /* Table 3 Charge Pump Current */

    switch(ChargePumpCurrent)
    {
    case 125:
        progdata[11]|=0x38;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
//      printf("125\n");
        break;

    case 250:
        progdata[11]|=0x38;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 375:
        progdata[11]|=0x38;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 500:
        progdata[11]|=0x38;
        progdata[15]|=0x80;
        progdata[16]|=0x01;             
        break;

    case 625:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 750:
        progdata[11]|=0x10;
        progdata[11]&=0xd7;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 875:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 1000:
        progdata[11]&=0xc7;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 1125:
        progdata[11]|=0x28;
        progdata[11]&=0xef;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 1250:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 1500:
        progdata[11]|=0x28;
        progdata[11]&=0xef;
        progdata[15]|=0x80;
        progdata[16]|=0x01;             
        break;

    case 1750:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 1875:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 2000:
        progdata[11]&=0xc7;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 2250:
        progdata[11]|=0x10;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 2500:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    case 2625:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 3000:
        progdata[11]&=0xc7;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 3500:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    case 4000:
        progdata[11]&=0xc7;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    default:
        return 6;
    }//end switch(j)
//  printf("6 0x%2.2X, 0x%2.2X, 0x%2.2X\n",progdata[16],progdata[15],progdata[11]);

    /* Table 5 Output Divider for Output 1 */
//OutputDividerOut1=38;
    if( (OutputDividerOut1 >= 2) && (OutputDividerOut1 <= 8232) )
    {
        switch(OutputDividerOut1)
        {
        case 2:
            progdata[11]&=0x7f;
            progdata[12]&=0x00;
            progdata[13]&=0xc0;
            break;

        case 3:
            progdata[11]|=0x80;
            progdata[12]&=0x00;
            progdata[13]&=0xc0;
            break;

        case 4:
            progdata[11]&=0x7f;
            progdata[12]|=0x04;
            progdata[13]&=0xc0;
            break;

        case 5:
            progdata[11]&=0x7f;
            progdata[12]|=0x01;
            progdata[13]&=0xc0;
            break;

        case 6:
            progdata[11]|=0x80;
            progdata[12]|=0x04;
            progdata[13]&=0xc0;
            break;

        case 7:
            progdata[11]|=0x80;
            progdata[12]|=0x01;
            progdata[13]&=0xc0;
            break;

        case 11:
            progdata[11]|=0x80;
            progdata[12]|=0x09;
            progdata[13]&=0xc0;
            break;

        case 9:
            progdata[11]|=0x80;
            progdata[12]|=0x05;
            progdata[13]&=0xc0;
            break;

        case 13:
            progdata[11]|=0x80;
            progdata[12]|=0x0d;
            progdata[13]&=0xc0;
            break;

        case 8: case 10: case 12: case 14: case 15: case 16: case 17:case 18: case 19:
        case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28:
        case 29: case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
            temp = ~(OutputDividerOut1-6);
            temp = (temp << 2);
            progdata[12] = (unsigned char)temp & 0x7f;
            
            progdata[11]&=0x7f;
            progdata[12]&=0xfe;
            progdata[12]|=0x02;
            progdata[13]&=0xc0;
            break;

        default:

            for(i=0;i<512;i++)
            {
                if( OutputDividerOut1 == ((((i+3)*2)+0)*(1)) )
                {
//                  printf("1 x=%d, y=0, z=0\n",i);
//                  DbgPrint("outputdivider1\n");
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xe7;
                    progdata[12]|=0x04;
                    break;
                }
                
                else if( OutputDividerOut1 == ((((i+3)*2)+0)*(2)) )
                {
//                  printf("2 x=%d, y=0, z=1\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xef;
                    progdata[12]|=0x0c;
                    break;
                }
                    
                else if( OutputDividerOut1 == ((((i+3)*2)+0)*(4)) )
                {
//                  printf("3 x=%d, y=0, z=2\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xf7;
                    progdata[12]|=0x14;
                    break;
                }
                    
                else if( OutputDividerOut1 == ( (((i+3)*2)+0)*(8)) )
                {
//                  printf("4 x=%d, y=0, z=3\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]|=0x1c;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(1)) )
                {
//                  printf("5 x=%d, y=1, z=0\n",i);
//                  DbgPrint("outputdivider5\n");
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xe3;
                    break;
                }
                    
                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(2)) )
                {
//                  printf("6 x=%d, y=1, z=1\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xeb; //power of 1
                    progdata[12]|=0x08;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(4)) )
                {
//                  printf("7 x=%d, y=1, z=2\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xf7;
                    progdata[12]|=0x10;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(8)) )
                {
//                  printf("8 x=%d, y=1, z=3\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xfb;
                    progdata[12]|=0x18;
                    break;
                }
            }
            progdata[11]|=0x80; //1
            progdata[12]&=0xfe; //0
            progdata[12]|=0x02; //1
        }
//      printf("0x%2.2x, 0x%2.2x, 0x%2.2x\n\n",progdata[13]&0x3f, progdata[12], progdata[11]&0x80);
    }
    else return 7;
//doitnow:

/*  progdata[15]|=0x03; //this will set
    progdata[14]|=0xc0; //the OD of clock 3
    progdata[11]&=0xbf; //to 2
*/
    return 0;

}
//end of GetICS30703Bits

#define STRB_BASE 0x00000008
#define DTA_BASE 0x00000001
#define CLK_BASE 0x00000002

NTSTATUS FastcomSetClockRateFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    UINT32 orig_fcr_value = 0;
    UINT32 new_fcr_value = 0;
    int j = 0; // Must be signed because we are going backwards through the array
    int i = 0; // Must be signed because we are going backwards through the array
    unsigned strb_value = STRB_BASE;
    unsigned dta_value = DTA_BASE;
    unsigned clk_value = CLK_BASE;
    UINT32 *data = 0;
    unsigned data_index = 0;
    unsigned char clock_data[20];

    struct ResultStruct solutiona;  //final results for ResultStruct data calculations
    struct IcpRsStruct solutionb;   //final results for IcpRsStruct data calculations

    if (value < 15000 || value > 270000000)
        return STATUS_INVALID_PARAMETER;

    memset(&solutiona,0,sizeof(struct ResultStruct));
    memset(&solutionb,0,sizeof(struct IcpRsStruct));

    GetICS30703Data(value, 2, &solutiona, &solutionb, clock_data);


#ifdef DISABLE_XTAL
    clock_data[15] &= 0xfb;
#else
    /* This enables XTAL on all cards except green FSCC cards with a revision
       greater than 6 and 232 cards. Some old protoype SuperFSCC cards will 
       need to manually disable XTAL as they are not supported in this driver 
       by default. */
    if ((FsccGetPdev(pDevExt) == 0x0f && FsccGetPrev(pDevExt) <= 6) ||
        FsccGetPdev(pDevExt) == 0x16) {
        clock_data[15] &= 0xfb;
    }
    else {
        clock_data[15] |= 0x04;
    }
#endif


    data = (UINT32 *)ExAllocatePoolWithTag(NonPagedPool, sizeof(UINT32) * 323, 'stiB');

    if (data == NULL) {
        SerialDbgPrintEx(TRACE_LEVEL_ERROR, DBG_PNP,
                        "--> ExAllocatePoolWithTag failed\n");
        return STATUS_UNSUCCESSFUL;
    }
    
    if (pDevExt->Channel == 1) {
        strb_value <<= 0x08;
        dta_value <<= 0x08;
        clk_value <<= 0x08;
    }

    orig_fcr_value = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

    data[data_index++] = new_fcr_value = orig_fcr_value & 0xfffff0f0;

    for (i = 19; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            int bit = ((clock_data[i] >> j) & 1);

            if (bit)
                new_fcr_value |= dta_value; /* Set data bit */
            else
                new_fcr_value &= ~dta_value; /* Clear clock bit */

            data[data_index++] = new_fcr_value |= clk_value; /* Set clock bit */
            data[data_index++] = new_fcr_value &= ~clk_value; /* Clear clock bit */
        }
    }

    new_fcr_value = orig_fcr_value & 0xfffff0f0;

    new_fcr_value |= strb_value; /* Set strobe bit */
    new_fcr_value &= ~clk_value; /* Clear clock bit */

    data[data_index++] = new_fcr_value;
    data[data_index++] = orig_fcr_value;

    WRITE_PORT_BUFFER_ULONG(ULongToPtr(pDevExt->Bar2), data, data_index);
    
    ExFreePoolWithTag (data, 'stiB');

    return STATUS_SUCCESS;
}

// Copied from old code. Needs a major code cleanup. TODO
// ICS307-02
#define MPIO_SDTA           0x01    //Each bit of MPIOLVL register
#define MPIO_SCLK           0x02
#define MPIO_SSTB           0x04

//WILL: Range 6MHz - 200MHz
NTSTATUS FastcomSetClockRatePCI(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned rate)
{
    #define STARTWRD 0x1e05
    #define MIDWRD   0x1e04
    #define ENDWRD   0x1e00
    
    unsigned long bestVDW=1;    //Best calculated VCO Divider Word
    unsigned long bestRDW=1;    //Best calculated Reference Divider Word
    unsigned long bestOD=1;     //Best calculated Output Divider
    unsigned long result=0;
    unsigned long t=0;
    unsigned long i=0;
    unsigned long j=0;
    unsigned long tempValue=0;
    unsigned long temp=0;
    unsigned long vdw=1;        //VCO Divider Word
    unsigned long rdw=1;        //Reference Divider Word
    unsigned long od=1;         //Output Divider
    unsigned long lVDW=1;       //Lowest vdw
    unsigned long lRDW=1;       //Lowest rdw
    unsigned long lOD=1;        //Lowest OD
    unsigned long hVDW=1;       //Highest vdw
    unsigned long hRDW=1;       //Highest rdw
    unsigned long hOD=1;        //Highest OD
    unsigned long hi;       //initial range freq Max
    unsigned long low;  //initial freq range Min
    unsigned long check;        //Calculated clock
    unsigned long clk1;     //Actual clock 1 output
    unsigned long inFreq=18432000;  //Input clock frequency
    unsigned long range1=0;     //Desired frequency range limit per ics307 mfg spec.
    unsigned long range2=0;     ////Desired frequency range limit per ics307 mfg spec.
    UCHAR data = 0;
    UCHAR saved = 0;
    PUCHAR base_add;
    int odskip=0;



    if (rate < 6000000 || rate > 200000000)
        return STATUS_INVALID_PARAMETER;

    //DBGP("SerialSetClock Executing %d\n",*rate);

    base_add = pDevExt->Controller;
    
    //DBGP("baseaddress = %8.8x \n", base_add);
    

    saved = READ_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET);  //Save MPIO pin state
    data = saved;

    hi = (rate + (rate / 10));
    low = (rate - (rate / 10));

    od = 2;
    while (od <= 10)
    {
        switch(od)  //check maximum frequency with given OD for industrial temp chips
        {
        case 2:
            if(rate>180000000)
                odskip=1;
            break;
        case 3:
            if(rate>120000000)
                odskip=1;
            break;
        case 4:
            if(rate>90000000)
                odskip=1;
            break;
        case 5:
            if(rate>72000000)
                odskip=1;
            break;
        case 6:
            if(rate>60000000)
                odskip=1;
            break;
        case 7:
            if(rate>50000000)
                odskip=1;
            break;
        case 8:
            if(rate>45000000)
                odskip=1;
            break;
        case 9: //OD=9 not allowed
            odskip=1;
        case 10:
            if(rate>36000000)
                odskip=1;
            break;
        default:
            //DBGP("Case 1 Invalid OD = %ld.\n",od);
            return STATUS_UNSUCCESSFUL;
        }

        rdw = 1;
        while ( (rdw <= 127) && (odskip==0) )
        {
            vdw = 4;
            while (vdw <= 511)
            {
                check = (((inFreq * 2) / ((rdw + 2)*od)) * (vdw + 8) ); //calculate a check frequency
                range1 = ((inFreq * 2 * (vdw + 8)) / (rdw + 2));
                range2 = (inFreq / (rdw + 2));
                //Calculate operating ranges

                if ( ((range1) > 60000000) && ((range1) < 360000000) && ((range2) > 200000) )   //check operating ranges
                {
                    if (check == low)   //If this combination of variables == the current lowest set
                    {
                        if (lRDW > rdw) //If this combination has a lower rdw
                        {
                            lVDW=vdw;
                            lRDW=rdw;
                            lOD=od;
                            low=check;
                        }
                        else if ((lRDW == rdw) && (lVDW < vdw)) //If this combo has the same rdw then take the higher vdw
                        {
                            lVDW=vdw;
                            lRDW=rdw;
                            lOD=od;
                            low=check;
                        }

                    }
                    else if (check == hi)   //If this combination of variables == the current lowest set
                    {
                        if (hRDW > rdw) //If this combination has a lower rdw
                        {
                            hVDW=vdw;
                            hRDW=rdw;
                            hOD=od;
                            hi=check;
                        }
                        else if ((hRDW == rdw) && (hVDW < vdw)) //If this combo has the same rdw then take the higher vdw   
                        {
                            hVDW=vdw;
                            hRDW=rdw;
                            hOD=od;
                            hi=check;
                        }

                    }


                    if ((check > low) && (check < hi))      //if difference is less than previous difference
                    {
                        if (check > rate)    //if the new combo is larger than the rate, new hi combination
                        {
                            hi=check;
                            hVDW=vdw;
                            hRDW=rdw;
                            hOD=od;
                        }
                            
                        else    //if the new combo is less than the rate, new low combination
                        {
                            low=check;
                            lVDW = vdw;
                            lRDW = rdw;
                            lOD = od;
                        }
                }
            }

            vdw++;
        }

        rdw++;
    }
    odskip=0;
    od++;
    if (od==9)
        od++;
    }

    if ((hi - rate) < (rate - low))
    {
        bestVDW=hVDW;
        bestRDW=hRDW;
        bestOD=hOD;
        clk1=hi;
    }
    else
    {
        bestVDW=lVDW;
        bestRDW=lRDW;
        bestOD=lOD;
        clk1=low;
    }
    switch(bestOD)
    {
    case 2:
        result=0x11;
        break;
    case 3:
        result=0x16;
        break;
    case 4:
        result=0x13;
        break;
    case 5:
        result=0x14;
        break;
    case 6:
        result=0x17;
        break;
    case 7:
        result=0x15;
        break;
    case 8:
        result=0x12;
        break;
    case 10:
        result=0x10;
        break;
    default:
        //DBGP("Case 2 Invalid OD=%ld.\n",od);
        return STATUS_UNSUCCESSFUL;
        
    }
    range1 = (inFreq * 2 * ((bestVDW + 8)/(bestRDW + 2)));
    range2 = (inFreq/(bestRDW + 2));
    clk1 = (((inFreq * 2) / ((bestRDW + 2)*bestOD)) * (bestVDW + 8) );
//      DBGP(" 60 MHz < %d MHz < 360 MHz\n", range1);
//      DBGP(" 200 kHz < %d kHz \n", range2*1000);
    result<<=9;
    result|=bestVDW;
    result<<=7;
    result|=bestRDW;
//      result|=0x200000;   //1 = set levels to TTL, 0 = CMOS

//      DBGP("Clock bytes = %X\n",result);

    //DBGP("The rate clock control bytes are: %X\n",result);
//      DBGP("The rate frequendy is: %dHz\n", rate);
//      DBGP("The base clock is: %dHz\n", inFreq);
//      DBGP("High: %d    ",hi);
//      DBGP("Low: %d.\n",low);
    //DBGP("The best calculated clock is: %dHz\n", clk1);
//      DBGP("The best VDW = %d  RDW = %d  OD = %d.\n", bestVDW, bestRDW, bestOD);
//      DBGP("CH_ID = %d \n", extension->chid);


    /*********************** Begin clock set *************************/

    tempValue = (result & 0x00ffffff);


    for(i=0;i<24;i++)
    {
        //data bit set
        if ((tempValue & 0x800000)!=0) 
        {
        data |= MPIO_SDTA;
//              DBGP("1");
        }
        else 
        {
            data &= ~MPIO_SDTA;
//              DBGP("0");
        }
        WRITE_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET,data);
        KeStallExecutionProcessor(20);  

                //clock high, data still there
                data |= MPIO_SCLK;
        WRITE_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET,data);
        KeStallExecutionProcessor(20);  
                //clock low, data still there
                data &= MPIO_SDTA;
        WRITE_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET,data);
        KeStallExecutionProcessor(20);  

        tempValue=tempValue<<1;
    }
//      DBGP("\n");
    
    data &= 0xF8;
    data |= MPIO_SSTB;      //strobe on
    WRITE_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET,data);
    KeStallExecutionProcessor(20);  
    
    data &= ~MPIO_SSTB;     //all off
    WRITE_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET,data);
    KeStallExecutionProcessor(20);

    WRITE_REGISTER_UCHAR(base_add+MPIOLVL_OFFSET,saved);   //Put MPIO pins back to saved state.
    return STATUS_SUCCESS;
}

NTSTATUS PCIeSetBaudRate(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    const unsigned input_freq = 125000000;
    const unsigned prescaler = 1;
    float divisor = 0;
    UCHAR orig_lcr = 0;
    UCHAR dlm = 0;
    UCHAR dll = 0;
    UCHAR dld = 0;
    UCHAR tmp = 0;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr | 0x80);

    divisor = (float)input_freq / prescaler / (value * pDevExt->SampleRate);

    dlm = (int)floor(divisor) >> 8;
    dll = (int)floor(divisor) & 0xff;

    dld = pDevExt->SerialReadUChar(pDevExt->Controller + DLD_OFFSET);
    dld &= 0xf0;
    dld |= (int)floor(((divisor - floor(divisor)) * 16) + 0.5);

    pDevExt->SerialWriteUChar(pDevExt->Controller + DLM_OFFSET, dlm);  
    pDevExt->SerialWriteUChar(pDevExt->Controller + DLL_OFFSET, dll);  
    pDevExt->SerialWriteUChar(pDevExt->Controller + DLD_OFFSET, dld);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetClockRate(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned value)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        status = FastcomSetClockRatePCI(pDevExt, value);
        break;

    case CARD_TYPE_FSCC:
        status = FastcomSetClockRateFSCC(pDevExt, value);
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Clock rate = %i\n", value);

        pDevExt->ClockRate = value;  
    }

    return status;
}

NTSTATUS FastcomSetExternalTransmitFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned num_frames)
{
    UCHAR orig_lcr;

    if (num_frames > 8191)
        return STATUS_INVALID_PARAMETER;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */


    if (num_frames != 0) {
        pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, EXTH_OFFSET); /* To allow access to EXTH */
        pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, num_frames >> 8); /* Actually writing to EXTH through ICR */

        pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, EXT_OFFSET); /* To allow access to EXTH */
        pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, (char)num_frames); /* Actually writing to EXT through ICR */
    }
    else {
        pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, EXTH_OFFSET); /* To allow access to EXTH */
        pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, 0x00); /* Actually writing to EXTH through ICR */

        pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, EXT_OFFSET); /* To allow access to EXTH */
        pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, 0x00); /* Actually writing to EXT through ICR */
    }

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

void FastcomGetExternalTransmitFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *num_frames)
{
    UCHAR orig_lcr;
    UCHAR ext, exth;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x40); /* Enable ICR read enable */

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, EXT_OFFSET); /* To allow access to EXT */
    ext = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get EXT through ICR */

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, EXTH_OFFSET); /* To allow access to EXTH */
    exth = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get EXTH through ICR */

    *num_frames = ((exth & 0x1F) << 8) + ext;

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

NTSTATUS FastcomGetExternalTransmit(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *num_frames)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        FastcomGetExternalTransmitFSCC(pDevExt, num_frames);
        break;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetExternalTransmit(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned num_frames)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        status = FastcomSetExternalTransmitFSCC(pDevExt, num_frames);
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "External Transmit = %i\n", num_frames); 
    }

    return status;
}

NTSTATUS FastcomEnableExternalTransmit(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned num_frames) 
{
    return FastcomSetExternalTransmit(pDevExt, num_frames);
}

NTSTATUS FastcomDisableExternalTransmit(SERIAL_DEVICE_EXTENSION *pDevExt) 
{
    return FastcomSetExternalTransmit(pDevExt, 0);
}

NTSTATUS FastcomSetFrameLengthFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned num_chars)
{
    UCHAR orig_lcr;
    UCHAR frev;

    if (num_chars == 0 || num_chars > 256)
        return STATUS_INVALID_PARAMETER;

    frev = FsccGetFrev(pDevExt);

    if (frev < 0x20)
        return STATUS_NOT_SUPPORTED;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, FLR_OFFSET); /* To allow access to FLR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, num_chars - 1); /* Actually writing to EXTH through ICR */

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomGetFrameLengthFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *num_chars)
{
    UCHAR orig_lcr;
    UCHAR flr;
    UCHAR frev;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    frev = FsccGetFrev(pDevExt);

    if (frev < 0x20)
        return STATUS_NOT_SUPPORTED;

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x40); /* Enable ICR read enable */

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, FLR_OFFSET); /* To allow access to FLR */
    flr = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get EXT through ICR */

    *num_chars = flr + 1;

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);

    return STATUS_SUCCESS;
}

NTSTATUS FastcomSetFrameLength(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned num_chars)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        status = FastcomSetFrameLengthFSCC(pDevExt, num_chars);
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "Frame Length = %i\n", num_chars); 

        pDevExt->FrameLength = num_chars;
    }

    return status;
}

NTSTATUS FastcomGetFrameLength(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned *num_chars)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        status = FastcomGetFrameLengthFSCC(pDevExt, num_chars);
        break;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return status;
}

void FastcomSet9BitFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    UCHAR orig_lcr;
    UCHAR new_nmr;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, NMR_OFFSET); /* To allow access to NMR */

    if (enable)
        new_nmr = 0x01;
    else
        new_nmr = 0x00;

    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, new_nmr);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

NTSTATUS FastcomSet9Bit(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN enable)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        FastcomSet9BitFSCC(pDevExt, enable);
        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_NOT_SUPPORTED;
        pDevExt->NineBit = FALSE;
    }

    if (NT_SUCCESS (status)) {
        SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                         "9-Bit = %i\n", enable);

        pDevExt->NineBit = enable;
    }

    return status;
}

void FastcomGet9BitFSCC(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    UCHAR orig_lcr;
    UCHAR nmr;

    orig_lcr = READ_LINE_CONTROL(pDevExt, pDevExt->Controller);

    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, 0); /* Ensure last LCR value is not 0xbf */
    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR | 0x40); /* Enable ICR read enable */

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, NMR_OFFSET); /* To allow access to FLR */
    nmr = pDevExt->SerialReadUChar(pDevExt->Controller + ICR_OFFSET); /* Get NMR through ICR */

    *enabled = nmr & 0x01;

    pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
    pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Restore original ACR value */
    WRITE_LINE_CONTROL(pDevExt, pDevExt->Controller, orig_lcr);
}

NTSTATUS FastcomGet9Bit(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *enabled)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_FSCC:
        FastcomGet9BitFSCC(pDevExt, enabled);
        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS FastcomEnable9Bit(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    return FastcomSet9Bit(pDevExt, TRUE);
}

NTSTATUS FastcomDisable9Bit(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    return FastcomSet9Bit(pDevExt, FALSE);
}

NTSTATUS FsccIsOpenedInSync(SERIAL_DEVICE_EXTENSION *pDevExt, BOOLEAN *status)
{
    UINT32 orig_fcr;

    if (FastcomGetCardType(pDevExt) != CARD_TYPE_FSCC)
        return STATUS_NOT_SUPPORTED;

    orig_fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

    *status = (orig_fcr & (0x40000000 << pDevExt->Channel)) ? TRUE: FALSE;

    return STATUS_SUCCESS;
}

NTSTATUS FsccEnableAsync(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    UINT32 orig_fcr, new_fcr;

    if (FastcomGetCardType(pDevExt) != CARD_TYPE_FSCC)
        return STATUS_NOT_SUPPORTED;

    orig_fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

    if ((orig_fcr & (0x01000000 << pDevExt->Channel)) == FALSE) {
        /* UART_{A,B} */
        new_fcr = orig_fcr | (0x01000000 << pDevExt->Channel);

        WRITE_PORT_ULONG(ULongToPtr(pDevExt->Bar2), new_fcr);
    }

    return STATUS_SUCCESS;
}

NTSTATUS FsccDisableAsync(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    UINT32 orig_fcr, new_fcr;

    if (FastcomGetCardType(pDevExt) != CARD_TYPE_FSCC)
        return STATUS_NOT_SUPPORTED;

    orig_fcr = READ_PORT_ULONG(ULongToPtr(pDevExt->Bar2));

    /* UART_{A,B} */
    new_fcr = orig_fcr & ~(0x01000000 << pDevExt->Channel);

    WRITE_PORT_ULONG(ULongToPtr(pDevExt->Bar2), new_fcr);

    return STATUS_SUCCESS;
}

UCHAR FsccGetFrev(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    return READ_PORT_ULONG(ULongToPtr(pDevExt->Bar0 + VSTR_OFFSET + (pDevExt->Channel * 0x80))) & 0x000000ff;
}

UCHAR FsccGetPrev(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    return (READ_PORT_ULONG(ULongToPtr(pDevExt->Bar0 + VSTR_OFFSET + (pDevExt->Channel * 0x80))) & 0x0000ff00) >> 8;
}

UINT16 FsccGetPdev(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    return (READ_PORT_ULONG(ULongToPtr(pDevExt->Bar0 + VSTR_OFFSET + (pDevExt->Channel * 0x80))) & 0xffff0000) >> 16;
}

void FastcomGetFixedBaudRate(SERIAL_DEVICE_EXTENSION *pDevExt, int *rate)
{
    *rate = pDevExt->FixedBaudRate;
}

void FastcomSetFixedBaudRate(SERIAL_DEVICE_EXTENSION *pDevExt, int rate)
{
    SerialDbgPrintEx(TRACE_LEVEL_INFORMATION, DBG_PNP,
                     "Fixed Baud Rate = %i\n", rate);

    pDevExt->FixedBaudRate = rate;
}

void FastcomEnableFixedBaudRate(SERIAL_DEVICE_EXTENSION *pDevExt, unsigned rate)
{
    FastcomSetFixedBaudRate(pDevExt, rate);
}

void FastcomDisableFixedBaudRate(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    FastcomSetFixedBaudRate(pDevExt, -1);
}

void FastcomInitGpio(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
        switch (pDevExt->DeviceID) {
        case FC_422_2_PCI_335_ID:
        case FC_422_4_PCI_335_ID:
            /* Switch GPIO pins to outputs */
            pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOSEL_OFFSET, 0x00);
            break;

        case FC_232_4_PCI_335_ID:
        case FC_232_8_PCI_335_ID:
            pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOSEL_OFFSET, 0xc0);
            pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOINV_OFFSET, 0xc0);
            break;
        }
        break;

    case CARD_TYPE_PCIe:
         /* Switch GPIO pins to outputs */
        pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOSEL_OFFSET, 0x00);
        pDevExt->SerialWriteUChar(pDevExt->Controller + MPIOSELH_OFFSET, 0x00);
        break;

    default:
        break;
    }
}

void FastcomInitTriggers(SERIAL_DEVICE_EXTENSION *pDevExt)
{
    /* Enable programmable trigger levels */
    switch (FastcomGetCardType(pDevExt)) {
    case CARD_TYPE_PCI:
    case CARD_TYPE_PCIe: {
            unsigned char current_fctr, new_fctr;

            current_fctr = pDevExt->SerialReadUChar(pDevExt->Controller + UART_EXAR_FCTR);

            new_fctr = current_fctr | 0xc0; /* Enable programmable triggers */

            pDevExt->SerialWriteUChar(pDevExt->Controller + FCR_OFFSET, 0x01); /* Enable TX & RX FIFO's */
            pDevExt->SerialWriteUChar(pDevExt->Controller + UART_EXAR_FCTR, new_fctr);
        }
        break;

    case CARD_TYPE_FSCC:
        pDevExt->SerialWriteUChar(pDevExt->Controller + FCR_OFFSET, 0x01); /* Enable FIFO (combined with enhanced enables 950 mode) */

        pDevExt->SerialWriteUChar(pDevExt->Controller + LCR_OFFSET, 0xbf); /* Set to 0xbf to access 650 registers */
        pDevExt->SerialWriteUChar(pDevExt->Controller + EFR_OFFSET, 0x10); /* Enable enhanced mode */

        /* Temporarily disable 950 trigger levels due to either interrupts not firing or not being handled correctly */
#if 0
        pDevExt->SerialWriteUChar(pDevExt->Controller + LCR_OFFSET, 0x00); /* Ensure last LCR value is not 0xbf */
        pDevExt->SerialWriteUChar(pDevExt->Controller + SPR_OFFSET, ACR_OFFSET); /* To allow access to ACR */
        pDevExt->ACR = 0x20;
        pDevExt->SerialWriteUChar(pDevExt->Controller + ICR_OFFSET, pDevExt->ACR); /* Enable 950 trigger to ACR through ICR */
#endif
        pDevExt->ACR = 0x00;
        break;

    default:
        break;
    }
}

void SerialFcInit(
    IN PSERIAL_DEVICE_EXTENSION pDevExt,
    IN PCONFIG_DATA PConfigData)
{
    FastcomInitGpio(pDevExt);
    FastcomInitTriggers(pDevExt);

    if (PConfigData) {
        FastcomSetClockRate(pDevExt, PConfigData->ClockRate);
        FastcomSetRS485(pDevExt, (BOOLEAN)PConfigData->RS485);
        FastcomSetSampleRate(pDevExt, PConfigData->SampleRate);
        FastcomSetTxTrigger(pDevExt, PConfigData->TxTrigger);
        FastcomSetRxTrigger(pDevExt, PConfigData->RxTrigger);
        FastcomSetTermination(pDevExt, (BOOLEAN)PConfigData->Termination);
        FastcomSetEchoCancel(pDevExt, (BOOLEAN)PConfigData->EchoCancel);
        FastcomSetIsochronous(pDevExt, PConfigData->Isochronous);
        FastcomSetFrameLength(pDevExt, PConfigData->FrameLength);
        FastcomSet9Bit(pDevExt, (BOOLEAN)PConfigData->NineBit);
        FastcomSetFixedBaudRate(pDevExt, PConfigData->FixedBaudRate);
    }
    else {
        FastcomSetClockRate(pDevExt, pDevExt->ClockRate);
        FastcomSetRS485(pDevExt, pDevExt->RS485);
        FastcomSetSampleRate(pDevExt, pDevExt->SampleRate);
        FastcomSetTxTrigger(pDevExt, pDevExt->TxTrigger);
        FastcomSetRxTrigger(pDevExt, pDevExt->RxTrigger);
        FastcomSetTermination(pDevExt, pDevExt->Termination);
        FastcomSetEchoCancel(pDevExt, pDevExt->EchoCancel);
        FastcomSetIsochronous(pDevExt, pDevExt->Isochronous);
        FastcomSetFrameLength(pDevExt, pDevExt->FrameLength);
        FastcomSet9Bit(pDevExt, pDevExt->NineBit);
        FastcomSetFixedBaudRate(pDevExt, pDevExt->FixedBaudRate);
    }
}