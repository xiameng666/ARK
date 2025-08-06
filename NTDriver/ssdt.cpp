#include "ssdt.h"
#include <ntimage.h>

// Win7 SSDT 函数名数组
const char* WIN7_SSDT_FUNCTIONS[] = {
      "NtMapUserPhysicalPagesScatter",
      "NtWaitForSingleObject",
      "NtCallbackReturn",
      "NtReadFile",
      "NtDeviceIoControlFile",
      "NtWriteFile",
      "NtRemoveIoCompletion",
      "NtReleaseSemaphore",
      "NtReplyWaitReceivePort",
      "NtReplyPort",
      "NtSetInformationThread",
      "NtSetEvent",
      "NtClose",
      "NtQueryObject",
      "NtQueryInformationFile",
      "NtOpenKey",
      "NtEnumerateValueKey",
      "NtFindAtom",
      "NtQueryDefaultLocale",
      "NtQueryKey",
      "NtQueryValueKey",
      "NtAllocateVirtualMemory",
      "NtQueryInformationProcess",
      "NtWaitForMultipleObjects32",
      "NtWriteFileGather",
      "NtSetInformationProcess",
      "NtCreateKey",
      "NtFreeVirtualMemory",
      "NtImpersonateClientOfPort",
      "NtReleaseMutant",
      "NtQueryInformationToken",
      "NtRequestWaitReplyPort",
      "NtQueryVirtualMemory",
      "NtOpenThreadToken",
      "NtQueryInformationThread",
      "NtOpenProcess",
      "NtSetInformationFile",
      "NtMapViewOfSection",
      "NtAccessCheckAndAuditAlarm",
      "NtUnmapViewOfSection",
      "NtReplyWaitReceivePortEx",
      "NtTerminateProcess",
      "NtSetEventBoostPriority",
      "NtReadFileScatter",
      "NtOpenThreadTokenEx",
      "NtOpenProcessTokenEx",
      "NtQueryPerformanceCounter",
      "NtEnumerateKey",
      "NtOpenFile",
      "NtDelayExecution",
      "NtQueryDirectoryFile",
      "NtQuerySystemInformation",
      "NtOpenSection",
      "NtQueryTimer",
      "NtFsControlFile",
      "NtWriteVirtualMemory",
      "NtCloseObjectAuditAlarm",
      "NtDuplicateObject",
      "NtQueryAttributesFile",
      "NtClearEvent",
      "NtReadVirtualMemory",
      "NtOpenEvent",
      "NtAdjustPrivilegesToken",
      "NtDuplicateToken",
      "NtContinue",
      "NtQueryDefaultUILanguage",
      "NtQueueApcThread",
      "NtYieldExecution",
      "NtAddAtom",
      "NtCreateEvent",
      "NtQueryVolumeInformationFile",
      "NtCreateSection",
      "NtFlushBuffersFile",
      "NtApphelpCacheControl",
      "NtCreateProcessEx",
      "NtCreateThread",
      "NtIsProcessInJob",
      "NtProtectVirtualMemory",
      "NtQuerySection",
      "NtResumeThread",
      "NtTerminateThread",
      "NtReadRequestData",
      "NtCreateFile",
      "NtQueryEvent",
      "NtWriteRequestData",
      "NtOpenDirectoryObject",
      "NtAccessCheckByTypeAndAuditAlarm",
      "NtQuerySystemTime",
      "NtWaitForMultipleObjects",
      "NtSetInformationObject",
      "NtCancelIoFile",
      "NtTraceEvent",
      "NtPowerInformation",
      "NtSetValueKey",
      "NtCancelTimer",
      "NtSetTimer",
      "NtAcceptConnectPort",
      "NtAccessCheck",
      "NtAccessCheckByType",
      "NtAccessCheckByTypeResultList",
      "NtAccessCheckByTypeResultListAndAuditAlarm",
      "NtAccessCheckByTypeResultListAndAuditAlarmByHandle",
      "NtAddBootEntry",
      "NtAddDriverEntry",
      "NtAdjustGroupsToken",
      "NtAlertResumeThread",
      "NtAlertThread",
      "NtAllocateLocallyUniqueId",
      "NtAllocateReserveObject",
      "NtAllocateUserPhysicalPages",
      "NtAllocateUuids",
      "NtAlpcAcceptConnectPort",
      "NtAlpcCancelMessage",
      "NtAlpcConnectPort",
      "NtAlpcCreatePort",
      "NtAlpcCreatePortSection",
      "NtAlpcCreateResourceReserve",
      "NtAlpcCreateSectionView",
      "NtAlpcCreateSecurityContext",
      "NtAlpcDeletePortSection",
      "NtAlpcDeleteResourceReserve",
      "NtAlpcDeleteSectionView",
      "NtAlpcDeleteSecurityContext",
      "NtAlpcDisconnectPort",
      "NtAlpcImpersonateClientOfPort",
      "NtAlpcOpenSenderProcess",
      "NtAlpcOpenSenderThread",
      "NtAlpcQueryInformation",
      "NtAlpcQueryInformationMessage",
      "NtAlpcRevokeSecurityContext",
      "NtAlpcSendWaitReceivePort",
      "NtAlpcSetInformation",
      "NtAreMappedFilesTheSame",
      "NtAssignProcessToJobObject",
      "NtCancelIoFileEx",
      "NtCancelSynchronousIoFile",
      "NtCommitComplete",
      "NtCommitEnlistment",
      "NtCommitTransaction",
      "NtCompactKeys",
      "NtCompareTokens",
      "CmBcbCacheTrimNotification",
      "NtCompressKey",
      "NtConnectPort",
      "NtCreateDebugObject",
      "NtCreateDirectoryObject",
      "NtCreateEnlistment",
      "NtCreateEventPair",
      "NtCreateIoCompletion",
      "NtCreateJobObject",
      "NtCreateJobSet",
      "NtCreateKeyTransacted",
      "NtCreateKeyedEvent",
      "NtCreateMailslotFile",
      "NtCreateMutant",
      "NtCreateNamedPipeFile",
      "NtCreatePagingFile",
      "NtCreatePort",
      "NtCreatePrivateNamespace",
      "NtCreateProcess",
      "NtCreateProfile",
      "NtCreateProfileEx",
      "NtCreateResourceManager",
      "NtCreateSemaphore",
      "NtCreateSymbolicLinkObject",
      "NtCreateThreadEx",
      "NtCreateTimer",
      "NtCreateToken",
      "NtCreateTransaction",
      "NtCreateTransactionManager",
      "NtCreateUserProcess",
      "NtCreateWaitablePort",
      "NtCreateWorkerFactory",
      "NtDebugActiveProcess",
      "NtDebugContinue",
      "NtDeleteAtom",
      "NtDeleteBootEntry",
      "NtDeleteDriverEntry",
      "NtDeleteFile",
      "NtDeleteKey",
      "NtDeleteObjectAuditAlarm",
      "NtDeletePrivateNamespace",
      "NtDeleteValueKey",
      "NtDisableLastKnownGood",
      "NtDisplayString",
      "NtDrawText",
      "NtEnableLastKnownGood",
      "NtEnumerateBootEntries",
      "NtEnumerateDriverEntries",
      "NtEnumerateSystemEnvironmentValuesEx",
      "NtEnumerateTransactionObject",
      "NtExtendSection",
      "NtFilterToken",
      "NtFlushInstallUILanguage",
      "NtFlushInstructionCache",
      "NtFlushKey",
      "NtFlushProcessWriteBuffers",
      "NtFlushVirtualMemory",
      "NtFlushWriteBuffer",
      "NtFreeUserPhysicalPages",
      "NtFreezeRegistry",
      "NtFreezeTransactions",
      "NtGetContextThread",
      "NtGetCurrentProcessorNumber",
      "NtGetDevicePowerState",
      "NtGetMUIRegistryInfo",
      "NtGetNextProcess",
      "NtGetNextThread",
      "NtGetNlsSectionPtr",
      "NtGetNotificationResourceManager",
      "NtGetPlugPlayEvent",
      "NtGetWriteWatch",
      "NtImpersonateAnonymousToken",
      "NtImpersonateThread",
      "NtInitializeNlsFiles",
      "NtInitializeRegistry",
      "NtInitiatePowerAction",
      "NtIsSystemResumeAutomatic",
      "NtIsUILanguageComitted",
      "NtListenPort",
      "NtLoadDriver",
      "NtLoadKey",
      "NtLoadKey2",
      "NtLoadKeyEx",
      "NtLockFile",
      "NtLockProductActivationKeys",
      "NtLockRegistryKey",
      "NtLockVirtualMemory",
      "NtMakePermanentObject",
      "NtMakeTemporaryObject",
      "NtMapCMFModule",
      "NtMapUserPhysicalPages",
      "NtModifyBootEntry",
      "NtModifyDriverEntry",
      "NtNotifyChangeDirectoryFile",
      "NtNotifyChangeKey",
      "NtNotifyChangeMultipleKeys",
      "NtNotifyChangeSession",
      "NtOpenEnlistment",
      "NtOpenEventPair",
      "NtOpenIoCompletion",
      "NtOpenJobObject",
      "NtOpenKeyEx",
      "NtOpenKeyTransacted",
      "NtOpenKeyTransactedEx",
      "NtOpenKeyedEvent",
      "NtOpenMutant",
      "NtOpenObjectAuditAlarm",
      "NtOpenPrivateNamespace",
      "NtOpenProcessToken",
      "NtOpenResourceManager",
      "NtOpenSemaphore",
      "NtOpenSession",
      "NtOpenSymbolicLinkObject",
      "NtOpenThread",
      "NtOpenTimer",
      "NtOpenTransaction",
      "NtOpenTransactionManager",
      "NtPlugPlayControl",
      "NtPrePrepareComplete",
      "NtPrePrepareEnlistment",
      "NtPrepareComplete",
      "NtPrepareEnlistment",
      "NtPrivilegeCheck",
      "NtPrivilegeObjectAuditAlarm",
      "NtPrivilegedServiceAuditAlarm",
      "NtPropagationComplete",
      "NtPropagationFailed",
      "NtPulseEvent",
      "NtQueryBootEntryOrder",
      "NtQueryBootOptions",
      "NtQueryDebugFilterState",
      "NtQueryDirectoryObject",
      "NtQueryDriverEntryOrder",
      "NtQueryEaFile",
      "NtQueryFullAttributesFile",
      "NtQueryInformationAtom",
      "NtQueryInformationEnlistment",
      "NtQueryInformationJobObject",
      "NtQueryInformationPort",
      "NtQueryInformationResourceManager",
      "NtQueryInformationTransaction",
      "NtQueryInformationTransactionManager",
      "NtQueryInformationWorkerFactory",
      "NtQueryInstallUILanguage",
      "NtQueryIntervalProfile",
      "NtQueryIoCompletion",
      "NtQueryLicenseValue",
      "NtQueryMultipleValueKey",
      "NtQueryMutant",
      "NtQueryOpenSubKeys",
      "NtQueryOpenSubKeysEx",
      "NtQueryPortInformationProcess",
      "NtQueryQuotaInformationFile",
      "NtQuerySecurityAttributesToken",
      "NtQuerySecurityObject",
      "NtQuerySemaphore",
      "NtQuerySymbolicLinkObject",
      "NtQuerySystemEnvironmentValue",
      "NtQuerySystemEnvironmentValueEx",
      "NtQuerySystemInformationEx",
      "NtQueryTimerResolution",
      "NtQueueApcThreadEx",
      "NtRaiseException",
      "NtRaiseHardError",
      "NtReadOnlyEnlistment",
      "NtRecoverEnlistment",
      "NtRecoverResourceManager",
      "NtRecoverTransactionManager",
      "NtRegisterProtocolAddressInformation",
      "NtRegisterThreadTerminatePort",
      "NtReleaseKeyedEvent",
      "NtReleaseWorkerFactoryWorker",
      "NtRemoveIoCompletionEx",
      "NtRemoveProcessDebug",
      "NtRenameKey",
      "NtRenameTransactionManager",
      "NtReplaceKey",
      "NtReplacePartitionUnit",
      "NtReplyWaitReplyPort",
      "NtRequestPort",
      "NtResetEvent",
      "NtResetWriteWatch",
      "NtRestoreKey",
      "NtResumeProcess",
      "NtRollbackComplete",
      "NtRollbackEnlistment",
      "NtRollbackTransaction",
      "NtRollforwardTransactionManager",
      "NtSaveKey",
      "NtSaveKeyEx",
      "NtSaveMergedKeys",
      "NtSecureConnectPort",
      "NtSerializeBoot",
      "NtSetBootEntryOrder",
      "NtSetBootOptions",
      "NtSetContextThread",
      "NtSetDebugFilterState",
      "NtSetDefaultHardErrorPort",
      "NtSetDefaultLocale",
      "NtSetDefaultUILanguage",
      "NtSetDriverEntryOrder",
      "NtSetEaFile",
      "NtSetHighEventPair",
      "NtSetHighWaitLowEventPair",
      "NtSetInformationDebugObject",
      "NtSetInformationEnlistment",
      "NtSetInformationJobObject",
      "NtSetInformationKey",
      "NtSetInformationResourceManager",
      "NtSetInformationToken",
      "NtSetInformationTransaction",
      "NtSetInformationTransactionManager",
      "NtSetInformationWorkerFactory",
      "NtSetIntervalProfile",
      "NtSetIoCompletion",
      "NtSetIoCompletionEx",
      "CcTestControl",
      "NtSetLowEventPair",
      "NtSetLowWaitHighEventPair",
      "NtSetQuotaInformationFile",
      "NtSetSecurityObject",
      "NtSetSystemEnvironmentValue",
      "NtSetSystemEnvironmentValueEx",
      "NtSetSystemInformation",
      "NtSetSystemPowerState",
      "NtSetSystemTime",
      "NtSetThreadExecutionState",
      "NtSetTimerEx",
      "NtSetTimerResolution",
      "NtSetUuidSeed",
      "NtSetVolumeInformationFile",
      "NtShutdownSystem",
      "NtShutdownWorkerFactory",
      "NtSignalAndWaitForSingleObject",
      "NtSinglePhaseReject",
      "NtStartProfile",
      "NtStopProfile",
      "NtSuspendProcess",
      "NtSuspendThread",
      "NtSystemDebugControl",
      "NtTerminateJobObject",
      "NtTestAlert",
      "NtThawRegistry",
      "NtThawTransactions",
      "NtTraceControl",
      "NtTranslateFilePath",
      "NtUmsThreadYield",
      "NtUnloadDriver",
      "NtUnloadKey",
      "NtUnloadKey2",
      "NtUnloadKeyEx",
      "NtUnlockFile",
      "NtUnlockVirtualMemory",
      "NtVdmControl",
      "NtWaitForDebugEvent",
      "NtWaitForKeyedEvent",
      "NtWaitForWorkViaWorkerFactory",
      "NtWaitHighEventPair",
      "NtWaitLowEventPair",
      "NtWorkerFactoryWorkerReady"
};

// Win7 ShadowSSDT 函数名数组
const char* WIN7_SHADOW_SSDT_FUNCTIONS[] = {
      "NtUserGetThreadState",
      "NtUserPeekMessage",
      "NtUserCallOneParam",
      "NtUserGetKeyState",
      "NtUserInvalidateRect",
      "NtUserCallNoParam",
      "NtUserGetMessage",
      "NtUserMessageCall",
      "NtGdiBitBlt",
      "NtGdiGetCharSet",
      "NtUserGetDC",
      "NtGdiSelectBitmap",
      "NtUserWaitMessage",
      "NtUserTranslateMessage",
      "NtUserGetProp",
      "NtUserPostMessage",
      "NtUserQueryWindow",
      "NtUserTranslateAccelerator",
      "NtGdiFlush",
      "NtUserRedrawWindow",
      "NtUserWindowFromPoint",
      "NtUserCallMsgFilter",
      "NtUserValidateTimerCallback",
      "NtUserBeginPaint",
      "NtUserSetTimer",
      "NtUserEndPaint",
      "NtUserSetCursor",
      "NtUserKillTimer",
      "NtUserBuildHwndList",
      "NtUserSelectPalette",
      "NtUserCallNextHookEx",
      "NtUserHideCaret",
      "NtGdiIntersectClipRect",
      "NtUserCallHwndLock",
      "NtUserGetProcessWindowStation",
      "NtGdiDeleteObjectApp",
      "NtUserSetWindowPos",
      "NtUserShowCaret",
      "NtUserEndDeferWindowPosEx",
      "NtUserCallHwndParamLock",
      "NtUserVkKeyScanEx",
      "NtGdiSetDIBitsToDeviceInternal",
      "NtUserCallTwoParam",
      "NtGdiGetRandomRgn",
      "NtUserCopyAcceleratorTable",
      "NtUserNotifyWinEvent",
      "NtGdiExtSelectClipRgn",
      "NtUserIsClipboardFormatAvailable",
      "NtUserSetScrollInfo",
      "NtGdiStretchBlt",
      "NtUserCreateCaret",
      "NtGdiRectVisible",
      "NtGdiCombineRgn",
      "NtGdiGetDCObject",
      "NtUserDispatchMessage",
      "NtUserRegisterWindowMessage",
      "NtGdiExtTextOutW",
      "NtGdiSelectFont",
      "NtGdiRestoreDC",
      "NtGdiSaveDC",
      "NtUserGetForegroundWindow",
      "NtUserShowScrollBar",
      "NtUserFindExistingCursorIcon",
      "NtGdiGetDCDword",
      "NtGdiGetRegionData",
      "NtGdiLineTo",
      "NtUserSystemParametersInfo",
      "NtGdiGetAppClipBox",
      "NtUserGetAsyncKeyState",
      "NtUserGetCPD",
      "NtUserRemoveProp",
      "NtGdiDoPalette",
      "NtGdiPolyPolyDraw",
      "NtUserSetCapture",
      "NtUserEnumDisplayMonitors",
      "NtGdiCreateCompatibleBitmap",
      "NtUserSetProp",
      "NtGdiGetTextCharsetInfo",
      "NtUserSBGetParms",
      "NtUserGetIconInfo",
      "NtUserExcludeUpdateRgn",
      "NtUserSetFocus",
      "NtGdiExtGetObjectW",
      "NtUserDeferWindowPos",
      "NtUserGetUpdateRect",
      "NtGdiCreateCompatibleDC",
      "NtUserGetClipboardSequenceNumber",
      "NtGdiCreatePen",
      "NtUserShowWindow",
      "NtUserGetKeyboardLayoutList",
      "NtGdiPatBlt",
      "NtUserMapVirtualKeyEx",
      "NtUserSetWindowLong",
      "NtGdiHfontCreate",
      "NtUserMoveWindow",
      "NtUserPostThreadMessage",
      "NtUserDrawIconEx",
      "NtUserGetSystemMenu",
      "NtGdiDrawStream",
      "NtUserInternalGetWindowText",
      "NtUserGetWindowDC",
      "NtGdiD3dDrawPrimitives2",
      "NtGdiInvertRgn",
      "NtGdiGetRgnBox",
      "NtGdiGetAndSetDCDword",
      "NtGdiMaskBlt",
      "NtGdiGetWidthTable",
      "NtUserScrollDC",
      "NtUserGetObjectInformation",
      "NtGdiCreateBitmap",
      "NtUserFindWindowEx",
      "NtGdiPolyPatBlt",
      "NtUserUnhookWindowsHookEx",
      "NtGdiGetNearestColor",
      "NtGdiTransformPoints",
      "NtGdiGetDCPoint",
      "NtGdiCreateDIBBrush",
      "NtGdiGetTextMetricsW",
      "NtUserCreateWindowEx",
      "NtUserSetParent",
      "NtUserGetKeyboardState",
      "NtUserToUnicodeEx",
      "NtUserGetControlBrush",
      "NtUserGetClassName",
      "NtGdiAlphaBlend",
      "NtGdiDdBlt",
      "NtGdiOffsetRgn",
      "NtUserDefSetText",
      "NtGdiGetTextFaceW",
      "NtGdiStretchDIBitsInternal",
      "NtUserSendInput",
      "NtUserGetThreadDesktop",
      "NtGdiCreateRectRgn",
      "NtGdiGetDIBitsInternal",
      "NtUserGetUpdateRgn",
      "NtGdiDeleteClientObj",
      "NtUserGetIconSize",
      "NtUserFillWindow",
      "NtGdiExtCreateRegion",
      "NtGdiComputeXformCoefficients",
      "NtUserSetWindowsHookEx",
      "NtUserNotifyProcessCreate",
      "NtGdiUnrealizeObject",
      "NtUserGetTitleBarInfo",
      "NtGdiRectangle",
      "NtUserSetThreadDesktop",
      "NtUserGetDCEx",
      "NtUserGetScrollBarInfo",
      "NtGdiGetTextExtent",
      "NtUserSetWindowFNID",
      "NtGdiSetLayout",
      "NtUserCalcMenuBar",
      "NtUserThunkedMenuItemInfo",
      "NtGdiExcludeClipRect",
      "NtGdiCreateDIBSection",
      "NtGdiGetDCforBitmap",
      "NtUserDestroyCursor",
      "NtUserDestroyWindow",
      "NtUserCallHwndParam",
      "NtGdiCreateDIBitmapInternal",
      "NtUserOpenWindowStation",
      "NtGdiDdDeleteSurfaceObject",
      "NtGdiDdCanCreateSurface",
      "NtGdiDdCreateSurface",
      "NtUserSetCursorIconData",
      "NtGdiDdDestroySurface",
      "NtUserCloseDesktop",
      "NtUserOpenDesktop",
      "NtUserSetProcessWindowStation",
      "NtUserGetAtomName",
      "NtGdiDdResetVisrgn",
      "NtGdiExtCreatePen",
      "NtGdiCreatePaletteInternal",
      "NtGdiSetBrushOrg",
      "NtUserBuildNameList",
      "NtGdiSetPixel",
      "NtUserRegisterClassExWOW",
      "NtGdiCreatePatternBrushInternal",
      "NtUserGetAncestor",
      "NtGdiGetOutlineTextMetricsInternalW",
      "NtGdiSetBitmapBits",
      "NtUserCloseWindowStation",
      "NtUserGetDoubleClickTime",
      "NtUserEnableScrollBar",
      "NtGdiCreateSolidBrush",
      "NtUserGetClassInfoEx",
      "NtGdiCreateClientObj",
      "NtUserUnregisterClass",
      "NtUserDeleteMenu",
      "NtGdiRectInRegion",
      "NtUserScrollWindowEx",
      "NtGdiGetPixel",
      "NtUserSetClassLong",
      "NtUserGetMenuBarInfo",
      "NtGdiDdCreateSurfaceEx",
      "NtGdiDdCreateSurfaceObject",
      "NtGdiGetNearestPaletteIndex",
      "NtGdiDdLockD3D",
      "NtGdiDdUnlockD3D",
      "NtGdiGetCharWidthW",
      "NtUserInvalidateRgn",
      "NtUserGetClipboardOwner",
      "NtUserSetWindowRgn",
      "NtUserBitBltSysBmp",
      "NtGdiGetCharWidthInfo",
      "NtUserValidateRect",
      "NtUserCloseClipboard",
      "NtUserOpenClipboard",
      "NtGdiGetStockObject",
      "NtUserSetClipboardData",
      "NtUserEnableMenuItem",
      "NtUserAlterWindowStyle",
      "NtGdiFillRgn",
      "NtUserGetWindowPlacement",
      "NtGdiModifyWorldTransform",
      "NtGdiGetFontData",
      "NtUserGetOpenClipboardWindow",
      "NtUserSetThreadState",
      "NtGdiOpenDCW",
      "NtUserTrackMouseEvent",
      "NtGdiGetTransform",
      "NtUserDestroyMenu",
      "NtGdiGetBitmapBits",
      "NtUserConsoleControl",
      "NtUserSetActiveWindow",
      "NtUserSetInformationThread",
      "NtUserSetWindowPlacement",
      "NtUserGetControlColor",
      "NtGdiSetMetaRgn",
      "NtGdiSetMiterLimit",
      "NtGdiSetVirtualResolution",
      "NtGdiGetRasterizerCaps",
      "NtUserSetWindowWord",
      "NtUserGetClipboardFormatName",
      "NtUserRealInternalGetMessage",
      "NtUserCreateLocalMemHandle",
      "NtUserAttachThreadInput",
      "NtGdiCreateHalftonePalette",
      "NtUserPaintMenuBar",
      "NtUserSetKeyboardState",
      "NtGdiCombineTransform",
      "NtUserCreateAcceleratorTable",
      "NtUserGetCursorFrameInfo",
      "NtUserGetAltTabInfo",
      "NtUserGetCaretBlinkTime",
      "NtGdiQueryFontAssocInfo",
      "NtUserProcessConnect",
      "NtUserEnumDisplayDevices",
      "NtUserEmptyClipboard",
      "NtUserGetClipboardData",
      "NtUserRemoveMenu",
      "NtGdiSetBoundsRect",
      "NtGdiGetBitmapDimension",
      "NtUserConvertMemHandle",
      "NtUserDestroyAcceleratorTable",
      "NtUserGetGUIThreadInfo",
      "NtGdiCloseFigure",
      "NtUserSetWindowsHookAW",
      "NtUserSetMenuDefaultItem",
      "NtUserCheckMenuItem",
      "NtUserSetWinEventHook",
      "NtUserUnhookWinEvent",
      "NtUserLockWindowUpdate",
      "NtUserSetSystemMenu",
      "NtUserThunkedMenuInfo",
      "NtGdiBeginPath",
      "NtGdiEndPath",
      "NtGdiFillPath",
      "NtUserCallHwnd",
      "NtUserDdeInitialize",
      "NtUserModifyUserStartupInfoFlags",
      "NtUserCountClipboardFormats",
      "NtGdiAddFontMemResourceEx",
      "NtGdiEqualRgn",
      "NtGdiGetSystemPaletteUse",
      "NtGdiRemoveFontMemResourceEx",
      "NtUserEnumDisplaySettings",
      "NtUserPaintDesktop",
      "NtGdiExtEscape",
      "NtGdiSetBitmapDimension",
      "NtGdiSetFontEnumeration",
      "NtUserChangeClipboardChain",
      "NtUserSetClipboardViewer",
      "NtUserShowWindowAsync",
      "NtGdiCreateColorSpace",
      "NtGdiDeleteColorSpace",
      "NtUserActivateKeyboardLayout",
      "NtGdiAbortDoc",
      "NtGdiAbortPath",
      "NtGdiAddEmbFontToDC",
      "NtGdiAddFontResourceW",
      "NtGdiAddRemoteFontToDC",
      "NtGdiAddRemoteMMInstanceToDC",
      "NtGdiAngleArc",
      "NtGdiAnyLinkedFonts",
      "NtGdiArcInternal",
      "NtGdiBRUSHOBJ_DeleteRbrush",
      "NtGdiBRUSHOBJ_hGetColorTransform",
      "NtGdiBRUSHOBJ_pvAllocRbrush",
      "NtGdiBRUSHOBJ_pvGetRbrush",
      "NtGdiBRUSHOBJ_ulGetBrushColor",
      "NtGdiBeginGdiRendering",
      "NtGdiCLIPOBJ_bEnum",
      "NtGdiCLIPOBJ_cEnumStart",
      "NtGdiCLIPOBJ_ppoGetPath",
      "NtGdiCancelDC",
      "NtGdiChangeGhostFont",
      "NtGdiCheckBitmapBits",
      "NtGdiClearBitmapAttributes",
      "NtGdiClearBrushAttributes",
      "NtGdiColorCorrectPalette",
      "NtGdiConfigureOPMProtectedOutput",
      "NtGdiConvertMetafileRect",
      "NtGdiCreateBitmapFromDxSurface",
      "NtGdiCreateColorTransform",
      "NtGdiCreateEllipticRgn",
      "NtGdiCreateHatchBrushInternal",
      "NtGdiCreateMetafileDC",
      "NtGdiCreateOPMProtectedOutputs",
      "NtGdiCreateRoundRectRgn",
      "NtGdiCreateServerMetaFile",
      "NtGdiD3dContextCreate",
      "NtGdiD3dContextDestroy",
      "NtGdiD3dContextDestroyAll",
      "NtGdiD3dValidateTextureStageState",
      "NtGdiDDCCIGetCapabilitiesString",
      "NtGdiDDCCIGetCapabilitiesStringLength",
      "NtGdiDDCCIGetTimingReport",
      "NtGdiDDCCIGetVCPFeature",
      "NtGdiDDCCISaveCurrentSettings",
      "NtGdiDDCCISetVCPFeature",
      "NtGdiDdAddAttachedSurface",
      "NtGdiDdAlphaBlt",
      "NtGdiDdAttachSurface",
      "NtGdiDdBeginMoCompFrame",
      "NtGdiDdCanCreateD3DBuffer",
      "NtGdiDdColorControl",
      "NtGdiDdCreateD3DBuffer",
      "NtGdiDdCreateDirectDrawObject",
      "NtGdiDdCreateFullscreenSprite",
      "NtGdiDdCreateMoComp",
      "NtGdiDdDDIAcquireKeyedMutex",
      "NtGdiDdDDICheckExclusiveOwnership",
      "NtGdiDdDDICheckMonitorPowerState",
      "NtGdiDdDDICheckOcclusion",
      "NtGdiDdDDICheckSharedResourceAccess",
      "NtGdiDdDDICheckVidPnExclusiveOwnership",
      "NtGdiDdDDICloseAdapter",
      "NtGdiDdDDIConfigureSharedResource",
      "NtGdiDdDDICreateAllocation",
      "NtGdiDdDDICreateContext",
      "NtGdiDdDDICreateDCFromMemory",
      "NtGdiDdDDICreateDevice",
      "NtGdiDdDDICreateKeyedMutex",
      "NtGdiDdDDICreateOverlay",
      "NtGdiDdDDICreateSynchronizationObject",
      "NtGdiDdDDIDestroyAllocation",
      "NtGdiDdDDIDestroyContext",
      "NtGdiDdDDIDestroyDCFromMemory",
      "NtGdiDdDDIDestroyDevice",
      "NtGdiDdDDIDestroyKeyedMutex",
      "NtGdiDdDDIDestroyOverlay",
      "NtGdiDdDDIDestroySynchronizationObject",
      "NtGdiDdDDIEscape",
      "NtGdiDdDDIFlipOverlay",
      "NtGdiDdDDIGetContextSchedulingPriority",
      "NtGdiDdDDIGetDeviceState",
      "NtGdiDdDDIGetDisplayModeList",
      "NtGdiDdDDIGetMultisampleMethodList",
      "NtGdiDdDDIGetOverlayState",
      "NtGdiDdDDIGetPresentHistory",
      "NtGdiDdDDIGetPresentQueueEvent",
      "NtGdiDdDDIGetProcessSchedulingPriorityClass",
      "NtGdiDdDDIGetRuntimeData",
      "NtGdiDdDDIGetScanLine",
      "NtGdiDdDDIGetSharedPrimaryHandle",
      "NtGdiDdDDIInvalidateActiveVidPn",
      "NtGdiDdDDILock",
      "NtGdiDdDDIOpenAdapterFromDeviceName",
      "NtGdiDdDDIOpenAdapterFromHdc",
      "NtGdiDdDDIOpenKeyedMutex",
      "NtGdiDdDDIOpenResource",
      "NtGdiDdDDIOpenSynchronizationObject",
      "NtGdiDdDDIPollDisplayChildren",
      "NtGdiDdDDIPresent",
      "NtGdiDdDDIQueryAdapterInfo",
      "NtGdiDdDDIQueryAllocationResidency",
      "NtGdiDdDDIQueryResourceInfo",
      "NtGdiDdDDIQueryStatistics",
      "NtGdiDdDDIReleaseKeyedMutex",
      "NtGdiDdDDIReleaseProcessVidPnSourceOwners",
      "NtGdiDdDDIRender",
      "NtGdiDdDDISetAllocationPriority",
      "NtGdiDdDDISetContextSchedulingPriority",
      "NtGdiDdDDISetDisplayMode",
      "NtGdiDdDDISetDisplayPrivateDriverFormat",
      "NtGdiDdDDISetGammaRamp",
      "NtGdiDdDDISetProcessSchedulingPriorityClass",
      "NtGdiDdDDISetQueuedLimit",
      "NtGdiDdDDISetVidPnSourceOwner",
      "NtGdiDdDDISharedPrimaryLockNotification",
      "NtGdiDdDDISharedPrimaryUnLockNotification",
      "NtGdiDdDDISignalSynchronizationObject",
      "NtGdiDdDDIUnlock",
      "NtGdiDdDDIUpdateOverlay",
      "NtGdiDdDDIWaitForIdle",
      "NtGdiDdDDIWaitForSynchronizationObject",
      "NtGdiDdDDIWaitForVerticalBlankEvent",
      "NtGdiDdDeleteDirectDrawObject",
      "NtGdiDdDestroyD3DBuffer",
      "NtGdiDdDestroyFullscreenSprite",
      "NtGdiDdDestroyMoComp",
      "NtGdiDdEndMoCompFrame",
      "NtGdiDdFlip",
      "NtGdiDdFlipToGDISurface",
      "NtGdiDdGetAvailDriverMemory",
      "NtGdiDdGetBltStatus",
      "NtGdiDdGetDC",
      "NtGdiDdGetDriverInfo",
      "NtGdiDdGetDriverState",
      "NtGdiDdGetDxHandle",
      "NtGdiDdGetFlipStatus",
      "NtGdiDdGetInternalMoCompInfo",
      "NtGdiDdGetMoCompBuffInfo",
      "NtGdiDdGetMoCompFormats",
      "NtGdiDdGetMoCompGuids",
      "NtGdiDdGetScanLine",
      "NtGdiDdLock",
      "NtGdiDdNotifyFullscreenSpriteUpdate",
      "NtGdiDdQueryDirectDrawObject",
      "NtGdiDdQueryMoCompStatus",
      "DxEngVisRgnUniq",
      "NtGdiDdReenableDirectDrawObject",
      "NtGdiDdReleaseDC",
      "NtGdiDdRenderMoComp",
      "NtGdiDdSetColorKey",
      "NtGdiDdSetExclusiveMode",
      "NtGdiDdSetGammaRamp",
      "NtGdiDdSetOverlayPosition",
      "NtGdiDdUnattachSurface",
      "NtGdiDdUnlock",
      "NtGdiDdUpdateOverlay",
      "NtGdiDdWaitForVerticalBlank",
      "NtGdiDeleteColorTransform",
      "NtGdiDescribePixelFormat",
      "NtGdiDestroyOPMProtectedOutput",
      "NtGdiDestroyPhysicalMonitor",
      "NtGdiDoBanding",
      "NtGdiDrawEscape",
      "NtGdiDvpAcquireNotification",
      "NtGdiDvpCanCreateVideoPort",
      "NtGdiDvpColorControl",
      "NtGdiDvpCreateVideoPort",
      "NtGdiDvpDestroyVideoPort",
      "NtGdiDvpFlipVideoPort",
      "NtGdiDvpGetVideoPortBandwidth",
      "NtGdiDvpGetVideoPortConnectInfo",
      "NtGdiDvpGetVideoPortField",
      "NtGdiDvpGetVideoPortFlipStatus",
      "NtGdiDvpGetVideoPortInputFormats",
      "NtGdiDvpGetVideoPortLine",
      "NtGdiDvpGetVideoPortOutputFormats",
      "NtGdiDvpGetVideoSignalStatus",
      "NtGdiDvpReleaseNotification",
      "NtGdiDvpUpdateVideoPort",
      "NtGdiDvpWaitForVideoPortSync",
      "NtGdiDxgGenericThunk",
      "NtGdiEllipse",
      "NtGdiEnableEudc",
      "NtGdiEndDoc",
      "NtGdiEndGdiRendering",
      "NtGdiEndPage",
      "NtGdiEngAlphaBlend",
      "NtGdiEngAssociateSurface",
      "NtGdiEngBitBlt",
      "NtGdiEngCheckAbort",
      "NtGdiEngComputeGlyphSet",
      "NtGdiEngCopyBits",
      "NtGdiEngCreateBitmap",
      "NtGdiEngCreateClip",
      "NtGdiEngCreateDeviceBitmap",
      "NtGdiEngCreateDeviceSurface",
      "NtGdiEngCreatePalette",
      "NtGdiEngDeleteClip",
      "NtGdiEngDeletePalette",
      "NtGdiEngDeletePath",
      "NtGdiEngDeleteSurface",
      "NtGdiEngEraseSurface",
      "NtGdiEngFillPath",
      "NtGdiEngGradientFill",
      "NtGdiEngLineTo",
      "NtGdiEngLockSurface",
      "NtGdiEngMarkBandingSurface",
      "NtGdiEngPaint",
      "NtGdiEngPlgBlt",
      "NtGdiEngStretchBlt",
      "NtGdiEngStretchBltROP",
      "NtGdiEngStrokeAndFillPath",
      "NtGdiEngStrokePath",
      "NtGdiEngTextOut",
      "NtGdiEngTransparentBlt",
      "NtGdiEngUnlockSurface",
      "NtGdiEnumFonts",
      "NtGdiEnumObjects",
      "NtGdiEudcLoadUnloadLink",
      "NtGdiExtFloodFill",
      "NtGdiFONTOBJ_cGetAllGlyphHandles",
      "NtGdiFONTOBJ_cGetGlyphs",
      "NtGdiFONTOBJ_pQueryGlyphAttrs",
      "NtGdiFONTOBJ_pfdg",
      "NtGdiFONTOBJ_pifi",
      "NtGdiFONTOBJ_pvTrueTypeFontFile",
      "NtGdiFONTOBJ_pxoGetXform",
      "NtGdiFONTOBJ_vGetInfo",
      "NtGdiFlattenPath",
      "NtGdiFontIsLinked",
      "NtGdiForceUFIMapping",
      "NtGdiFrameRgn",
      "NtGdiFullscreenControl",
      "NtGdiGetBoundsRect",
      "NtGdiGetCOPPCompatibleOPMInformation",
      "NtGdiGetCertificate",
      "NtGdiGetCertificateSize",
      "NtGdiGetCharABCWidthsW",
      "NtGdiGetCharacterPlacementW",
      "NtGdiGetColorAdjustment",
      "NtGdiGetColorSpaceforBitmap",
      "NtGdiGetDeviceCaps",
      "NtGdiGetDeviceCapsAll",
      "NtGdiGetDeviceGammaRamp",
      "NtGdiGetDeviceWidth",
      "NtGdiGetDhpdev",
      "NtGdiGetETM",
      "NtGdiGetEmbUFI",
      "NtGdiGetEmbedFonts",
      "NtGdiGetEudcTimeStampEx",
      "NtGdiGetFontFileData",
      "NtGdiGetFontFileInfo",
      "NtGdiGetFontResourceInfoInternalW",
      "NtGdiGetFontUnicodeRanges",
      "NtGdiGetGlyphIndicesW",
      "NtGdiGetGlyphIndicesWInternal",
      "NtGdiGetGlyphOutline",
      "NtGdiGetKerningPairs",
      "NtGdiGetLinkedUFIs",
      "NtGdiGetMiterLimit",
      "NtGdiGetMonitorID",
      "NtGdiGetNumberOfPhysicalMonitors",
      "NtGdiGetOPMInformation",
      "NtGdiGetOPMRandomNumber",
      "NtGdiGetObjectBitmapHandle",
      "NtGdiGetPath",
      "NtGdiGetPerBandInfo",
      "NtGdiGetPhysicalMonitorDescription",
      "NtGdiGetPhysicalMonitors",
      "NtGdiGetRealizationInfo",
      "NtGdiGetServerMetaFileBits",
      "DxgStubGenericThunk",
      "NtGdiGetStats",
      "NtGdiGetStringBitmapW",
      "NtGdiGetSuggestedOPMProtectedOutputArraySize",
      "NtGdiGetTextExtentExW",
      "NtGdiGetUFI",
      "NtGdiGetUFIPathname",
      "NtGdiGradientFill",
      "NtGdiHLSurfGetInformation",
      "NtGdiHLSurfSetInformation",
      "NtGdiHT_Get8BPPFormatPalette",
      "NtGdiHT_Get8BPPMaskPalette",
      "NtGdiIcmBrushInfo",
      "DxgStubEnableDirectDraw",
      "EngGetPrinter",
      "NtGdiMakeFontDir",
      "NtGdiMakeInfoDC",
      "NtGdiMakeObjectUnXferable",
      "NtGdiMakeObjectXferable",
      "NtGdiMirrorWindowOrg",
      "NtGdiMonoBitmap",
      "NtGdiMoveTo",
      "NtGdiOffsetClipRgn",
      "NtGdiPATHOBJ_bEnum",
      "NtGdiPATHOBJ_bEnumClipLines",
      "NtGdiPATHOBJ_vEnumStart",
      "NtGdiPATHOBJ_vEnumStartClipLines",
      "NtGdiPATHOBJ_vGetBounds",
      "NtGdiPathToRegion",
      "NtGdiPlgBlt",
      "NtGdiPolyDraw",
      "NtGdiPolyTextOutW",
      "NtGdiPtInRegion",
      "NtGdiPtVisible",
      "NtGdiQueryFonts",
      "NtGdiRemoveFontResourceW",
      "NtGdiRemoveMergeFont",
      "NtGdiResetDC",
      "NtGdiResizePalette",
      "NtGdiRoundRect",
      "NtGdiSTROBJ_bEnum",
      "NtGdiSTROBJ_bEnumPositionsOnly",
      "NtGdiSTROBJ_bGetAdvanceWidths",
      "NtGdiSTROBJ_dwGetCodePage",
      "NtGdiSTROBJ_vEnumStart",
      "NtGdiScaleViewportExtEx",
      "NtGdiScaleWindowExtEx",
      "NtGdiSelectBrush",
      "NtGdiSelectClipPath",
      "NtGdiSelectPen",
      "NtGdiSetBitmapAttributes",
      "NtGdiSetBrushAttributes",
      "NtGdiSetColorAdjustment",
      "NtGdiSetColorSpace",
      "NtGdiSetDeviceGammaRamp",
      "NtGdiSetFontXform",
      "NtGdiSetIcmMode",
      "NtGdiSetLinkedUFIs",
      "NtGdiSetMagicColors",
      "NtGdiSetOPMSigningKeyAndSequenceNumbers",
      "NtGdiSetPUMPDOBJ",
      "NtGdiSetPixelFormat",
      "NtGdiSetRectRgn",
      "NtGdiSetSizeDevice",
      "NtGdiSetSystemPaletteUse",
      "NtGdiSetTextJustification",
      "NtGdiSfmGetNotificationTokens",
      "NtGdiStartDoc",
      "NtGdiStartPage",
      "NtGdiStrokeAndFillPath",
      "NtGdiStrokePath",
      "NtGdiSwapBuffers",
      "NtGdiTransparentBlt",
      "NtGdiUMPDEngFreeUserMem",
      "NtGdiUpdateColors",
      "NtGdiUpdateTransform",
      "NtGdiWidenPath",
      "NtGdiXFORMOBJ_bApplyXform",
      "NtGdiXFORMOBJ_iGetXform",
      "NtGdiXLATEOBJ_cGetPalette",
      "NtGdiXLATEOBJ_hGetColorTransform",
      "NtGdiXLATEOBJ_iXlate",
      "NtUserAddClipboardFormatListener",
      "NtUserAssociateInputContext",
      "NtUserBlockInput",
      "NtUserBuildHimcList",
      "NtUserBuildPropList",
      "NtUserCalculatePopupWindowPosition",
      "NtUserCallHwndOpt",
      "NtUserChangeDisplaySettings",
      "NtUserChangeWindowMessageFilterEx",
      "NtUserCheckAccessForIntegrityLevel",
      "NtUserCheckDesktopByThreadId",
      "NtUserCheckWindowThreadDesktop",
      "NtUserChildWindowFromPointEx",
      "NtUserClipCursor",
      "NtUserCreateDesktopEx",
      "NtUserCreateInputContext",
      "NtUserCreateWindowStation",
      "NtUserCtxDisplayIOCtl",
      "NtUserDestroyInputContext",
      "NtUserDisableThreadIme",
      "NtUserDisplayConfigGetDeviceInfo",
      "NtUserDisplayConfigSetDeviceInfo",
      "NtUserDoSoundConnect",
      "NtUserDoSoundDisconnect",
      "NtUserDragDetect",
      "NtUserDragObject",
      "NtUserDrawAnimatedRects",
      "NtUserDrawCaption",
      "NtUserDrawCaptionTemp",
      "NtUserDrawMenuBarTemp",
      "NtUserDwmStartRedirection",
      "NtUserDwmStopRedirection",
      "NtUserEndMenu",
      "NtUserEndTouchOperation",
      "NtUserEvent",
      "NtUserFlashWindowEx",
      "NtUserFrostCrashedWindow",
      "NtUserGetAppImeLevel",
      "NtUserGetCaretPos",
      "NtUserGetClipCursor",
      "NtUserGetClipboardViewer",
      "NtUserGetComboBoxInfo",
      "NtUserGetCursorInfo",
      "NtUserGetDisplayConfigBufferSizes",
      "NtUserGetGestureConfig",
      "NtUserGetGestureExtArgs",
      "NtUserGetGestureInfo",
      "NtUserGetGuiResources",
      "NtUserGetImeHotKey",
      "NtUserGetImeInfoEx",
      "NtUserGetInputLocaleInfo",
      "NtUserGetInternalWindowPos",
      "NtUserGetKeyNameText",
      "NtUserGetKeyboardLayoutName",
      "NtUserGetLayeredWindowAttributes",
      "NtUserGetListBoxInfo",
      "NtUserGetMenuIndex",
      "NtUserGetMenuItemRect",
      "NtUserGetMouseMovePointsEx",
      "NtUserGetPriorityClipboardFormat",
      "NtUserGetRawInputBuffer",
      "NtUserGetRawInputData",
      "NtUserGetRawInputDeviceInfo",
      "NtUserGetRawInputDeviceList",
      "NtUserGetRegisteredRawInputDevices",
      "NtUserGetTopLevelWindow",
      "NtUserGetTouchInputInfo",
      "NtUserGetUpdatedClipboardFormats",
      "NtUserGetWOWClass",
      "NtUserGetWindowCompositionAttribute",
      "NtUserGetWindowCompositionInfo",
      "NtUserGetWindowDisplayAffinity",
      "NtUserGetWindowMinimizeRect",
      "NtUserGetWindowRgnEx",
      "NtUserGhostWindowFromHungWindow",
      "NtUserHardErrorControl",
      "NtUserHiliteMenuItem",
      "NtUserHungWindowFromGhostWindow",
      "NtUserHwndQueryRedirectionInfo",
      "NtUserHwndSetRedirectionInfo",
      "NtUserImpersonateDdeClientWindow",
      "NtUserInitTask",
      "NtUserInitialize",
      "NtUserInitializeClientPfnArrays",
      "NtUserInjectGesture",
      "NtUserInternalGetWindowIcon",
      "NtUserIsTopLevelWindow",
      "NtUserIsTouchWindow",
      "NtUserLoadKeyboardLayoutEx",
      "NtUserLockWindowStation",
      "NtUserLockWorkStation",
      "NtUserLogicalToPhysicalPoint",
      "NtUserMNDragLeave",
      "NtUserMNDragOver",
      "NtUserMagControl",
      "NtUserMagGetContextInformation",
      "NtUserMagSetContextInformation",
      "NtUserManageGestureHandlerWindow",
      "NtUserMenuItemFromPoint",
      "NtUserMinMaximize",
      "NtUserModifyWindowTouchCapability",
      "NtUserNotifyIMEStatus",
      "NtUserOpenInputDesktop",
      "NtUserOpenThreadDesktop",
      "NtUserPaintMonitor",
      "NtUserPhysicalToLogicalPoint",
      "NtUserPrintWindow",
      "NtUserQueryDisplayConfig",
      "NtUserQueryInformationThread",
      "NtUserQueryInputContext",
      "NtUserQuerySendMessage",
      "NtUserRealChildWindowFromPoint",
      "NtUserRealWaitMessageEx",
      "NtUserRegisterErrorReportingDialog",
      "NtUserRegisterHotKey",
      "NtUserRegisterRawInputDevices",
      "NtUserRegisterServicesProcess",
      "NtUserRegisterSessionPort",
      "NtUserRegisterTasklist",
      "NtUserRegisterUserApiHook",
      "NtUserRemoteConnect",
      "NtUserRemoteRedrawRectangle",
      "NtUserRemoteRedrawScreen",
      "NtUserRemoteStopScreenUpdates",
      "NtUserRemoveClipboardFormatListener",
      "NtUserResolveDesktopForWOW",
      "NtUserSendTouchInput",
      "NtUserSetAppImeLevel",
      "NtUserSetChildWindowNoActivate",
      "NtUserSetClassWord",
      "NtUserSetCursorContents",
      "NtUserSetDisplayConfig",
      "NtUserSetGestureConfig",
      "NtUserSetImeHotKey",
      "NtUserSetImeInfoEx",
      "NtUserSetImeOwnerWindow",
      "NtUserSetInternalWindowPos",
      "NtUserSetLayeredWindowAttributes",
      "NtUserSetMenu",
      "NtUserSetMenuContextHelpId",
      "NtUserSetMenuFlagRtoL",
      "NtUserSetMirrorRendering",
      "NtUserSetObjectInformation",
      "NtUserSetProcessDPIAware",
      "NtUserSetShellWindowEx",
      "NtUserSetSysColors",
      "NtUserSetSystemCursor",
      "NtUserSetSystemTimer",
      "NtUserSetThreadLayoutHandles",
      "NtUserSetWindowCompositionAttribute",
      "NtUserSetWindowDisplayAffinity",
      "NtUserSetWindowRgnEx",
      "NtUserSetWindowStationUser",
      "NtUserSfmDestroyLogicalSurfaceBinding",
      "NtUserSfmDxBindSwapChain",
      "NtUserSfmDxGetSwapChainStats",
      "NtUserSfmDxOpenSwapChain",
      "NtUserSfmDxQuerySwapChainBindingStatus",
      "NtUserSfmDxReleaseSwapChain",
      "NtUserSfmDxReportPendingBindingsToDwm",
      "NtUserSfmDxSetSwapChainBindingStatus",
      "NtUserSfmDxSetSwapChainStats",
      "NtUserSfmGetLogicalSurfaceBinding",
      "NtUserShowSystemCursor",
      "NtUserSoundSentry",
      "NtUserSwitchDesktop",
      "NtUserTestForInteractiveUser",
      "NtUserTrackPopupMenuEx",
      "NtUserUnloadKeyboardLayout",
      "NtUserUnlockWindowStation",
      "NtUserUnregisterHotKey",
      "NtUserUnregisterSessionPort",
      "NtUserUnregisterUserApiHook",
      "NtUserUpdateInputContext",
      "NtUserUpdateInstance",
      "NtUserUpdateLayeredWindow",
      "NtUserUpdatePerUserSystemParameters",
      "NtUserUpdateWindowTransform",
      "NtUserUserHandleGrantAccess",
      "NtUserValidateHandleSecure",
      "NtUserWaitForInputIdle",
      "NtUserWaitForMsgAndEvent",
      "NtUserWindowFromPhysicalPoint",
      "NtUserYieldTask",
      "NtUserSetClassLongPtr",
      "NtUserSetWindowLongPtr"
};

// Win10 SSDT 函数名数组
const char* WIN10_SSDT_FUNCTIONS[] = {
      "NtAccessCheck",
      "NtWorkerFactoryWorkerReady",
      "NtAcceptConnectPort",
      "NtMapUserPhysicalPagesScatter",
      "NtWaitForSingleObject",
      "NtCallbackReturn",
      "NtReadFile",
      "NtDeviceIoControlFile",
      "NtWriteFile",
      "NtRemoveIoCompletion",
      "NtReleaseSemaphore",
      "NtReplyWaitReceivePort",
      "NtReplyPort",
      "NtSetInformationThread",
      "NtSetEvent",
      "NtClose",
      "NtQueryObject",
      "NtQueryInformationFile",
      "NtOpenKey",
      "NtEnumerateValueKey",
      "NtFindAtom",
      "NtQueryDefaultLocale",
      "NtQueryKey",
      "NtQueryValueKey",
      "NtAllocateVirtualMemory",
      "NtQueryInformationProcess",
      "NtWaitForMultipleObjects32",
      "NtWriteFileGather",
      "NtSetInformationProcess",
      "NtCreateKey",
      "NtFreeVirtualMemory",
      "NtImpersonateClientOfPort",
      "NtReleaseMutant",
      "NtQueryInformationToken",
      "NtRequestWaitReplyPort",
      "NtQueryVirtualMemory",
      "NtOpenThreadToken",
      "NtQueryInformationThread",
      "NtOpenProcess",
      "NtSetInformationFile",
      "NtMapViewOfSection",
      "NtAccessCheckAndAuditAlarm",
      "NtUnmapViewOfSection",
      "NtReplyWaitReceivePortEx",
      "NtTerminateProcess",
      "NtSetEventBoostPriority",
      "NtReadFileScatter",
      "NtOpenThreadTokenEx",
      "NtOpenProcessTokenEx",
      "NtQueryPerformanceCounter",
      "NtEnumerateKey",
      "NtOpenFile",
      "NtDelayExecution",
      "NtQueryDirectoryFile",
      "NtQuerySystemInformation",
      "NtOpenSection",
      "NtQueryTimer",
      "NtFsControlFile",
      "NtWriteVirtualMemory",
      "NtCloseObjectAuditAlarm",
      "NtDuplicateObject",
      "NtQueryAttributesFile",
      "NtClearEvent",
      "NtReadVirtualMemory",
      "NtOpenEvent",
      "NtAdjustPrivilegesToken",
      "NtDuplicateToken",
      "NtContinue",
      "NtQueryDefaultUILanguage",
      "NtQueueApcThread",
      "NtYieldExecution",
      "NtAddAtom",
      "NtCreateEvent",
      "NtQueryVolumeInformationFile",
      "NtCreateSection",
      "NtFlushBuffersFile",
      "NtApphelpCacheControl",
      "NtCreateProcessEx",
      "NtCreateThread",
      "NtIsProcessInJob",
      "NtProtectVirtualMemory",
      "NtQuerySection",
      "NtResumeThread",
      "NtTerminateThread",
      "NtReadRequestData",
      "NtCreateFile",
      "NtQueryEvent",
      "NtWriteRequestData",
      "NtOpenDirectoryObject",
      "NtAccessCheckByTypeAndAuditAlarm",
      "NtQuerySystemTime",
      "NtWaitForMultipleObjects",
      "NtSetInformationObject",
      "NtCancelIoFile",
      "NtTraceEvent",
      "NtPowerInformation",
      "NtSetValueKey",
      "NtCancelTimer",
      "NtSetTimer",
      "NtAccessCheckByType",
      "NtAccessCheckByTypeResultList",
      "NtAccessCheckByTypeResultListAndAuditAlarm",
      "NtAccessCheckByTypeResultListAndAuditAlarmByHandle",
      "NtAcquireProcessActivityReference",
      "NtAddAtomEx",
      "NtAddBootEntry",
      "NtAddDriverEntry",
      "NtAdjustGroupsToken",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtAlertResumeThread",
      "NtAlertThread",
      "NtAlertThreadByThreadId",
      "NtAllocateLocallyUniqueId",
      "NtAllocateReserveObject",
      "NtAllocateUserPhysicalPages",
      "NtAllocateUuids",
      "NtAllocateVirtualMemoryEx",
      "NtAlpcAcceptConnectPort",
      "NtAlpcCancelMessage",
      "NtAlpcConnectPort",
      "NtAlpcConnectPortEx",
      "NtAlpcCreatePort",
      "NtAlpcCreatePortSection",
      "NtAlpcCreateResourceReserve",
      "NtAlpcCreateSectionView",
      "NtAlpcCreateSecurityContext",
      "NtAlpcDeletePortSection",
      "NtAlpcDeleteResourceReserve",
      "NtAlpcDeleteSectionView",
      "NtAlpcDeleteSecurityContext",
      "NtAlpcDisconnectPort",
      "NtAlpcImpersonateClientContainerOfPort",
      "NtAlpcImpersonateClientOfPort",
      "NtAlpcOpenSenderProcess",
      "NtAlpcOpenSenderThread",
      "NtAlpcQueryInformation",
      "NtAlpcQueryInformationMessage",
      "NtAlpcRevokeSecurityContext",
      "NtAlpcSendWaitReceivePort",
      "NtAlpcSetInformation",
      "NtAreMappedFilesTheSame",
      "NtAssignProcessToJobObject",
      "NtAssociateWaitCompletionPacket",
      "NtCallEnclave",
      "NtCancelIoFileEx",
      "NtCancelSynchronousIoFile",
      "NtCancelTimer2",
      "NtCancelWaitCompletionPacket",
      "NtCommitComplete",
      "NtCommitEnlistment",
      "NtCommitRegistryTransaction",
      "NtCommitTransaction",
      "NtCompactKeys",
      "NtCompareObjects",
      "NtCompareSigningLevels",
      "NtCompareTokens",
      "ArbPreprocessEntry",
      "NtCompressKey",
      "NtConnectPort",
      "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter",
      "ArbAddReserved",
      "NtCreateDebugObject",
      "NtCreateDirectoryObject",
      "NtCreateDirectoryObjectEx",
      "NtCreateEnclave",
      "NtCreateEnlistment",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtCreateIRTimer",
      "NtCreateIoCompletion",
      "NtCreateJobObject",
      "ArbAddReserved",
      "NtCreateKeyTransacted",
      "NtCreateKeyedEvent",
      "NtCreateLowBoxToken",
      "NtCreateMailslotFile",
      "NtCreateMutant",
      "NtCreateNamedPipeFile",
      "NtCreatePagingFile",
      "NtCreatePartition",
      "NtCreatePort",
      "NtCreatePrivateNamespace",
      "NtCreateProcess",
      "NtCreateProfile",
      "NtCreateProfileEx",
      "NtCreateRegistryTransaction",
      "NtCreateResourceManager",
      "NtCreateSectionEx",
      "NtCreateSemaphore",
      "NtCreateSymbolicLinkObject",
      "NtCreateThreadEx",
      "NtCreateTimer",
      "NtCreateTimer2",
      "NtCreateToken",
      "NtCreateTokenEx",
      "NtCreateTransaction",
      "NtCreateTransactionManager",
      "NtCreateUserProcess",
      "NtCreateWaitCompletionPacket",
      "NtCreateWaitablePort",
      "NtCreateWnfStateName",
      "NtCreateWorkerFactory",
      "NtDebugActiveProcess",
      "NtDebugContinue",
      "NtDeleteAtom",
      "NtDeleteBootEntry",
      "NtDeleteDriverEntry",
      "NtDeleteFile",
      "NtDeleteKey",
      "NtDeleteObjectAuditAlarm",
      "NtDeletePrivateNamespace",
      "NtDeleteValueKey",
      "NtDeleteWnfStateData",
      "NtDeleteWnfStateName",
      "NtDisableLastKnownGood",
      "NtDisplayString",
      "NtDrawText",
      "NtEnableLastKnownGood",
      "NtEnumerateBootEntries",
      "NtEnumerateDriverEntries",
      "NtEnumerateSystemEnvironmentValuesEx",
      "NtEnumerateTransactionObject",
      "NtExtendSection",
      "NtFilterBootOption",
      "NtFilterToken",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtFlushBuffersFileEx",
      "NtFlushInstallUILanguage",
      "ArbPreprocessEntry",
      "NtFlushKey",
      "NtFlushProcessWriteBuffers",
      "NtFlushVirtualMemory",
      "NtFlushWriteBuffer",
      "NtFreeUserPhysicalPages",
      "NtFreezeRegistry",
      "NtFreezeTransactions",
      "NtGetCachedSigningLevel",
      "NtGetCompleteWnfStateSubscription",
      "NtGetContextThread",
      "NtGetCurrentProcessorNumber",
      "NtGetCurrentProcessorNumberEx",
      "NtGetDevicePowerState",
      "NtGetMUIRegistryInfo",
      "NtGetNextProcess",
      "NtGetNextThread",
      "NtGetNlsSectionPtr",
      "NtGetNotificationResourceManager",
      "NtGetWriteWatch",
      "NtImpersonateAnonymousToken",
      "NtImpersonateThread",
      "NtInitializeEnclave",
      "NtInitializeNlsFiles",
      "NtInitializeRegistry",
      "NtInitiatePowerAction",
      "NtIsSystemResumeAutomatic",
      "NtIsUILanguageComitted",
      "NtListenPort",
      "NtLoadDriver",
      "NtLoadEnclaveData",
      "NtLoadKey",
      "NtLoadKey2",
      "NtLoadKeyEx",
      "NtLockFile",
      "NtLockProductActivationKeys",
      "NtLockRegistryKey",
      "NtLockVirtualMemory",
      "NtMakePermanentObject",
      "NtMakeTemporaryObject",
      "NtManageHotPatch",
      "NtManagePartition",
      "NtMapCMFModule",
      "NtMapUserPhysicalPages",
      "NtMapViewOfSectionEx",
      "NtModifyBootEntry",
      "NtModifyDriverEntry",
      "NtNotifyChangeDirectoryFile",
      "NtNotifyChangeDirectoryFileEx",
      "NtNotifyChangeKey",
      "NtNotifyChangeMultipleKeys",
      "NtNotifyChangeSession",
      "NtOpenEnlistment",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtOpenIoCompletion",
      "NtOpenJobObject",
      "NtOpenKeyEx",
      "NtOpenKeyTransacted",
      "NtOpenKeyTransactedEx",
      "NtOpenKeyedEvent",
      "NtOpenMutant",
      "NtOpenObjectAuditAlarm",
      "NtOpenPartition",
      "NtOpenPrivateNamespace",
      "NtOpenProcessToken",
      "NtOpenRegistryTransaction",
      "NtOpenResourceManager",
      "NtOpenSemaphore",
      "NtOpenSession",
      "NtOpenSymbolicLinkObject",
      "NtOpenThread",
      "NtOpenTimer",
      "NtOpenTransaction",
      "NtOpenTransactionManager",
      "NtPlugPlayControl",
      "NtPrePrepareComplete",
      "NtPrePrepareEnlistment",
      "NtPrepareComplete",
      "NtPrepareEnlistment",
      "NtPrivilegeCheck",
      "NtPrivilegeObjectAuditAlarm",
      "NtPrivilegedServiceAuditAlarm",
      "NtPropagationComplete",
      "NtPropagationFailed",
      "NtPulseEvent",
      "NtQueryAuxiliaryCounterFrequency",
      "NtQueryBootEntryOrder",
      "NtQueryBootOptions",
      "NtQueryDebugFilterState",
      "NtQueryDirectoryFileEx",
      "NtQueryDirectoryObject",
      "NtQueryDriverEntryOrder",
      "NtQueryEaFile",
      "NtQueryFullAttributesFile",
      "NtQueryInformationAtom",
      "NtQueryInformationByName",
      "NtQueryInformationEnlistment",
      "NtQueryInformationJobObject",
      "NtQueryInformationPort",
      "NtQueryInformationResourceManager",
      "NtQueryInformationTransaction",
      "NtQueryInformationTransactionManager",
      "NtQueryInformationWorkerFactory",
      "NtQueryInstallUILanguage",
      "NtQueryIntervalProfile",
      "NtQueryIoCompletion",
      "NtQueryLicenseValue",
      "NtQueryMultipleValueKey",
      "NtQueryMutant",
      "NtQueryOpenSubKeys",
      "NtQueryOpenSubKeysEx",
      "CmpCleanUpHigherLayerKcbCachesPreCallback",
      "NtQueryQuotaInformationFile",
      "NtQuerySecurityAttributesToken",
      "NtQuerySecurityObject",
      "NtQuerySecurityPolicy",
      "NtQuerySemaphore",
      "NtQuerySymbolicLinkObject",
      "NtQuerySystemEnvironmentValue",
      "NtQuerySystemEnvironmentValueEx",
      "NtQuerySystemInformationEx",
      "NtQueryTimerResolution",
      "NtQueryWnfStateData",
      "NtQueryWnfStateNameInformation",
      "NtQueueApcThreadEx",
      "NtRaiseException",
      "NtRaiseHardError",
      "NtReadOnlyEnlistment",
      "NtRecoverEnlistment",
      "NtRecoverResourceManager",
      "NtRecoverTransactionManager",
      "NtRegisterProtocolAddressInformation",
      "NtRegisterThreadTerminatePort",
      "NtReleaseKeyedEvent",
      "NtReleaseWorkerFactoryWorker",
      "NtRemoveIoCompletionEx",
      "NtRemoveProcessDebug",
      "NtRenameKey",
      "NtRenameTransactionManager",
      "NtReplaceKey",
      "NtReplacePartitionUnit",
      "NtReplyWaitReplyPort",
      "NtRequestPort",
      "NtResetEvent",
      "NtResetWriteWatch",
      "NtRestoreKey",
      "NtResumeProcess",
      "NtRevertContainerImpersonation",
      "NtRollbackComplete",
      "NtRollbackEnlistment",
      "NtRollbackRegistryTransaction",
      "NtRollbackTransaction",
      "NtRollforwardTransactionManager",
      "NtSaveKey",
      "NtSaveKeyEx",
      "NtSaveMergedKeys",
      "NtSecureConnectPort",
      "NtSerializeBoot",
      "NtSetBootEntryOrder",
      "NtSetBootOptions",
      "NtSetCachedSigningLevel",
      "NtSetCachedSigningLevel2",
      "NtSetContextThread",
      "NtSetDebugFilterState",
      "NtSetDefaultHardErrorPort",
      "NtSetDefaultLocale",
      "NtSetDefaultUILanguage",
      "NtSetDriverEntryOrder",
      "NtSetEaFile",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtSetIRTimer",
      "NtSetInformationDebugObject",
      "NtSetInformationEnlistment",
      "NtSetInformationJobObject",
      "NtSetInformationKey",
      "NtSetInformationResourceManager",
      "NtSetInformationSymbolicLink",
      "NtSetInformationToken",
      "NtSetInformationTransaction",
      "NtSetInformationTransactionManager",
      "NtSetInformationVirtualMemory",
      "NtSetInformationWorkerFactory",
      "NtSetIntervalProfile",
      "NtSetIoCompletion",
      "NtSetIoCompletionEx",
      "BvgaSetVirtualFrameBuffer",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtSetQuotaInformationFile",
      "NtSetSecurityObject",
      "NtSetSystemEnvironmentValue",
      "NtSetSystemEnvironmentValueEx",
      "NtSetSystemInformation",
      "NtSetSystemPowerState",
      "NtSetSystemTime",
      "NtSetThreadExecutionState",
      "NtSetTimer2",
      "NtSetTimerEx",
      "NtSetTimerResolution",
      "NtSetUuidSeed",
      "NtSetVolumeInformationFile",
      "NtSetWnfProcessNotificationEvent",
      "NtShutdownSystem",
      "NtShutdownWorkerFactory",
      "NtSignalAndWaitForSingleObject",
      "NtSinglePhaseReject",
      "NtStartProfile",
      "NtStopProfile",
      "NtSubscribeWnfStateChange",
      "NtSuspendProcess",
      "NtSuspendThread",
      "NtSystemDebugControl",
      "NtTerminateEnclave",
      "NtTerminateJobObject",
      "NtTestAlert",
      "NtThawRegistry",
      "NtThawTransactions",
      "NtTraceControl",
      "NtTranslateFilePath",
      "NtUmsThreadYield",
      "NtUnloadDriver",
      "NtUnloadKey",
      "NtUnloadKey2",
      "NtUnloadKeyEx",
      "NtUnlockFile",
      "NtUnlockVirtualMemory",
      "NtUnmapViewOfSectionEx",
      "NtUnsubscribeWnfStateChange",
      "NtUpdateWnfStateData",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "NtWaitForAlertByThreadId",
      "NtWaitForDebugEvent",
      "NtWaitForKeyedEvent",
      "NtWaitForWorkViaWorkerFactory",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z",
      "?GetPnpProperty@NT_DISK@@UEAAJPEAXPEAPEAX@Z"
};

// Win10 ShadowSSDT 函数名数组
const char* WIN10_SHADOWSSDT_FUNCTIONS[] = {
    "NtUserGetOwnerTransformedMonitorRect"
    
    "NtUserYieldTask"
    
    "NtUserSetSensorPresence"
    
    "NtUserGetThreadState"
    
    "NtUserPeekMessage"
    
    "NtUserCallOneParam"
    
    "NtUserGetKeyState"
    
    "NtUserInvalidateRect"
    
    "NtUserCallNoParam"
    
    "NtUserGetMessage"
    
    "NtUserMessageCall"
    
    "NtGdiBitBlt"
    
    "NtGdiGetCharSet"
    
    "NtUserGetDC"
    
    "NtGdiSelectBitmap"
    
    "NtUserWaitMessage"
    
    "NtUserTranslateMessage"
    
    "NtUserGetProp"
    
    "NtUserPostMessage"
    
    "NtUserQueryWindow"
    
    "NtUserTranslateAccelerator"
    
    "NtGdiFlush"
    
    "NtUserRedrawWindow"
    
    "NtUserWindowFromPoint"
    
    "NtUserCallMsgFilter"
    
    "NtUserValidateTimerCallback"
    
    "NtUserBeginPaint"
    
    "NtUserSetTimer"
    
    "NtUserEndPaint"
    
    "NtUserSetCursor"
    
    "NtUserKillTimer"
    
    "NtUserBuildHwndList"
    
    "NtUserSelectPalette"
    
    "NtUserCallNextHookEx"
    
    "NtUserHideCaret"
    
    "NtGdiIntersectClipRect"
    
    "NtUserCallHwndLock"
    
    "NtUserGetProcessWindowStation"
    
    "NtGdiDeleteObjectApp"
    
    "NtUserSetWindowPos"
    
    "NtUserShowCaret"
    
    "NtUserEndDeferWindowPosEx"
    
    "NtUserCallHwndParamLock"
    
    "NtUserVkKeyScanEx"
    
    "NtGdiSetDIBitsToDeviceInternal"
    
    "NtUserCallTwoParam"
    
    "NtGdiGetRandomRgn"
    
    "NtUserCopyAcceleratorTable"
    
    "NtUserNotifyWinEvent"
    
    "NtGdiExtSelectClipRgn"
    
    "NtUserIsClipboardFormatAvailable"
    
    "NtUserSetScrollInfo"
    
    "NtGdiStretchBlt"
    
    "NtUserCreateCaret"
    
    "NtGdiRectVisible"
    
    "NtGdiCombineRgn"
    
    "NtGdiGetDCObject"
    
    "NtUserDispatchMessage"
    
    "NtUserRegisterWindowMessage"
    
    "NtGdiExtTextOutW"
    
    "NtGdiSelectFont"
    
    "NtGdiRestoreDC"
    
    "NtGdiSaveDC"
    
    "NtUserGetForegroundWindow"
    
    "NtUserShowScrollBar"
    
    "NtUserFindExistingCursorIcon"
    
    "NtGdiGetDCDword"
    
    "NtGdiGetRegionData"
    
    "NtGdiLineTo"
    
    "NtUserSystemParametersInfo"
    
    "NtGdiGetAppClipBox"
    
    "NtUserGetAsyncKeyState"
    
    "NtUserGetCPD"
    
    "NtUserRemoveProp"
    
    "NtGdiDoPalette"
    
    "NtGdiPolyPolyDraw"
    
    "NtUserSetCapture"
    
    "NtUserEnumDisplayMonitors"
    
    "NtGdiCreateCompatibleBitmap"
    
    "NtUserSetProp"
    
    "NtGdiGetTextCharsetInfo"
    
    "NtUserSBGetParms"
    
    "NtUserGetIconInfo"
    
    "NtUserExcludeUpdateRgn"
    
    "NtUserSetFocus"
    
    "NtGdiExtGetObjectW"
    
    "NtUserGetUpdateRect"
    
    "NtGdiCreateCompatibleDC"
    
    "NtUserGetClipboardSequenceNumber"
    
    "NtGdiCreatePen"
    
    "NtUserShowWindow"
    
    "NtUserGetKeyboardLayoutList"
    
    "NtGdiPatBlt"
    
    "NtUserMapVirtualKeyEx"
    
    "NtUserSetWindowLong"
    
    "NtGdiHfontCreate"
    
    "NtUserMoveWindow"
    
    "NtUserPostThreadMessage"
    
    "NtUserDrawIconEx"
    
    "NtUserGetSystemMenu"
    
    "NtGdiDrawStream"
    
    "NtUserInternalGetWindowText"
    
    "NtUserGetWindowDC"
    
    "NtGdiInvertRgn"
    
    "NtGdiGetRgnBox"
    
    "NtGdiGetAndSetDCDword"
    
    "NtGdiMaskBlt"
    
    "NtGdiGetWidthTable"
    
    "NtUserScrollDC"
    
    "NtUserGetObjectInformation"
    
    "NtGdiCreateBitmap"
    
    "NtUserFindWindowEx"
    
    "NtGdiPolyPatBlt"
    
    "NtUserUnhookWindowsHookEx"
    
    "NtGdiGetNearestColor"
    
    "NtGdiTransformPoints"
    
    "NtGdiGetDCPoint"
    
    "NtGdiCreateDIBBrush"
    
    "NtGdiGetTextMetricsW"
    
    "NtUserCreateWindowEx"
    
    "NtUserSetParent"
    
    "NtUserGetKeyboardState"
    
    "NtUserToUnicodeEx"
    
    "NtUserGetControlBrush"
    
    "NtUserGetClassName"
    
    "NtGdiAlphaBlend"
    
    "NtGdiOffsetRgn"
    
    "NtUserDefSetText"
    
    "NtGdiGetTextFaceW"
    
    "NtGdiStretchDIBitsInternal"
    
    "NtUserSendInput"
    
    "NtUserGetThreadDesktop"
    
    "NtGdiCreateRectRgn"
    
    "NtGdiGetDIBitsInternal"
    
    "NtUserGetUpdateRgn"
    
    "NtGdiDeleteClientObj"
    
    "NtUserGetIconSize"
    
    "NtUserFillWindow"
    
    "NtGdiExtCreateRegion"
    
    "NtGdiComputeXformCoefficients"
    
    "NtUserSetWindowsHookEx"
    
    "NtUserNotifyProcessCreate"
    
    "NtGdiUnrealizeObject"
    
    "NtUserGetTitleBarInfo"
    
    "NtGdiRectangle"
    
    "NtUserSetThreadDesktop"
    
    "NtUserGetDCEx"
    
    "NtUserGetScrollBarInfo"
    
    "NtGdiGetTextExtent"
    
    "NtUserSetWindowFNID"
    
    "NtGdiSetLayout"
    
    "NtUserCalcMenuBar"
    
    "NtUserThunkedMenuItemInfo"
    
    "NtGdiExcludeClipRect"
    
    "NtGdiCreateDIBSection"
    
    "NtGdiGetDCforBitmap"
    
    "NtUserDestroyCursor"
    
    "NtUserDestroyWindow"
    
    "NtUserCallHwndParam"
    
    "NtGdiCreateDIBitmapInternal"
    
    "NtUserOpenWindowStation"
    
    "NtUserSetCursorIconData"
    
    "NtUserCloseDesktop"
    
    "NtUserOpenDesktop"
    
    "NtUserSetProcessWindowStation"
    
    "NtUserGetAtomName"
    
    "NtGdiExtCreatePen"
    
    "NtGdiCreatePaletteInternal"
    
    "NtGdiSetBrushOrg"
    
    "NtUserBuildNameList"
    
    "NtGdiSetPixel"
    
    "NtUserRegisterClassExWOW"
    
    "NtGdiCreatePatternBrushInternal"
    
    "NtUserGetAncestor"
    
    "NtGdiGetOutlineTextMetricsInternalW"
    
    "NtGdiSetBitmapBits"
    
    "NtUserCloseWindowStation"
    
    "NtUserGetDoubleClickTime"
    
    "NtUserEnableScrollBar"
    
    "NtGdiCreateSolidBrush"
    
    "NtUserGetClassInfoEx"
    
    "NtGdiCreateClientObj"
    
    "NtUserUnregisterClass"
    
    "NtUserDeleteMenu"
    
    "NtGdiRectInRegion"
    
    "NtUserScrollWindowEx"
    
    "NtGdiGetPixel"
    
    "NtUserSetClassLong"
    
    "NtUserGetMenuBarInfo"
    
    "NtGdiGetNearestPaletteIndex"
    
    "NtGdiGetCharWidthW"
    
    "NtUserInvalidateRgn"
    
    "NtUserGetClipboardOwner"
    
    "NtUserSetWindowRgn"
    
    "NtUserBitBltSysBmp"
    
    "NtGdiGetCharWidthInfo"
    
    "NtUserValidateRect"
    
    "NtUserCloseClipboard"
    
    "NtUserOpenClipboard"
    
    "NtUserSetClipboardData"
    
    "NtUserEnableMenuItem"
    
    "NtUserAlterWindowStyle"
    
    "NtGdiFillRgn"
    
    "NtUserGetWindowPlacement"
    
    "NtGdiModifyWorldTransform"
    
    "NtGdiGetFontData"
    
    "NtUserGetOpenClipboardWindow"
    
    "NtUserSetThreadState"
    
    "NtGdiOpenDCW"
    
    "NtUserTrackMouseEvent"
    
    "NtGdiGetTransform"
    
    "NtUserDestroyMenu"
    
    "NtGdiGetBitmapBits"
    
    "NtUserConsoleControl"
    
    "NtUserSetActiveWindow"
    
    "NtUserSetInformationThread"
    
    "NtUserSetWindowPlacement"
    
    "NtUserGetControlColor"
    
    "NtGdiSetMetaRgn"
    
    "NtGdiSetMiterLimit"
    
    "NtGdiSetVirtualResolution"
    
    "NtGdiGetRasterizerCaps"
    
    "NtUserSetWindowWord"
    
    "NtUserGetClipboardFormatName"
    
    "NtUserRealInternalGetMessage"
    
    "NtUserCreateLocalMemHandle"
    
    "NtUserAttachThreadInput"
    
    "NtGdiCreateHalftonePalette"
    
    "NtUserPaintMenuBar"
    
    "NtUserSetKeyboardState"
    
    "NtGdiCombineTransform"
    
    "NtUserCreateAcceleratorTable"
    
    "NtUserGetCursorFrameInfo"
    
    "NtUserGetAltTabInfo"
    
    "NtUserGetCaretBlinkTime"
    
    "NtGdiQueryFontAssocInfo"
    
    "NtUserProcessConnect"
    
    "NtUserEnumDisplayDevices"
    
    "NtUserEmptyClipboard"
    
    "NtUserGetClipboardData"
    
    "NtUserRemoveMenu"
    
    "NtGdiSetBoundsRect"
    
    "NtGdiGetBitmapDimension"
    
    "NtUserConvertMemHandle"
    
    "NtUserDestroyAcceleratorTable"
    
    "NtUserGetGUIThreadInfo"
    
    "NtGdiCloseFigure"
    
    "NtUserSetWindowsHookAW"
    
    "NtUserSetMenuDefaultItem"
    
    "NtUserCheckMenuItem"
    
    "NtUserSetWinEventHook"
    
    "NtUserUnhookWinEvent"
    
    "NtUserLockWindowUpdate"
    
    "NtUserSetSystemMenu"
    
    "NtUserThunkedMenuInfo"
    
    "NtGdiBeginPath"
    
    "NtGdiEndPath"
    
    "NtGdiFillPath"
    
    "NtUserCallHwnd"
    
    "NtUserDdeInitialize"
    
    "NtUserModifyUserStartupInfoFlags"
    
    "NtUserCountClipboardFormats"
    
    "NtGdiAddFontMemResourceEx"
    
    "NtGdiEqualRgn"
    
    "NtGdiGetSystemPaletteUse"
    
    "NtGdiRemoveFontMemResourceEx"
    
    "NtUserEnumDisplaySettings"
    
    "NtUserPaintDesktop"
    
    "NtGdiExtEscape"
    
    "NtGdiSetBitmapDimension"
    
    "NtGdiSetFontEnumeration"
    
    "NtUserChangeClipboardChain"
    
    "NtUserSetClipboardViewer"
    
    "NtUserShowWindowAsync"
    
    "NtGdiCreateColorSpace"
    
    "NtGdiDeleteColorSpace"
    
    "NtUserActivateKeyboardLayout"
    
    "NtBindCompositionSurface"
    
    "NtCloseCompositionInputSink"
    
    "NtCompositionInputThread"
    
    "NtCompositionSetDropTarget"
    
    "NtConfigureInputSpace"
    
    "NtCreateCompositionInputSink"
    
    "NtCreateCompositionSurfaceHandle"
    
    "NtCreateImplicitCompositionInputSink"
    
    "NtDCompositionAddCrossDeviceVisualChild"
    
    "NtDCompositionBeginFrame"
    
    "NtDCompositionCommitChannel"
    
    "NtDCompositionCommitSynchronizationObject"
    
    "NtDCompositionConfirmFrame"
    
    "NtDCompositionConnectPipe"
    
    "NtDCompositionCreateAndBindSharedSection"
    
    "NtDCompositionCreateChannel"
    
    "NtDCompositionCreateConnection"
    
    "NtDCompositionCreateDwmChannel"
    
    "NtDCompositionCreateSharedResourceHandle"
    
    "NtDCompositionCreateSynchronizationObject"
    
    "NtDCompositionDestroyChannel"
    
    "NtDCompositionDestroyConnection"
    
    "NtDCompositionDiscardFrame"
    
    "NtDCompositionDuplicateHandleToProcess"
    
    "NtDCompositionDuplicateSwapchainHandleToDwm"
    
    "NtDCompositionEnableMMCSS"
    
    "NtDCompositionGetBatchId"
    
    "NtDCompositionGetChannels"
    
    "NtDCompositionGetConnectionBatch"
    
    "NtDCompositionGetDeletedResources"
    
    "NtDCompositionGetFrameLegacyTokens"
    
    "NtDCompositionGetFrameStatistics"
    
    "NtDCompositionGetFrameSurfaceUpdates"
    
    "NtDCompositionGetMaterialProperty"
    
    "NtDCompositionProcessChannelBatchBuffer"
    
    "NtDCompositionReferenceSharedResourceOnDwmChannel"
    
    "NtDCompositionRegisterThumbnailVisual"
    
    "NtDCompositionRegisterVirtualDesktopVisual"
    
    "NtDCompositionReleaseAllResources"
    
    "NtDCompositionRemoveCrossDeviceVisualChild"
    
    "NtDCompositionRetireFrame"
    
    "NtDCompositionSetChannelCommitCompletionEvent"
    
    "NtDCompositionSetChannelConnectionId"
    
    "NtDCompositionSetChildRootVisual"
    
    "NtDCompositionSetDebugCounter"
    
    "NtDCompositionSetMaterialProperty"
    
    "NtDCompositionSubmitDWMBatch"
    
    "NtDCompositionSuspendAnimations"
    
    "NtDCompositionSynchronize"
    
    "NtDCompositionTelemetryAnimationScenarioBegin"
    
    "NtDCompositionTelemetryAnimationScenarioReference"
    
    "NtDCompositionTelemetryAnimationScenarioUnreference"
    
    "NtDCompositionTelemetrySetApplicationId"
    
    "NtDCompositionTelemetryTouchInteractionBegin"
    
    "NtDCompositionTelemetryTouchInteractionEnd"
    
    "NtDCompositionTelemetryTouchInteractionUpdate"
    
    "NtDCompositionUpdatePointerCapture"
    
    "NtDCompositionWaitForChannel"
    
    "NtDesktopCaptureBits"
    
    "NtDuplicateCompositionInputSink"
    
    "NtDxgkCreateTrackedWorkload"
    
    "NtDxgkDestroyTrackedWorkload"
    
    "NtDxgkDispMgrOperation"
    
    "NtDxgkEndTrackedWorkload"
    
    "NtDxgkGetAvailableTrackedWorkloadIndex"
    
    "NtDxgkGetProcessList"
    
    "NtDxgkGetTrackedWorkloadStatistics"
    
    "NtDxgkOutputDuplPresentToHwQueue"
    
    "NtDxgkRegisterVailProcess"
    
    "NtDxgkResetTrackedWorkload"
    
    "NtDxgkSubmitPresentBltToHwQueue"
    
    "NtDxgkSubmitPresentToHwQueue"
    
    "NtDxgkUpdateTrackedWorkload"
    
    "NtDxgkVailConnect"
    
    "NtDxgkVailDisconnect"
    
    "NtDxgkVailPromoteCompositionSurface"
    
    "NtEnableOneCoreTransformMode"
    
    "NtFlipObjectAddContent"
    
    "NtFlipObjectAddPoolBuffer"
    
    "NtFlipObjectConsumerAcquirePresent"
    
    "NtFlipObjectConsumerAdjustUsageReference"
    
    "NtFlipObjectConsumerBeginProcessPresent"
    
    "NtFlipObjectConsumerEndProcessPresent"
    
    "NtFlipObjectConsumerPostMessage"
    
    "NtFlipObjectConsumerQueryBufferInfo"
    
    "NtFlipObjectCreate"
    
    "NtFlipObjectDisconnectEndpoint"
    
    "NtFlipObjectOpen"
    
    "NtFlipObjectPresentCancel"
    
    "NtFlipObjectQueryBufferAvailableEvent"
    
    "NtFlipObjectQueryEndpointConnected"
    
    "NtFlipObjectQueryNextMessageToProducer"
    
    "NtFlipObjectReadNextMessageToProducer"
    
    "NtFlipObjectRemoveContent"
    
    "NtFlipObjectRemovePoolBuffer"
    
    "NtFlipObjectSetContent"
    
    "NtGdiAbortDoc"
    
    "NtGdiAbortPath"
    
    "NtGdiAddEmbFontToDC"
    
    "NtGdiAddFontResourceW"
    
    "NtGdiAddInitialFonts"
    
    "NtGdiAddRemoteFontToDC"
    
    "NtGdiAddRemoteMMInstanceToDC"
    
    "NtGdiAngleArc"
    
    "NtGdiAnyLinkedFonts"
    
    "NtGdiArcInternal"
    
    "NtGdiBRUSHOBJ_DeleteRbrush"
    
    "NtGdiBRUSHOBJ_hGetColorTransform"
    
    "NtGdiBRUSHOBJ_pvAllocRbrush"
    
    "NtGdiBRUSHOBJ_pvGetRbrush"
    
    "NtGdiBRUSHOBJ_ulGetBrushColor"
    
    "NtGdiBeginGdiRendering"
    
    "NtGdiCLIPOBJ_bEnum"
    
    "NtGdiCLIPOBJ_cEnumStart"
    
    "NtGdiCLIPOBJ_ppoGetPath"
    
    "NtGdiCancelDC"
    
    "NtGdiChangeGhostFont"
    
    "NtGdiCheckBitmapBits"
    
    "NtGdiClearBitmapAttributes"
    
    "NtGdiClearBrushAttributes"
    
    "NtGdiColorCorrectPalette"
    
    "NtGdiConfigureOPMProtectedOutput"
    
    "NtGdiConvertMetafileRect"
    
    "NtGdiCreateBitmapFromDxSurface"
    
    "NtGdiCreateBitmapFromDxSurface2"
    
    "NtGdiCreateColorTransform"
    
    "NtGdiCreateEllipticRgn"
    
    "NtGdiCreateHatchBrushInternal"
    
    "NtGdiCreateMetafileDC"
    
    "NtGdiCreateOPMProtectedOutput"
    
    "NtGdiCreateOPMProtectedOutputs"
    
    "NtGdiCreateRoundRectRgn"
    
    "NtGdiCreateServerMetaFile"
    
    "NtGdiCreateSessionMappedDIBSection"
    
    "NtGdiDDCCIGetCapabilitiesString"
    
    "NtGdiDDCCIGetCapabilitiesStringLength"
    
    "NtGdiDDCCIGetTimingReport"
    
    "NtGdiDDCCIGetVCPFeature"
    
    "NtGdiDDCCISaveCurrentSettings"
    
    "NtGdiDDCCISetVCPFeature"
    
    "NtGdiDdCreateFullscreenSprite"
    
    "NtGdiDdDDIAbandonSwapChain"
    
    "NtGdiDdDDIAcquireKeyedMutex"
    
    "NtGdiDdDDIAcquireKeyedMutex2"
    
    "NtGdiDdDDIAcquireSwapChain"
    
    "NtGdiDdDDIAddSurfaceToSwapChain"
    
    "NtGdiDdDDIAdjustFullscreenGamma"
    
    "NtGdiDdDDICacheHybridQueryValue"
    
    "NtGdiDdDDIChangeVideoMemoryReservation"
    
    "NtGdiDdDDICheckExclusiveOwnership"
    
    "NtGdiDdDDICheckMonitorPowerState"
    
    "NtGdiDdDDICheckMultiPlaneOverlaySupport"
    
    "NtGdiDdDDICheckMultiPlaneOverlaySupport2"
    
    "NtGdiDdDDICheckMultiPlaneOverlaySupport3"
    
    "NtGdiDdDDICheckOcclusion"
    
    "NtGdiDdDDICheckSharedResourceAccess"
    
    "NtGdiDdDDICheckVidPnExclusiveOwnership"
    
    "NtGdiDdDDICloseAdapter"
    
    "NtGdiDdDDIConfigureSharedResource"
    
    "NtGdiDdDDICreateAllocation"
    
    "NtGdiDdDDICreateBundleObject"
    
    "NtGdiDdDDICreateContext"
    
    "NtGdiDdDDICreateContextVirtual"
    
    "NtGdiDdDDICreateDCFromMemory"
    
    "NtGdiDdDDICreateDevice"
    
    "NtGdiDdDDICreateHwContext"
    
    "NtGdiDdDDICreateHwQueue"
    
    "NtGdiDdDDICreateKeyedMutex"
    
    "NtGdiDdDDICreateKeyedMutex2"
    
    "NtGdiDdDDICreateOutputDupl"
    
    "NtGdiDdDDICreateOverlay"
    
    "NtGdiDdDDICreatePagingQueue"
    
    "NtGdiDdDDICreateProtectedSession"
    
    "NtGdiDdDDICreateSwapChain"
    
    "NtGdiDdDDICreateSynchronizationObject"
    
    "NtGdiDdDDIDDisplayEnum"
    
    "NtGdiDdDDIDestroyAllocation"
    
    "NtGdiDdDDIDestroyAllocation2"
    
    "NtGdiDdDDIDestroyContext"
    
    "NtGdiDdDDIDestroyDCFromMemory"
    
    "NtGdiDdDDIDestroyDevice"
    
    "NtGdiDdDDIDestroyHwContext"
    
    "NtGdiDdDDIDestroyHwQueue"
    
    "NtGdiDdDDIDestroyKeyedMutex"
    
    "NtGdiDdDDIDestroyOutputDupl"
    
    "NtGdiDdDDIDestroyOverlay"
    
    "NtGdiDdDDIDestroyPagingQueue"
    
    "NtGdiDdDDIDestroyProtectedSession"
    
    "NtGdiDdDDIDestroySynchronizationObject"
    
    "NtGdiDdDDIDispMgrCreate"
    
    "NtGdiDdDDIDispMgrSourceOperation"
    
    "NtGdiDdDDIDispMgrTargetOperation"
    
    "NtGdiDdDDIEnumAdapters"
    
    "NtGdiDdDDIEnumAdapters2"
    
    "NtGdiDdDDIEscape"
    
    "NtGdiDdDDIEvict"
    
    "NtGdiDdDDIExtractBundleObject"
    
    "NtGdiDdDDIFlipOverlay"
    
    "NtGdiDdDDIFlushHeapTransitions"
    
    "NtGdiDdDDIFreeGpuVirtualAddress"
    
    "NtGdiDdDDIGetAllocationPriority"
    
    "NtGdiDdDDIGetCachedHybridQueryValue"
    
    "NtGdiDdDDIGetContextInProcessSchedulingPriority"
    
    "NtGdiDdDDIGetContextSchedulingPriority"
    
    "NtGdiDdDDIGetDWMVerticalBlankEvent"
    
    "NtGdiDdDDIGetDeviceState"
    
    "NtGdiDdDDIGetDisplayModeList"
    
    "NtGdiDdDDIGetMemoryBudgetTarget"
    
    "NtGdiDdDDIGetMultiPlaneOverlayCaps"
    
    "NtGdiDdDDIGetMultisampleMethodList"
    
    "NtGdiDdDDIGetOverlayState"
    
    "NtGdiDdDDIGetPostCompositionCaps"
    
    "NtGdiDdDDIGetPresentHistory"
    
    "NtGdiDdDDIGetPresentQueueEvent"
    
    "NtGdiDdDDIGetProcessDeviceRemovalSupport"
    
    "NtGdiDdDDIGetProcessSchedulingPriorityBand"
    
    "NtGdiDdDDIGetProcessSchedulingPriorityClass"
    
    "NtGdiDdDDIGetResourcePresentPrivateDriverData"
    
    "NtGdiDdDDIGetRuntimeData"
    
    "NtGdiDdDDIGetScanLine"
    
    "NtGdiDdDDIGetSetSwapChainMetadata"
    
    "NtGdiDdDDIGetSharedPrimaryHandle"
    
    "NtGdiDdDDIGetSharedResourceAdapterLuid"
    
    "NtGdiDdDDIGetSharedResourceAdapterLuidFlipManager"
    
    "NtGdiDdDDIGetYieldPercentage"
    
    "NtGdiDdDDIInvalidateActiveVidPn"
    
    "NtGdiDdDDIInvalidateCache"
    
    "NtGdiDdDDILock"
    
    "NtGdiDdDDILock2"
    
    "NtGdiDdDDIMakeResident"
    
    "NtGdiDdDDIMapGpuVirtualAddress"
    
    "NtGdiDdDDIMarkDeviceAsError"
    
    "NtGdiDdDDINetDispGetNextChunkInfo"
    
    "NtGdiDdDDINetDispQueryMiracastDisplayDeviceStatus"
    
    "NtGdiDdDDINetDispQueryMiracastDisplayDeviceSupport"
    
    "NtGdiDdDDINetDispStartMiracastDisplayDevice"
    
    "NtGdiDdDDINetDispStopMiracastDisplayDevice"
    
    "NtGdiDdDDIOfferAllocations"
    
    "NtGdiDdDDIOpenAdapterFromDeviceName"
    
    "NtGdiDdDDIOpenAdapterFromHdc"
    
    "NtGdiDdDDIOpenAdapterFromLuid"
    
    "NtGdiDdDDIOpenBundleObjectNtHandleFromName"
    
    "NtGdiDdDDIOpenKeyedMutex"
    
    "NtGdiDdDDIOpenKeyedMutex2"
    
    "NtGdiDdDDIOpenKeyedMutexFromNtHandle"
    
    "NtGdiDdDDIOpenNtHandleFromName"
    
    "NtGdiDdDDIOpenProtectedSessionFromNtHandle"
    
    "NtGdiDdDDIOpenResource"
    
    "NtGdiDdDDIOpenResourceFromNtHandle"
    
    "NtGdiDdDDIOpenSwapChain"
    
    "NtGdiDdDDIOpenSyncObjectFromNtHandle"
    
    "NtGdiDdDDIOpenSyncObjectFromNtHandle2"
    
    "NtGdiDdDDIOpenSyncObjectNtHandleFromName"
    
    "NtGdiDdDDIOpenSynchronizationObject"
    
    "NtGdiDdDDIOutputDuplGetFrameInfo"
    
    "NtGdiDdDDIOutputDuplGetMetaData"
    
    "NtGdiDdDDIOutputDuplGetPointerShapeData"
    
    "NtGdiDdDDIOutputDuplPresent"
    
    "NtGdiDdDDIOutputDuplReleaseFrame"
    
    "NtGdiDdDDIPinDirectFlipResources"
    
    "NtGdiDdDDIPollDisplayChildren"
    
    "NtGdiDdDDIPresent"
    
    "NtGdiDdDDIPresentMultiPlaneOverlay"
    
    "NtGdiDdDDIPresentMultiPlaneOverlay2"
    
    "NtGdiDdDDIPresentMultiPlaneOverlay3"
    
    "NtGdiDdDDIPresentRedirected"
    
    "NtGdiDdDDIQueryAdapterInfo"
    
    "NtGdiDdDDIQueryAllocationResidency"
    
    "NtGdiDdDDIQueryClockCalibration"
    
    "NtGdiDdDDIQueryFSEBlock"
    
    "NtGdiDdDDIQueryProcessOfferInfo"
    
    "NtGdiDdDDIQueryProtectedSessionInfoFromNtHandle"
    
    "NtGdiDdDDIQueryProtectedSessionStatus"
    
    "NtGdiDdDDIQueryRemoteVidPnSourceFromGdiDisplayName"
    
    "NtGdiDdDDIQueryResourceInfo"
    
    "NtGdiDdDDIQueryResourceInfoFromNtHandle"
    
    "NtGdiDdDDIQueryStatistics"
    
    "NtGdiDdDDIQueryVidPnExclusiveOwnership"
    
    "NtGdiDdDDIQueryVideoMemoryInfo"
    
    "NtGdiDdDDIReclaimAllocations"
    
    "NtGdiDdDDIReclaimAllocations2"
    
    "NtGdiDdDDIReleaseKeyedMutex"
    
    "NtGdiDdDDIReleaseKeyedMutex2"
    
    "NtGdiDdDDIReleaseProcessVidPnSourceOwners"
    
    "NtGdiDdDDIReleaseSwapChain"
    
    "NtGdiDdDDIRemoveSurfaceFromSwapChain"
    
    "NtGdiDdDDIRender"
    
    "NtGdiDdDDIReserveGpuVirtualAddress"
    
    "NtGdiDdDDISetAllocationPriority"
    
    "NtGdiDdDDISetContextInProcessSchedulingPriority"
    
    "NtGdiDdDDISetContextSchedulingPriority"
    
    "NtGdiDdDDISetDisplayMode"
    
    "NtGdiDdDDISetDodIndirectSwapchain"
    
    "NtGdiDdDDISetFSEBlock"
    
    "NtGdiDdDDISetGammaRamp"
    
    "NtGdiDdDDISetHwProtectionTeardownRecovery"
    
    "NtGdiDdDDISetMemoryBudgetTarget"
    
    "NtGdiDdDDISetMonitorColorSpaceTransform"
    
    "NtGdiDdDDISetProcessDeviceRemovalSupport"
    
    "NtGdiDdDDISetProcessSchedulingPriorityBand"
    
    "NtGdiDdDDISetProcessSchedulingPriorityClass"
    
    "NtGdiDdDDISetQueuedLimit"
    
    "NtGdiDdDDISetStablePowerState"
    
    "NtGdiDdDDISetStereoEnabled"
    
    "NtGdiDdDDISetSyncRefreshCountWaitTarget"
    
    "NtGdiDdDDISetVidPnSourceHwProtection"
    
    "NtGdiDdDDISetVidPnSourceOwner"
    
    "NtGdiDdDDISetYieldPercentage"
    
    "NtGdiDdDDIShareObjects"
    
    "NtGdiDdDDISharedPrimaryLockNotification"
    
    "NtGdiDdDDISharedPrimaryUnLockNotification"
    
    "NtGdiDdDDISignalSynchronizationObject"
    
    "NtGdiDdDDISignalSynchronizationObjectFromCpu"
    
    "NtGdiDdDDISignalSynchronizationObjectFromGpu"
    
    "NtGdiDdDDISignalSynchronizationObjectFromGpu2"
    
    "NtGdiDdDDISubmitCommand"
    
    "NtGdiDdDDISubmitCommandToHwQueue"
    
    "NtGdiDdDDISubmitSignalSyncObjectsToHwQueue"
    
    "NtGdiDdDDISubmitWaitForSyncObjectsToHwQueue"
    
    "NtGdiDdDDITrimProcessCommitment"
    
    "NtGdiDdDDIUnOrderedPresentSwapChain"
    
    "NtGdiDdDDIUnlock"
    
    "NtGdiDdDDIUnlock2"
    
    "NtGdiDdDDIUnpinDirectFlipResources"
    
    "NtGdiDdDDIUpdateAllocationProperty"
    
    "NtGdiDdDDIUpdateGpuVirtualAddress"
    
    "NtGdiDdDDIUpdateOverlay"
    
    "NtGdiDdDDIWaitForIdle"
    
    "NtGdiDdDDIWaitForSynchronizationObject"
    
    "NtGdiDdDDIWaitForSynchronizationObjectFromCpu"
    
    "NtGdiDdDDIWaitForSynchronizationObjectFromGpu"
    
    "NtGdiDdDDIWaitForVerticalBlankEvent"
    
    "NtGdiDdDDIWaitForVerticalBlankEvent2"
    
    "NtGdiDdDestroyFullscreenSprite"
    
    "NtGdiDdNotifyFullscreenSpriteUpdate"
    
    "NtGdiDdQueryVisRgnUniqueness"
    
    "NtGdiDeleteColorTransform"
    
    "NtGdiDescribePixelFormat"
    
    "NtGdiDestroyOPMProtectedOutput"
    
    "NtGdiDestroyPhysicalMonitor"
    
    "NtGdiDoBanding"
    
    "NtGdiDrawEscape"
    
    "NtGdiDwmCreatedBitmapRemotingOutput"
    
    "NtGdiEllipse"
    
    "NtGdiEnableEudc"
    
    "NtGdiEndDoc"
    
    "NtGdiEndGdiRendering"
    
    "NtGdiEndPage"
    
    "NtGdiEngAlphaBlend"
    
    "NtGdiEngAssociateSurface"
    
    "NtGdiEngBitBlt"
    
    "NtGdiEngCheckAbort"
    
    "NtGdiEngComputeGlyphSet"
    
    "NtGdiEngCopyBits"
    
    "NtGdiEngCreateBitmap"
    
    "NtGdiEngCreateClip"
    
    "NtGdiEngCreateDeviceBitmap"
    
    "NtGdiEngCreateDeviceSurface"
    
    "NtGdiEngCreatePalette"
    
    "NtGdiEngDeleteClip"
    
    "NtGdiEngDeletePalette"
    
    "NtGdiEngDeletePath"
    
    "NtGdiEngDeleteSurface"
    
    "NtGdiEngEraseSurface"
    
    "NtGdiEngFillPath"
    
    "NtGdiEngGradientFill"
    
    "NtGdiEngLineTo"
    
    "NtGdiEngLockSurface"
    
    "NtGdiEngMarkBandingSurface"
    
    "NtGdiEngPaint"
    
    "NtGdiEngPlgBlt"
    
    "NtGdiEngStretchBlt"
    
    "NtGdiEngStretchBltROP"
    
    "NtGdiEngStrokeAndFillPath"
    
    "NtGdiEngStrokePath"
    
    "NtGdiEngTextOut"
    
    "NtGdiEngTransparentBlt"
    
    "NtGdiEngUnlockSurface"
    
    "NtGdiEnsureDpiDepDefaultGuiFontForPlateau"
    
    "NtGdiEnumFonts"
    
    "NtGdiEnumObjects"
    
    "NtGdiEudcLoadUnloadLink"
    
    "NtGdiExtFloodFill"
    
    "NtGdiFONTOBJ_cGetAllGlyphHandles"
    
    "NtGdiFONTOBJ_cGetGlyphs"
    
    "NtGdiFONTOBJ_pQueryGlyphAttrs"
    
    "NtGdiFONTOBJ_pfdg"
    
    "NtGdiFONTOBJ_pifi"
    
    "NtGdiFONTOBJ_pvTrueTypeFontFile"
    
    "NtGdiFONTOBJ_pxoGetXform"
    
    "NtGdiFONTOBJ_vGetInfo"
    
    "NtGdiFlattenPath"
    
    "NtGdiFontIsLinked"
    
    "NtGdiForceUFIMapping"
    
    "NtGdiFrameRgn"
    
    "NtGdiFullscreenControl"
    
    "NtGdiGetAppliedDeviceGammaRamp"
    
    "NtGdiGetBitmapDpiScaleValue"
    
    "NtGdiGetBoundsRect"
    
    "NtGdiGetCOPPCompatibleOPMInformation"
    
    "NtGdiGetCertificate"
    
    "NtGdiGetCertificateByHandle"
    
    "NtGdiGetCertificateSize"
    
    "NtGdiGetCertificateSizeByHandle"
    
    "NtGdiGetCharABCWidthsW"
    
    "NtGdiGetCharacterPlacementW"
    
    "NtGdiGetColorAdjustment"
    
    "NtGdiGetColorSpaceforBitmap"
    
    "NtGdiGetCurrentDpiInfo"
    
    "NtGdiGetDCDpiScaleValue"
    
    "NtGdiGetDeviceCaps"
    
    "NtGdiGetDeviceCapsAll"
    
    "NtGdiGetDeviceGammaRamp"
    
    "NtGdiGetDeviceWidth"
    
    "NtGdiGetDhpdev"
    
    "NtGdiGetETM"
    
    "NtGdiGetEmbUFI"
    
    "NtGdiGetEmbedFonts"
    
    "NtGdiGetEntry"
    
    "NtGdiGetEudcTimeStampEx"
    
    "NtGdiGetFontFileData"
    
    "NtGdiGetFontFileInfo"
    
    "NtGdiGetFontResourceInfoInternalW"
    
    "NtGdiGetFontUnicodeRanges"
    
    "NtGdiGetGammaRampCapability"
    
    "NtGdiGetGlyphIndicesW"
    
    "NtGdiGetGlyphIndicesWInternal"
    
    "NtGdiGetGlyphOutline"
    
    "NtGdiGetKerningPairs"
    
    "NtGdiGetLinkedUFIs"
    
    "NtGdiGetMiterLimit"
    
    "NtGdiGetMonitorID"
    
    "NtGdiGetNumberOfPhysicalMonitors"
    
    "NtGdiGetOPMInformation"
    
    "NtGdiGetOPMRandomNumber"
    
    "NtGdiGetObjectBitmapHandle"
    
    "NtGdiGetPath"
    
    "NtGdiGetPerBandInfo"
    
    "NtGdiGetPhysicalMonitorDescription"
    
    "NtGdiGetPhysicalMonitors"
    
    "NtGdiGetProcessSessionFonts"
    
    "NtGdiGetPublicFontTableChangeCookie"
    
    "NtGdiGetRealizationInfo"
    
    "NtGdiGetServerMetaFileBits"
    
    "NtGdiGetSpoolMessage"
    
    "NtGdiGetStats"
    
    "NtGdiGetStringBitmapW"
    
    "NtGdiGetSuggestedOPMProtectedOutputArraySize"
    
    "NtGdiGetTextExtentExW"
    
    "NtGdiGetUFI"
    
    "NtGdiGetUFIPathname"
    
    "NtGdiGradientFill"
    
    "NtGdiHLSurfGetInformation"
    
    "NtGdiHLSurfSetInformation"
    
    "NtGdiHT_Get8BPPFormatPalette"
    
    "NtGdiHT_Get8BPPMaskPalette"
    
    "NtGdiIcmBrushInfo"
    
    "NtGdiInit"
    
    "NtGdiInitSpool"
    
    "NtGdiMakeFontDir"
    
    "NtGdiMakeInfoDC"
    
    "NtGdiMakeObjectUnXferable"
    
    "NtGdiMakeObjectXferable"
    
    "NtGdiMirrorWindowOrg"
    
    "NtGdiMonoBitmap"
    
    "NtGdiMoveTo"
    
    "NtGdiOffsetClipRgn"
    
    "NtGdiPATHOBJ_bEnum"
    
    "NtGdiPATHOBJ_bEnumClipLines"
    
    "NtGdiPATHOBJ_vEnumStart"
    
    "NtGdiPATHOBJ_vEnumStartClipLines"
    
    "NtGdiPATHOBJ_vGetBounds"
    
    "NtGdiPathToRegion"
    
    "NtGdiPlgBlt"
    
    "NtGdiPolyDraw"
    
    "NtGdiPolyTextOutW"
    
    "NtGdiPtInRegion"
    
    "NtGdiPtVisible"
    
    "NtGdiQueryFonts"
    
    "NtGdiRemoveFontResourceW"
    
    "NtGdiRemoveMergeFont"
    
    "NtGdiResetDC"
    
    "NtGdiResizePalette"
    
    "NtGdiRoundRect"
    
    "NtGdiSTROBJ_bEnum"
    
    "NtGdiSTROBJ_bEnumPositionsOnly"
    
    "NtGdiSTROBJ_bGetAdvanceWidths"
    
    "NtGdiSTROBJ_dwGetCodePage"
    
    "NtGdiSTROBJ_vEnumStart"
    
    "NtGdiScaleRgn"
    
    "NtGdiScaleValues"
    
    "NtGdiScaleViewportExtEx"
    
    "NtGdiScaleWindowExtEx"
    
    "NtGdiSelectBrush"
    
    "NtGdiSelectClipPath"
    
    "NtGdiSelectPen"
    
    "NtGdiSetBitmapAttributes"
    
    "NtGdiSetBrushAttributes"
    
    "NtGdiSetColorAdjustment"
    
    "NtGdiSetColorSpace"
    
    "NtGdiSetDeviceGammaRamp"
    
    "NtGdiSetFontXform"
    
    "NtGdiSetIcmMode"
    
    "NtGdiSetLinkedUFIs"
    
    "NtGdiSetMagicColors"
    
    "NtGdiSetOPMSigningKeyAndSequenceNumbers"
    
    "NtGdiSetPUMPDOBJ"
    
    "NtGdiSetPixelFormat"
    
    "NtGdiSetPrivateDeviceGammaRamp"
    
    "NtGdiSetRectRgn"
    
    "NtGdiSetSizeDevice"
    
    "NtGdiSetSystemPaletteUse"
    
    "NtGdiSetTextJustification"
    
    "NtGdiSetUMPDSandboxState"
    
    "NtGdiStartDoc"
    
    "NtGdiStartPage"
    
    "NtGdiStrokeAndFillPath"
    
    "NtGdiStrokePath"
    
    "NtGdiSwapBuffers"
    
    "NtGdiTransparentBlt"
    
    "NtGdiUMPDEngFreeUserMem"
    
    "NtGdiUnloadPrinterDriver"
    
    "NtGdiUnmapMemFont"
    
    "NtGdiUpdateColors"
    
    "NtGdiUpdateTransform"
    
    "NtGdiWidenPath"
    
    "NtGdiXFORMOBJ_bApplyXform"
    
    "NtGdiXFORMOBJ_iGetXform"
    
    "NtGdiXLATEOBJ_cGetPalette"
    
    "NtGdiXLATEOBJ_hGetColorTransform"
    
    "NtGdiXLATEOBJ_iXlate"
    
    "NtHWCursorUpdatePointer"
    
    "NtIsOneCoreTransformMode"
    
    "NtMITActivateInputProcessing"
    
    "NtMITCoreMsgKOpenConnectionTo"
    
    "NtMITDeactivateInputProcessing"
    
    "NtMITDisableMouseIntercept"
    
    "NtMITDispatchCompletion"
    
    "NtMITEnableMouseIntercept"
    
    "NtMITGetCursorUpdateHandle"
    
    "NtMITSetInputCallbacks"
    
    "NtMITSetInputDelegationMode"
    
    "NtMITSetInputSuppressionState"
    
    "NtMITSetKeyboardInputRoutingPolicy"
    
    "NtMITSetKeyboardOverriderState"
    
    "NtMITSetLastInputRecipient"
    
    "NtMITSynthesizeKeyboardInput"
    
    "NtMITSynthesizeMouseInput"
    
    "NtMITSynthesizeTouchInput"
    
    "NtMITUpdateInputGlobals"
    
    "NtMapVisualRelativePoints"
    
    "NtNotifyPresentToCompositionSurface"
    
    "NtOpenCompositionSurfaceDirtyRegion"
    
    "NtOpenCompositionSurfaceSectionInfo"
    
    "NtOpenCompositionSurfaceSwapChainHandleInfo"
    
    "NtQueryCompositionInputIsImplicit"
    
    "NtQueryCompositionInputQueueAndTransform"
    
    "NtQueryCompositionInputSink"
    
    "NtQueryCompositionInputSinkLuid"
    
    "NtQueryCompositionInputSinkViewId"
    
    "NtQueryCompositionSurfaceBinding"
    
    "NtQueryCompositionSurfaceHDRMetaData"
    
    "NtQueryCompositionSurfaceRenderingRealization"
    
    "NtQueryCompositionSurfaceStatistics"
    
    "NtRIMAddInputObserver"
    
    "NtRIMAreSiblingDevices"
    
    "NtRIMDeviceIoControl"
    
    "NtRIMEnableMonitorMappingForDevice"
    
    "NtRIMFreeInputBuffer"
    
    "NtRIMGetDevicePreparsedData"
    
    "NtRIMGetDevicePreparsedDataLockfree"
    
    "NtRIMGetDeviceProperties"
    
    "NtRIMGetDevicePropertiesLockfree"
    
    "NtRIMGetPhysicalDeviceRect"
    
    "NtRIMGetSourceProcessId"
    
    "NtRIMObserveNextInput"
    
    "NtRIMOnPnpNotification"
    
    "NtRIMOnTimerNotification"
    
    "NtRIMReadInput"
    
    "NtRIMRegisterForInput"
    
    "NtRIMRemoveInputObserver"
    
    "NtRIMSetExtendedDeviceProperty"
    
    "NtRIMSetTestModeStatus"
    
    "NtRIMUnregisterForInput"
    
    "NtRIMUpdateInputObserverRegistration"
    
    "NtSetCompositionSurfaceAnalogExclusive"
    
    "NtSetCompositionSurfaceBufferUsage"
    
    "NtSetCompositionSurfaceDirectFlipState"
    
    "NtSetCompositionSurfaceIndependentFlipInfo"
    
    "NtSetCompositionSurfaceStatistics"
    
    "NtSetCursorInputSpace"
    
    "NtSetPointerDeviceInputSpace"
    
    "NtSetShellCursorState"
    
    "NtTokenManagerConfirmOutstandingAnalogToken"
    
    "NtTokenManagerCreateCompositionTokenHandle"
    
    "NtTokenManagerCreateFlipObjectReturnTokenHandle"
    
    "NtTokenManagerCreateFlipObjectTokenHandle"
    
    "NtTokenManagerGetAnalogExclusiveSurfaceUpdates"
    
    "NtTokenManagerGetAnalogExclusiveTokenEvent"
    
    "NtTokenManagerOpenSectionAndEvents"
    
    "NtTokenManagerThread"
    
    "NtUnBindCompositionSurface"
    
    "NtUpdateInputSinkTransforms"
    
    "NtUserAcquireIAMKey"
    
    "NtUserAcquireInteractiveControlBackgroundAccess"
    
    "NtUserAddClipboardFormatListener"
    
    "NtUserAddVisualIdentifier"
    
    "NtUserAssociateInputContext"
    
    "NtUserAutoPromoteMouseInPointer"
    
    "NtUserAutoRotateScreen"
    
    "NtUserBeginLayoutUpdate"
    
    "NtUserBlockInput"
    
    "NtUserBroadcastThemeChangeEvent"
    
    "NtUserBuildHimcList"
    
    "NtUserBuildPropList"
    
    "NtUserCalculatePopupWindowPosition"
    
    "NtUserCallHwndLockSafe"
    
    "NtUserCallHwndOpt"
    
    "NtUserCallHwndParamLockSafe"
    
    "NtUserCallHwndSafe"
    
    "NtUserCanBrokerForceForeground"
    
    "NtUserChangeDisplaySettings"
    
    "NtUserChangeWindowMessageFilterEx"
    
    "NtUserCheckAccessForIntegrityLevel"
    
    "NtUserCheckProcessForClipboardAccess"
    
    "NtUserCheckProcessSession"
    
    "NtUserCheckWindowThreadDesktop"
    
    "NtUserChildWindowFromPointEx"
    
    "NtUserClearForeground"
    
    "NtUserClipCursor"
    
    "NtUserCompositionInputSinkLuidFromPoint"
    
    "NtUserCompositionInputSinkViewInstanceIdFromPoint"
    
    "NtUserConfigureActivationObject"
    
    "NtUserConfirmResizeCommit"
    
    "NtUserCreateActivationObject"
    
    "NtUserCreateDCompositionHwndTarget"
    
    "NtUserCreateDesktopEx"
    
    "NtUserCreateEmptyCursorObject"
    
    "NtUserCreateInputContext"
    
    "NtUserCreatePalmRejectionDelayZone"
    
    "NtUserCreateWindowGroup"
    
    "NtUserCreateWindowStation"
    
    "NtUserCtxDisplayIOCtl"
    
    "NtUserDeferWindowDpiChanges"
    
    "NtUserDeferWindowPosAndBand"
    
    "NtUserDelegateCapturePointers"
    
    "NtUserDelegateInput"
    
    "NtUserDeleteWindowGroup"
    
    "NtUserDestroyActivationObject"
    
    "NtUserDestroyDCompositionHwndTarget"
    
    "NtUserDestroyInputContext"
    
    "NtUserDestroyPalmRejectionDelayZone"
    
    "NtUserDisableImmersiveOwner"
    
    "NtUserDisableProcessWindowFiltering"
    
    "NtUserDisableThreadIme"
    
    "NtUserDiscardPointerFrameMessages"
    
    "NtUserDisplayConfigGetDeviceInfo"
    
    "NtUserDisplayConfigSetDeviceInfo"
    
    "NtUserDoSoundConnect"
    
    "NtUserDoSoundDisconnect"
    
    "NtUserDownlevelTouchpad"
    
    "NtUserDragDetect"
    
    "NtUserDragObject"
    
    "NtUserDrawAnimatedRects"
    
    "NtUserDrawCaption"
    
    "NtUserDrawCaptionTemp"
    
    "NtUserDrawMenuBarTemp"
    
    "NtUserDwmGetRemoteSessionOcclusionEvent"
    
    "NtUserDwmGetRemoteSessionOcclusionState"
    
    "NtUserDwmKernelShutdown"
    
    "NtUserDwmKernelStartup"
    
    "NtUserDwmValidateWindow"
    
    "NtUserEnableChildWindowDpiMessage"
    
    "NtUserEnableIAMAccess"
    
    "NtUserEnableMouseInPointer"
    
    "NtUserEnableMouseInputForCursorSuppression"
    
    "NtUserEnableNonClientDpiScaling"
    
    "NtUserEnableResizeLayoutSynchronization"
    
    "NtUserEnableSoftwareCursorForScreenCapture"
    
    "NtUserEnableTouchPad"
    
    "NtUserEnableWindowGDIScaledDpiMessage"
    
    "NtUserEnableWindowGroupPolicy"
    
    "NtUserEnableWindowResizeOptimization"
    
    "NtUserEndMenu"
    
    "NtUserEvent"
    
    "NtUserFlashWindowEx"
    
    "NtUserForceWindowToDpiForTest"
    
    "NtUserFrostCrashedWindow"
    
    "NtUserFunctionalizeDisplayConfig"
    
    "NtUserGetActiveProcessesDpis"
    
    "NtUserGetAppImeLevel"
    
    "NtUserGetAutoRotationState"
    
    "NtUserGetCIMSSM"
    
    "NtUserGetCaretPos"
    
    "NtUserGetClipCursor"
    
    "NtUserGetClipboardAccessToken"
    
    "NtUserGetClipboardViewer"
    
    "NtUserGetComboBoxInfo"
    
    "NtUserGetCurrentDpiInfoForWindow"
    
    "NtUserGetCurrentInputMessageSource"
    
    "NtUserGetCursor"
    
    "NtUserGetCursorInfo"
    
    "NtUserGetDManipHookInitFunction"
    
    "NtUserGetDesktopID"
    
    "NtUserGetDisplayAutoRotationPreferences"
    
    "NtUserGetDisplayAutoRotationPreferencesByProcessId"
    
    "NtUserGetDisplayConfigBufferSizes"
    
    "NtUserGetDpiForCurrentProcess"
    
    "NtUserGetDpiForMonitor"
    
    "NtUserGetExtendedPointerDeviceProperty"
    
    "NtUserGetGestureConfig"
    
    "NtUserGetGestureExtArgs"
    
    "NtUserGetGestureInfo"
    
    "NtUserGetGuiResources"
    
    "NtUserGetHDevName"
    
    "NtUserGetHimetricScaleFactorFromPixelLocation"
    
    "NtUserGetImeHotKey"
    
    "NtUserGetImeInfoEx"
    
    "NtUserGetInputContainerId"
    
    "NtUserGetInputLocaleInfo"
    
    "NtUserGetInteractiveControlDeviceInfo"
    
    "NtUserGetInteractiveControlInfo"
    
    "NtUserGetInteractiveCtrlSupportedWaveforms"
    
    "NtUserGetInternalWindowPos"
    
    "NtUserGetKeyNameText"
    
    "NtUserGetKeyboardLayout"
    
    "NtUserGetKeyboardLayoutName"
    
    "NtUserGetLayeredWindowAttributes"
    
    "NtUserGetListBoxInfo"
    
    "NtUserGetMenuIndex"
    
    "NtUserGetMenuItemRect"
    
    "NtUserGetMouseMovePointsEx"
    
    "NtUserGetOemBitmapSize"
    
    "NtUserGetPhysicalDeviceRect"
    
    "NtUserGetPointerCursorId"
    
    "NtUserGetPointerDevice"
    
    "NtUserGetPointerDeviceCursors"
    
    "NtUserGetPointerDeviceOrientation"
    
    "NtUserGetPointerDeviceProperties"
    
    "NtUserGetPointerDeviceRects"
    
    "NtUserGetPointerDevices"
    
    "NtUserGetPointerFrameTimes"
    
    "NtUserGetPointerInfoList"
    
    "NtUserGetPointerInputTransform"
    
    "NtUserGetPointerProprietaryId"
    
    "NtUserGetPointerType"
    
    "NtUserGetPrecisionTouchPadConfiguration"
    
    "NtUserGetPriorityClipboardFormat"
    
    "NtUserGetProcessDpiAwarenessContext"
    
    "NtUserGetProcessUIContextInformation"
    
    "NtUserGetQueueStatusReadonly"
    
    "NtUserGetRawInputBuffer"
    
    "NtUserGetRawInputData"
    
    "NtUserGetRawInputDeviceInfo"
    
    "NtUserGetRawInputDeviceList"
    
    "NtUserGetRawPointerDeviceData"
    
    "NtUserGetRegisteredRawInputDevices"
    
    "NtUserGetRequiredCursorSizes"
    
    "NtUserGetResizeDCompositionSynchronizationObject"
    
    "NtUserGetSystemDpiForProcess"
    
    "NtUserGetTopLevelWindow"
    
    "NtUserGetTouchInputInfo"
    
    "NtUserGetTouchValidationStatus"
    
    "NtUserGetUniformSpaceMapping"
    
    "NtUserGetUpdatedClipboardFormats"
    
    "NtUserGetWOWClass"
    
    "NtUserGetWindowBand"
    
    "NtUserGetWindowCompositionAttribute"
    
    "NtUserGetWindowCompositionInfo"
    
    "NtUserGetWindowDisplayAffinity"
    
    "NtUserGetWindowFeedbackSetting"
    
    "NtUserGetWindowGroupId"
    
    "NtUserGetWindowMinimizeRect"
    
    "NtUserGetWindowProcessHandle"
    
    "NtUserGetWindowRgnEx"
    
    "NtUserGhostWindowFromHungWindow"
    
    "NtUserHandleDelegatedInput"
    
    "NtUserHardErrorControl"
    
    "NtUserHidePointerContactVisualization"
    
    "NtUserHiliteMenuItem"
    
    "NtUserHungWindowFromGhostWindow"
    
    "NtUserHwndQueryRedirectionInfo"
    
    "NtUserHwndSetRedirectionInfo"
    
    "NtUserImpersonateDdeClientWindow"
    
    "NtUserInheritWindowMonitor"
    
    "NtUserInitTask"
    
    "NtUserInitialize"
    
    "NtUserInitializeClientPfnArrays"
    
    "NtUserInitializeGenericHidInjection"
    
    "NtUserInitializeInputDeviceInjection"
    
    "NtUserInitializePointerDeviceInjection"
    
    "NtUserInitializePointerDeviceInjectionEx"
    
    "NtUserInitializeTouchInjection"
    
    "NtUserInjectDeviceInput"
    
    "NtUserInjectGenericHidInput"
    
    "NtUserInjectGesture"
    
    "NtUserInjectKeyboardInput"
    
    "NtUserInjectMouseInput"
    
    "NtUserInjectPointerInput"
    
    "NtUserInjectTouchInput"
    
    "NtUserInteractiveControlQueryUsage"
    
    "NtUserInternalGetWindowIcon"
    
    "NtUserIsChildWindowDpiMessageEnabled"
    
    "NtUserIsMouseInPointerEnabled"
    
    "NtUserIsMouseInputEnabled"
    
    "NtUserIsNonClientDpiScalingEnabled"
    
    "NtUserIsResizeLayoutSynchronizationEnabled"
    
    "NtUserIsTopLevelWindow"
    
    "NtUserIsTouchWindow"
    
    "NtUserIsWindowBroadcastingDpiToChildren"
    
    "NtUserIsWindowGDIScaledDpiMessageEnabled"
    
    "NtUserLayoutCompleted"
    
    "NtUserLinkDpiCursor"
    
    "NtUserLoadKeyboardLayoutEx"
    
    "NtUserLockCursor"
    
    "NtUserLockWindowStation"
    
    "NtUserLockWorkStation"
    
    "NtUserLogicalToPerMonitorDPIPhysicalPoint"
    
    "NtUserLogicalToPhysicalDpiPointForWindow"
    
    "NtUserLogicalToPhysicalPoint"
    
    "NtUserMNDragLeave"
    
    "NtUserMNDragOver"
    
    "NtUserMagControl"
    
    "NtUserMagGetContextInformation"
    
    "NtUserMagSetContextInformation"
    
    "NtUserMapPointsByVisualIdentifier"
    
    "NtUserMenuItemFromPoint"
    
    "NtUserMinMaximize"
    
    "NtUserModifyWindowTouchCapability"
    
    "NtUserMsgWaitForMultipleObjectsEx"
    
    "NtUserNavigateFocus"
    
    "NtUserNotifyIMEStatus"
    
    "NtUserOpenInputDesktop"
    
    "NtUserOpenThreadDesktop"
    
    "NtUserPaintMonitor"
    
    "NtUserPerMonitorDPIPhysicalToLogicalPoint"
    
    "NtUserPhysicalToLogicalDpiPointForWindow"
    
    "NtUserPhysicalToLogicalPoint"
    
    "NtUserPrintWindow"
    
    "NtUserProcessInkFeedbackCommand"
    
    "NtUserPromoteMouseInPointer"
    
    "NtUserPromotePointer"
    
    "NtUserQueryActivationObject"
    
    "NtUserQueryBSDRWindow"
    
    "NtUserQueryDisplayConfig"
    
    "NtUserQueryInformationThread"
    
    "NtUserQueryInputContext"
    
    "NtUserQuerySendMessage"
    
    "NtUserRealChildWindowFromPoint"
    
    "NtUserRealWaitMessageEx"
    
    "NtUserRegisterBSDRWindow"
    
    "NtUserRegisterDManipHook"
    
    "NtUserRegisterEdgy"
    
    "NtUserRegisterErrorReportingDialog"
    
    "NtUserRegisterHotKey"
    
    "NtUserRegisterManipulationThread"
    
    "NtUserRegisterPointerDeviceNotifications"
    
    "NtUserRegisterPointerInputTarget"
    
    "NtUserRegisterRawInputDevices"
    
    "NtUserRegisterServicesProcess"
    
    "NtUserRegisterSessionPort"
    
    "NtUserRegisterShellPTPListener"
    
    "NtUserRegisterTasklist"
    
    "NtUserRegisterTouchHitTestingWindow"
    
    "NtUserRegisterTouchPadCapable"
    
    "NtUserRegisterUserApiHook"
    
    "NtUserReleaseDC"
    
    "NtUserReleaseDwmHitTestWaiters"
    
    "NtUserRemoteConnect"
    
    "NtUserRemoteRedrawRectangle"
    
    "NtUserRemoteRedrawScreen"
    
    "NtUserRemoteStopScreenUpdates"
    
    "NtUserRemoveClipboardFormatListener"
    
    "NtUserRemoveInjectionDevice"
    
    "NtUserRemoveVisualIdentifier"
    
    "NtUserReportInertia"
    
    "NtUserRequestMoveSizeOperation"
    
    "NtUserResolveDesktopForWOW"
    
    "NtUserRestoreWindowDpiChanges"
    
    "NtUserSendEventMessage"
    
    "NtUserSendInteractiveControlHapticsReport"
    
    "NtUserSetActivationFilter"
    
    "NtUserSetActiveProcessForMonitor"
    
    "NtUserSetAppImeLevel"
    
    "NtUserSetAutoRotation"
    
    "NtUserSetBridgeWindowChild"
    
    "NtUserSetBrokeredForeground"
    
    "NtUserSetCalibrationData"
    
    "NtUserSetChildWindowNoActivate"
    
    "NtUserSetClassWord"
    
    "NtUserSetCoreWindow"
    
    "NtUserSetCoreWindowPartner"
    
    "NtUserSetCursorContents"
    
    "NtUserSetCursorPos"
    
    "NtUserSetDesktopColorTransform"
    
    "NtUserSetDialogControlDpiChangeBehavior"
    
    "NtUserSetDisplayAutoRotationPreferences"
    
    "NtUserSetDisplayConfig"
    
    "NtUserSetDisplayMapping"
    
    "NtUserSetFallbackForeground"
    
    "NtUserSetFeatureReportResponse"
    
    "NtUserSetForegroundWindowForApplication"
    
    "NtUserSetGestureConfig"
    
    "NtUserSetImeHotKey"
    
    "NtUserSetImeInfoEx"
    
    "NtUserSetImeOwnerWindow"
    
    "NtUserSetInteractiveControlFocus"
    
    "NtUserSetInteractiveCtrlRotationAngle"
    
    "NtUserSetInternalWindowPos"
    
    "NtUserSetLayeredWindowAttributes"
    
    "NtUserSetMagnificationDesktopMagnifierOffsetsDWMUpdated"
    
    "NtUserSetManipulationInputTarget"
    
    "NtUserSetMenu"
    
    "NtUserSetMenuContextHelpId"
    
    "NtUserSetMenuFlagRtoL"
    
    "NtUserSetMirrorRendering"
    
    "NtUserSetObjectInformation"
    
    "NtUserSetPrecisionTouchPadConfiguration"
    
    "NtUserSetProcessDpiAwarenessContext"
    
    "NtUserSetProcessInteractionFlags"
    
    "NtUserSetProcessMousewheelRoutingMode"
    
    "NtUserSetProcessRestrictionExemption"
    
    "NtUserSetProcessUIAccessZorder"
    
    "NtUserSetShellWindowEx"
    
    "NtUserSetSysColors"
    
    "NtUserSetSystemCursor"
    
    "NtUserSetSystemTimer"
    
    "NtUserSetTargetForResourceBrokering"
    
    "NtUserSetThreadInputBlocked"
    
    "NtUserSetThreadLayoutHandles"
    
    "NtUserSetWindowArrangement"
    
    "NtUserSetWindowBand"
    
    "NtUserSetWindowCompositionAttribute"
    
    "NtUserSetWindowCompositionTransition"
    
    "NtUserSetWindowDisplayAffinity"
    
    "NtUserSetWindowFeedbackSetting"
    
    "NtUserSetWindowGroup"
    
    "NtUserSetWindowRgnEx"
    
    "NtUserSetWindowShowState"
    
    "NtUserSetWindowStationUser"
    
    "NtUserShowCursor"
    
    "NtUserShowSystemCursor"
    
    "NtUserShutdownBlockReasonCreate"
    
    "NtUserShutdownBlockReasonQuery"
    
    "NtUserShutdownReasonDestroy"
    
    "NtUserSignalRedirectionStartComplete"
    
    "NtUserSlicerControl"
    
    "NtUserSoundSentry"
    
    "NtUserStopAndEndInertia"
    
    "NtUserSwitchDesktop"
    
    "NtUserSystemParametersInfoForDpi"
    
    "NtUserTestForInteractiveUser"
    
    "NtUserTrackPopupMenuEx"
    
    "NtUserTransformPoint"
    
    "NtUserTransformRect"
    
    "NtUserUndelegateInput"
    
    "NtUserUnloadKeyboardLayout"
    
    "NtUserUnlockWindowStation"
    
    "NtUserUnregisterHotKey"
    
    "NtUserUnregisterSessionPort"
    
    "NtUserUnregisterUserApiHook"
    
    "NtUserUpdateDefaultDesktopThumbnail"
    
    "NtUserUpdateInputContext"
    
    "NtUserUpdateInstance"
    
    "NtUserUpdateLayeredWindow"
    
    "NtUserUpdatePerUserSystemParameters"
    
    "NtUserUpdateWindowInputSinkHints"
    
    "NtUserUpdateWindowTrackingInfo"
    
    "NtUserUserHandleGrantAccess"
    
    "NtUserValidateHandleSecure"
    
    "NtUserWOWCleanup"
    
    "NtUserWaitAvailableMessageEx"
    
    "NtUserWaitForInputIdle"
    
    "NtUserWaitForMsgAndEvent"
    
    "NtUserWaitForRedirectionStartComplete"
    
    "NtUserWindowFromDC"
    
    "NtUserWindowFromPhysicalPoint"
    
    "NtValidateCompositionSurfaceHandle"
    
    "NtVisualCaptureBits"
    
    "NtUserSetClassLongPtr"
    
    "NtUserSetWindowLongPtr"


};

ULONG_PTR SSDT_GetPfnAddr(ULONG dwIndex, PULONG lpBase)//https://bbs.kanxue.com/thread-248117.htm
{
    ULONG_PTR lpAddr = NULL;

    ULONG dwOffset = lpBase[dwIndex];

    //按16位对齐省空间，所以>>4;负偏移有+-问题，所以|0xF00..
    if (dwOffset & 0x80000000)
        dwOffset = (dwOffset >> 4) | 0xF0000000;
    else
        dwOffset >>= 4;

    lpAddr = (ULONG_PTR)((PUCHAR)lpBase + (LONG)dwOffset);

    return lpAddr;
}

NTSTATUS EnumSSDTFormMem(PSSDT_INFO SsdtBuffer, PULONG SsdtCount)//X64的SSDT是rva
{
    INIT_PDB;
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTable = (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTable");
    
    if (!KeServiceDescriptorTable) {
        Log("[XM] KeServiceDescriptorTable == null");
        return STATUS_UNSUCCESSFUL;
    }

    ULONG nums = KeServiceDescriptorTable->NumberOfServices;
    PULONG ssdt = KeServiceDescriptorTable->Base;
    *SsdtCount = nums;

    for (ULONG i = 0; i < nums; i++) {
        SsdtBuffer[i].Index = i;

        ULONG_PTR pfnAddr = SSDT_GetPfnAddr(i, ssdt);
        SsdtBuffer[i].FunctionAddress = (PVOID)pfnAddr;

        const char* functionName = ntos.GetNameByVA(pfnAddr);
        Log("[XM] GetName result: %s", functionName ? functionName : "(null)");

        if (functionName && strlen(functionName) > 0) {
            strcpy_s(SsdtBuffer[i].FunctionName, sizeof(SsdtBuffer[i].FunctionName), functionName);
        }
        else {
            // 如果没找到，使用索引号
            sprintf_s(SsdtBuffer[i].FunctionName, sizeof(SsdtBuffer[i].FunctionName), "Nt#%d", i);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS EnumShadowSSDT(PSSDT_INFO SsdtBuffer, PULONG SsdtCount)
{
    INIT_PDB;
                          
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTableArray =
    (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTableShadow");

    Log("[XM] KeServiceDescriptorTableShadow Array: %p", ShadowTableArray);

    if (!ShadowTableArray) {
        Log("[XM] KeServiceDescriptorTableShadow == null");
        return STATUS_UNSUCCESSFUL;
    }
    
    // 访问数组的第二个元素 [1] - 这才是真正的 ShadowSSDT                                     
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTable = &ShadowTableArray[1];
    
    Log("[XM] ShadowSSDT [0]: Base=%p, Count=%d",
        ShadowTableArray[0].Base, ShadowTableArray[0].NumberOfServices);
    Log("[XM] ShadowSSDT [1]: Base=%p, Count=%d",
        ShadowTable->Base, ShadowTable->NumberOfServices);
    
    if (!ShadowTable->Base || ShadowTable->NumberOfServices == 0) {
        Log("[XM] ShadowSSDT not available");
        Log("[XM] ShadowSSDT [1] not available");
    }

    ULONG nums = ShadowTable->NumberOfServices;
    PULONG shadowSsdt = ShadowTable->Base;
    *SsdtCount = nums;

    Log("[XM] ShadowSSDT found: %d services", nums);

    for (ULONG i = 0; i < nums; i++) {
        SsdtBuffer[i].Index = i + 0x1000;  // ShadowSSDT的调用号从0x1000开始

        // ShadowSSDT存储的是相对于win32k.sys的RVA，不需要解码
        ULONG_PTR pfnAddr = SSDT_GetPfnAddr(i, shadowSsdt);
        SsdtBuffer[i].FunctionAddress = (PVOID)pfnAddr;

        Log("[XM] ShadowSSDT[%d]: Raw=0x%X, Decoded=0x%p", i, shadowSsdt[i], pfnAddr);
    }

    return STATUS_SUCCESS;
}

ULONG_PTR RVA2FOA(PVOID fileBuffer, ULONG rva) {
    // 1. 获取PE头
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)fileBuffer;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        Log("[XM] 无效的DOS头");
        return 0;
    }

    PIMAGE_NT_HEADERS64 ntHeaders =
        (PIMAGE_NT_HEADERS64)((ULONG_PTR)fileBuffer + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        Log("[XM] 无效的PE头");
        return 0;
    }

    // 2. 获取节表
    PIMAGE_SECTION_HEADER sections =
        (PIMAGE_SECTION_HEADER)((ULONG_PTR)ntHeaders + sizeof(IMAGE_NT_HEADERS64));

    // 3. 遍历所有节
    for (USHORT i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER section = &sections[i];

        Log("[XM] 检查节[%d]: %s, VA=0x%x, VSize=0x%x, FOA=0x%x, FSize=0x%x",
            i, section->Name, section->VirtualAddress, section->Misc.VirtualSize,
            section->PointerToRawData, section->SizeOfRawData);

        // 4. 检查RVA是否在当前节范围内
        ULONG sectionStart = section->VirtualAddress;
        ULONG sectionEnd = section->VirtualAddress + section->Misc.VirtualSize;

        if (rva >= sectionStart && rva < sectionEnd) {
            // 5. 计算节内偏移
            ULONG offsetInSection = rva - sectionStart;

            // 6. 检查偏移是否超出文件中节的大小
            if (offsetInSection >= section->SizeOfRawData) {
                Log("[XM] RVA 0x%x 超出节 %s 的文件大小", rva, section->Name);
                return 0;
            }

            // 7. 计算文件偏移
            ULONG fileOffset = section->PointerToRawData + offsetInSection;

            Log("[XM] RVA转换成功: 0x%x -> 节%s -> FOA=0x%x",
                rva, section->Name, fileOffset);

            return fileOffset;
        }
    }

    Log("[XM] RVA 0x%x 不在任何节范围内", rva);
    return 0;
}


NTSTATUS EnumSSDTFromFile(PSSDT_INFO SsdtBuffer, PULONG SsdtCount) {
    *SsdtCount = 0;
    NTSTATUS status;
    HANDLE ntdllHandle = NULL;
    PVOID fileBuffer = NULL;
    ULONG fileSize = 0;

    do {
        // 打开ntdll.dll
        UNICODE_STRING fileName;
        RtlInitUnicodeString(&fileName, L"\\SystemRoot\\system32\\ntdll.dll");
        OBJECT_ATTRIBUTES objAttr;
        InitializeObjectAttributes(&objAttr, &fileName,
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

        IO_STATUS_BLOCK ioStatus;
        status = IoCreateFile(&ntdllHandle,
            FILE_READ_ATTRIBUTES | SYNCHRONIZE,
            &objAttr, &ioStatus, 0,
            FILE_READ_ATTRIBUTES, FILE_SHARE_READ,
            FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0, CreateFileTypeNone, NULL,
            IO_NO_PARAMETER_CHECKING);

        if (!NT_SUCCESS(status)) {
            Log("[XM] 打开ntdll.dll失败: 0x%x", status);
            break;
        }

        // 获取文件大小
        FILE_STANDARD_INFORMATION fileInfo;
        status = ZwQueryInformationFile(ntdllHandle, &ioStatus, &fileInfo,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation);
        if (!NT_SUCCESS(status) || fileInfo.EndOfFile.HighPart != 0) {
            Log("[XM] 获取文件信息失败或文件过大");
            break;
        }

        fileSize = fileInfo.EndOfFile.LowPart;

        // 分配内存并读取文件
        fileBuffer = ExAllocatePoolWithTag(PagedPool, fileSize + 0x100, 'SsDt');
        if (!fileBuffer) {
            Log("[XM] 内存分配失败");
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        LARGE_INTEGER byteOffset = { 0 };
        status = ZwReadFile(ntdllHandle, NULL, NULL, NULL, &ioStatus,
            fileBuffer, fileSize, &byteOffset, NULL);
        if (!NT_SUCCESS(status)) {
            Log("[XM] 读取文件失败: 0x%x", status);
            break;
        }

        // 解析PE结构
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)fileBuffer;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            Log("[XM] 无效的DOS头");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)fileBuffer + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            Log("[XM] 无效的NT头");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        // 检查是否有导出表
        if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0) {
            Log("[XM] 没有导出表");
            status = STATUS_NOT_FOUND;
            break;
        }

        // 获取节表
        PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)((ULONG_PTR)ntHeaders +
            sizeof(IMAGE_NT_HEADERS));

        // 定位导出表
        ULONG exportRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        ULONG_PTR exportFileOffset = 0;

        // RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportRva >= section->VirtualAddress &&
                exportRva < section->VirtualAddress + section->SizeOfRawData) {
                exportFileOffset = exportRva - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        if (!exportFileOffset) {
            Log("[XM] 导出表RVA转换失败");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)fileBuffer + exportFileOffset);

        ULONG_PTR funcAddrOffset = 0, funcNameOffset = 0, ordinalOffset = 0;

        // AddressOfFunctions RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfFunctions >= section->VirtualAddress &&
                exportDir->AddressOfFunctions < section->VirtualAddress + section->SizeOfRawData) {
                funcAddrOffset = exportDir->AddressOfFunctions - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        // AddressOfNames RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfNames >= section->VirtualAddress &&
                exportDir->AddressOfNames < section->VirtualAddress + section->SizeOfRawData) {
                funcNameOffset = exportDir->AddressOfNames - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        // AddressOfNameOrdinals RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfNameOrdinals >= section->VirtualAddress &&
                exportDir->AddressOfNameOrdinals < section->VirtualAddress + section->SizeOfRawData) {
                ordinalOffset = exportDir->AddressOfNameOrdinals - section->VirtualAddress +
                    section->PointerToRawData;
                break;
            }
        }

        if (!funcAddrOffset || !funcNameOffset || !ordinalOffset) {
            Log("[XM] 导出表数组RVA转换失败");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PULONG addressOfFunctions = (PULONG)((ULONG_PTR)fileBuffer + funcAddrOffset);
        PULONG addressOfNames = (PULONG)((ULONG_PTR)fileBuffer + funcNameOffset);
        PUSHORT addressOfNameOrdinals = (PUSHORT)((ULONG_PTR)fileBuffer + ordinalOffset);

        // 遍历导出函数，查找Zw*函数
        ULONG ssdtIndex = 0;
        Log("[XM] 开始枚举ntdll.dll导出的Zw函数，总计%d个导出函数", exportDir->NumberOfNames);

        for (ULONG i = 0; i < exportDir->NumberOfNames ; i++) {
            // 获取函数名
            ULONG nameRva = addressOfNames[i];
            ULONG_PTR nameFileOffset = 0;

            // 函数名RVA转文件偏移
            for (UINT16 j = 0; j < ntHeaders->FileHeader.NumberOfSections; j++) {
                PIMAGE_SECTION_HEADER section = &sectionHeaders[j];
                if (nameRva >= section->VirtualAddress &&
                    nameRva < section->VirtualAddress + section->SizeOfRawData) {
                    nameFileOffset = nameRva - section->VirtualAddress + section->PointerToRawData;
                    break;
                }
            }

            if (!nameFileOffset) continue;

            PCHAR functionName = (PCHAR)((ULONG_PTR)fileBuffer + nameFileOffset);

            // 只处理Zw开头的函数
            if (functionName[0] != 'Z' || functionName[1] != 'w') {
                continue;
            }

            // 获取函数地址
            USHORT ordinal = addressOfNameOrdinals[i];
            ULONG funcRva = addressOfFunctions[ordinal];
            ULONG_PTR funcFileOffset = 0;

            // 函数地址RVA转文件偏移
            for (UINT16 j = 0; j < ntHeaders->FileHeader.NumberOfSections; j++) {
                PIMAGE_SECTION_HEADER section = &sectionHeaders[j];
                if (funcRva >= section->VirtualAddress &&
                    funcRva < section->VirtualAddress + section->SizeOfRawData) {
                    funcFileOffset = funcRva - section->VirtualAddress + section->PointerToRawData;
                    break;
                }
            }

            if (!funcFileOffset) continue;

            PUCHAR funcBytes = (PUCHAR)((ULONG_PTR)fileBuffer + funcFileOffset);

            // 从函数字节码中提取系统调用号
            /*
             * Zw函数的标准格式：
             * 4C 8B D1          mov r10, rcx
             * B8 XX XX XX XX    mov eax, syscall_number
             * 0F 05             syscall
             */

             // 验证函数头部特征
            if (funcBytes[0] == 0x4C && funcBytes[1] == 0x8B && funcBytes[2] == 0xD1 &&
                funcBytes[3] == 0xB8) {

                // 提取系统调用号
                ULONG syscallNumber = *(PULONG)(funcBytes + 4);

                // 填充SSDT_INFO结构
                PSSDT_INFO info = &SsdtBuffer[ssdtIndex];
                RtlZeroMemory(info, sizeof(SSDT_INFO));

                info->Index = syscallNumber;

                // 将Zw改为Nt并复制函数名
                CHAR ntName[64];
                ntName[0] = 'N';
                ntName[1] = 't';
                RtlStringCbCopyA(&ntName[2], sizeof(ntName) - 2, &functionName[2]);
                RtlStringCbCopyA(info->FunctionName, sizeof(info->FunctionName), ntName);

                //获取SSDT内存中的基址
                INIT_PDB;
                PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT =
                    (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTable");

                //将PE中拿到的表项RVA + 基址 ，转为VA
                if (pSSDT && pSSDT->Base && syscallNumber < pSSDT->NumberOfServices) {

                    //VA写入缓冲区发给R3
                    info->FunctionAddress = (PVOID)SSDT_GetPfnAddr(syscallNumber, pSSDT->Base);

                    Log("[XM] SSDT[%d] %s -> %p",
                        syscallNumber, info->FunctionName, info->FunctionAddress);
                }
                else {
                    Log("[XM] SSDT访问失败");
                }


                ssdtIndex++;
            }
        }

        *SsdtCount = ssdtIndex;
        status = STATUS_SUCCESS;

        Log("[XM] EnumSSDTFromFile完成，共找到%d个SSDT函数", ssdtIndex);

    } while (0);


    Log("[XM] EnumSSDTFromFile CLEAN_UP");

    if (fileBuffer) {
        ExFreePoolWithTag(fileBuffer, 'SsDt');
    }

    if (ntdllHandle) {
        ZwClose(ntdllHandle);
    }

    return status;
}

NTSTATUS GetOriginalSSDTFromFilePDB(PSSDT_INFO SsdtBuffer, PULONG SsdtCount) {
    *SsdtCount = 0;
    NTSTATUS status;
    HANDLE fileHandle = NULL;
    PVOID fileBuffer = NULL;

    do {
        // 1. 打开ntoskrnl.exe文件

        // 2. 解析PE头
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)fileBuffer;
        PIMAGE_NT_HEADERS64 ntHeaders = (PIMAGE_NT_HEADERS64)((ULONG_PTR)fileBuffer + dosHeader->e_lfanew);
        PIMAGE_SECTION_HEADER sections = (PIMAGE_SECTION_HEADER)((ULONG_PTR)ntHeaders + sizeof(IMAGE_NT_HEADERS64));

        // 3. 通过PDB获取KeServiceDescriptorTable的RVA
        INIT_PDB;
        ULONG_PTR ssdtVA = (ULONG_PTR)ntos.GetPointer("KeServiceDescriptorTable");
        ULONG_PTR kernelBase = ntos.GetModuleBase();

        // 计算RVA：VA - KernelBase
        ULONG ssdtRVA = (ULONG)(ssdtVA - kernelBase);
        Log("[XM] KeServiceDescriptorTable RVA: 0x%x", ssdtRVA);

        // 4. RVA转FOA
        ULONG_PTR ssdtFOA = RVA2FOA(ntHeaders, sections, ssdtRVA);
        if (!ssdtFOA) {
            Log("[XM] KeServiceDescriptorTable RVA转FOA失败");
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        Log("[XM] KeServiceDescriptorTable FOA: 0x%x", ssdtFOA);

        // 5. 从文件中读取KeServiceDescriptorTable结构
        PSYSTEM_SERVICE_DESCRIPTOR_TABLE fileSSDT =
            (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)((ULONG_PTR)fileBuffer + ssdtFOA);

        Log("[XM] 文件中的SSDT结构:");
        Log("[XM]   Base: %p", fileSSDT->Base);
        Log("[XM]   NumberOfServices: %d", fileSSDT->NumberOfServices);
        Log("[XM]   ServiceCounterTable: %p", fileSSDT->ServiceCounterTable);
        Log("[XM]   ParamTableBase: %p", fileSSDT->ParamTableBase);

        // 6. 计算SSDT数组的文件位置
        // 注意：fileSSDT->Base是基于文件ImageBase的地址
        ULONG_PTR fileImageBase = ntHeaders->OptionalHeader.ImageBase;
        ULONG ssdtArrayRVA = (ULONG)((ULONG_PTR)fileSSDT->Base - fileImageBase);

        ULONG_PTR ssdtArrayFOA = RVA2FOA(ntHeaders, sections, ssdtArrayRVA);
        if (!ssdtArrayFOA) {
            Log("[XM] SSDT数组RVA转FOA失败");
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        // 7. 读取文件中的原始SSDT数组
        PULONG originalSSDTArray = (PULONG)((ULONG_PTR)fileBuffer + ssdtArrayFOA);

        Log("[XM] SSDT数组文件偏移: 0x%x", ssdtArrayFOA);

        // 8. 处理每个SSDT表项
        for (ULONG i = 0; i < fileSSDT->NumberOfServices && i < 500; i++) {
            PSSDT_INFO info = &SsdtBuffer[i];
            info->Index = i;

            // 从文件读取原始编码值
            ULONG originalEncoded = originalSSDTArray[i];

            // 解码RVA
            ULONG originalRVA;
            if (originalEncoded & 0x80000000) {
                originalRVA = (originalEncoded >> 4) | 0xF0000000;
            }
            else {
                originalRVA = originalEncoded >> 4;
            }

            // 重定位到当前内核基址
            ULONG_PTR originalVA = kernelBase + originalRVA;
            info->FunctionAddress = (PVOID)originalVA;

            // 对比验证
            PVOID currentAddr = (PVOID)SSDT_GetPfnAddr(i, KeServiceDescriptorTable->Base);
            if (currentAddr != info->FunctionAddress) {
                Log("[XM] SSDT[%d] 不匹配: 当前=%p, 文件=%p",
                    i, currentAddr, info->FunctionAddress);
            }
        }

        *SsdtCount = fileSSDT->NumberOfServices;
        status = STATUS_SUCCESS;

    } while (0);

    // 清理
    if (fileBuffer) ExFreePool(fileBuffer);
    if (fileHandle) ZwClose(fileHandle);

    return status;
}
