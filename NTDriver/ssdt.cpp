#include "ssdt.h"
#include <ntimage.h>
#include "pe.h"


// Win7 ShadowSSDT 函数名数组
 char* WIN7_SHADOW_SSDT_FUNCTIONS[] = {
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
 "DxgStubGenericThunk",
 "DxgStubEnableDirectDraw",
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
 "NtUserSetWindowLongPtr",
};

char* IDT_0x13_FUNCTIONS[] = {
     "KiDivideErrorFault",
     "KiDebugTrapOrFault",
     "KiNmiInterrupt",
     "KiBreakpointTrap",
     "KiOverflowTrap",
     "KiBoundFault",
     "KiInvalidOpcodeFault",
     "KiNpxNotAvailableFault",
     "KiDoubleFaultAbort",
     "KiNpxSegmentOverrunAbort",
     "KiInvalidTssFault",
     "KiSegmentNotPresentFault",
     "KiStackFault",
     "KiGeneralProtectionFault",
     "KiPageFault",
     "KiIsrThunk",
     "KiFloatingErrorFault",
     "KiAlignmentFault",
     "KiMcheckAbort",
     "KiXmmException"
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

//VA转成SSDT表项的值
ULONG SSDT_EncodePfnAddr(ULONG_PTR FunctionAddress, PULONG SsdtBase)
{
    // 计算函数地址相对于SSDT基址的偏移                                          
    LONG_PTR offset = (LONG_PTR)FunctionAddress - (LONG_PTR)SsdtBase;

    // SSDT编码格式：(offset << 4) | sign_bit                                    
    // 如果偏移为负数，需要设置符号位                                            
    ULONG encodedValue;
    if (offset < 0) {
        // 负偏移：左移4位并设置最高位为1                                        
        encodedValue = ((ULONG)(-offset) << 4) | 0x80000000;
    }
    else {
        // 正偏移：直接左移4位                                                   
        encodedValue = (ULONG)(offset << 4);
    }

    Log("[XM] SSDT编码: FuncAddr=%p, SsdtBase=%p, Offset=0x%x, Encoded=0x%08X",
        FunctionAddress, SsdtBase, offset, encodedValue);

    return encodedValue;
}

NTSTATUS EnumSSDTFromMem(PSSDT_INFO SsdtBuffer, PULONG SsdtCount)//X64的SSDT是rva
{
    INIT_NTOS;
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
    INIT_NTOS;
                          
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

        ULONG_PTR pfnAddr = SSDT_GetPfnAddr(i, shadowSsdt);
        SsdtBuffer[i].FunctionAddress = (PVOID)pfnAddr;

        Log("[XM] ShadowSSDT[%d]: Raw=0x%X, Decoded=0x%p", i, shadowSsdt[i], pfnAddr);
    }

    return STATUS_SUCCESS;
}

//解析PE导出表  特征Zw*函数的服务号 得到索引与函数名的映射关系 函数名查pdb获得rva 
NTSTATUS RecoverSSDT() {
    NTSTATUS status;
    HANDLE ntdllHandle = NULL;
    PVOID fileBuffer = NULL;
    ULONG fileSize = 0;

    INIT_NTOS;

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
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)fileBuffer + dosHeader->e_lfanew);
        PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)((ULONG_PTR)ntHeaders +
            sizeof(IMAGE_NT_HEADERS));

        // 定位导出表
        ULONG exportRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        ULONG_PTR exportFOA = 0;

        // RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportRva >= section->VirtualAddress &&
                exportRva < section->VirtualAddress + section->SizeOfRawData) {
                exportFOA = exportRva - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        if (!exportFOA) {
            Log("[XM] 导出表RVA转换失败");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)fileBuffer + exportFOA);

        ULONG_PTR funcAddrFOA = 0, funcNameFOA = 0, ordinalFOA = 0;

        // AddressOfFunctions RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfFunctions >= section->VirtualAddress &&
                exportDir->AddressOfFunctions < section->VirtualAddress + section->SizeOfRawData) {
                funcAddrFOA = exportDir->AddressOfFunctions - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        // AddressOfNames RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfNames >= section->VirtualAddress &&
                exportDir->AddressOfNames < section->VirtualAddress + section->SizeOfRawData) {
                funcNameFOA = exportDir->AddressOfNames - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        // AddressOfNameOrdinals RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfNameOrdinals >= section->VirtualAddress &&
                exportDir->AddressOfNameOrdinals < section->VirtualAddress + section->SizeOfRawData) {
                ordinalFOA = exportDir->AddressOfNameOrdinals - section->VirtualAddress +
                    section->PointerToRawData;
                break;
            }
        }

        if (!funcAddrFOA || !funcNameFOA || !ordinalFOA) {
            Log("[XM] 导出表数组RVA转换失败");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PULONG addressOfFunctions = (PULONG)((ULONG_PTR)fileBuffer + funcAddrFOA);
        PULONG addressOfNames = (PULONG)((ULONG_PTR)fileBuffer + funcNameFOA);
        PUSHORT addressOfNameOrdinals = (PUSHORT)((ULONG_PTR)fileBuffer + ordinalFOA);

        // 遍历导出函数，查找Zw*函数
        ULONG ssdtIndex = 0;
        Log("[XM] 开始枚举ntdll.dll导出的Zw函数，总计%d个导出函数", exportDir->NumberOfNames);

        for (ULONG i = 0; i < exportDir->NumberOfNames ; i++) {
            // 获取函数名
            ULONG nameRva = addressOfNames[i];
            ULONG_PTR nameFOA = 0;

            // 函数名RVA转文件偏移
            for (UINT16 j = 0; j < ntHeaders->FileHeader.NumberOfSections; j++) {
                PIMAGE_SECTION_HEADER section = &sectionHeaders[j];
                if (nameRva >= section->VirtualAddress &&
                    nameRva < section->VirtualAddress + section->SizeOfRawData) {
                    nameFOA = nameRva - section->VirtualAddress + section->PointerToRawData;
                    break;
                }
            }

            if (!nameFOA) continue;

            PCHAR functionName = (PCHAR)((ULONG_PTR)fileBuffer + nameFOA);

            // 只处理Zw开头的函数
            if (functionName[0] != 'Z' || functionName[1] != 'w') {
                continue;
            }

            // 获取函数地址
            USHORT ordinal = addressOfNameOrdinals[i];
            ULONG funcRva = addressOfFunctions[ordinal];
            ULONG_PTR funcFOA = 0;

            // 函数地址RVA转文件偏移
            for (UINT16 j = 0; j < ntHeaders->FileHeader.NumberOfSections; j++) {
                PIMAGE_SECTION_HEADER section = &sectionHeaders[j];
                if (funcRva >= section->VirtualAddress &&
                    funcRva < section->VirtualAddress + section->SizeOfRawData) {
                    funcFOA = funcRva - section->VirtualAddress + section->PointerToRawData;
                    break;
                }
            }

            if (!funcFOA) continue;

            PUCHAR funcBytes = (PUCHAR)((ULONG_PTR)fileBuffer + funcFOA);

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

                // 将Zw改为Nt并复制函数名
                CHAR ntName[64];
                ntName[0] = 'N';
                ntName[1] = 't';
                RtlStringCbCopyA(&ntName[2], sizeof(ntName) - 2, &functionName[2]);

                PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT =
                    (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTable");

                //pdb获取函数RVA
                ULONG_PTR originalRVA = ntos.GetPointerRVA(ntName);

                //RVA+BASE 得到函数VA   
                ULONG_PTR pdb_ssdtItemVA = ntos.GetModuleBase() + originalRVA;

                // 内存SSDT表项中的函数地址  这个是通过表在内存中的数据得到的VA     
                ULONG_PTR mem_ssdtItemVA = SSDT_GetPfnAddr(syscallNumber, pSSDT->Base);

                //如果表项被修改过
                if (pdb_ssdtItemVA != mem_ssdtItemVA) {
                    Log("不相等 mem_ssdtItemVA:%p pdb_ssdtItemVA:%p", mem_ssdtItemVA, pdb_ssdtItemVA);

                    //拿正确的VA反向得出偏移
                    ULONG ssdtItem = SSDT_EncodePfnAddr(pdb_ssdtItemVA, pSSDT->Base);

                    //写回SSDT
                    EnableWrite();
                    pSSDT->Base[syscallNumber] = ssdtItem;
                    DisableWrite();
                    
                
                }
                else {
                    Log("相等  mem_ssdtItemVA:%p pdb_ssdtItemVA:%p", mem_ssdtItemVA, pdb_ssdtItemVA);
                }
                
                
                ssdtIndex++;
            }
        }

        status = STATUS_SUCCESS;

        Log("[XM] RecoverSSDT完成，共找到%d个SSDT函数", ssdtIndex);

    } while (0);


    Log("[XM] RecoverSSDT CLEAN_UP");

    if (fileBuffer) {
        ExFreePoolWithTag(fileBuffer, 'SsDt');
    }

    if (ntdllHandle) {
        ZwClose(ntdllHandle);
    }

    return status;
}

NTSTATUS RecoverShadowSSDT_Win7() {
    NTSTATUS status = STATUS_SUCCESS;
    INIT_NTOS;
    INIT_WIN32K;

    // 1. 获取ShadowSSDT指针
    PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTableArray =
        (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTableShadow");
    if (!ShadowTableArray) {
        Log("[XM] 获取KeServiceDescriptorTableShadow失败\n");
        return STATUS_UNSUCCESSFUL;
    }

    PSYSTEM_SERVICE_DESCRIPTOR_TABLE pShadowSSDT = &ShadowTableArray[1]; // ShadowSSDT是数组[1]
    if (!pShadowSSDT || !pShadowSSDT->Base) {
        Log("[XM] ShadowSSDT Base为空\n");
        return STATUS_UNSUCCESSFUL;
    }

    // 2. 遍历WIN7_SHADOW_SSDT_FUNCTIONS映射表
    ULONG restoredCount = 0;
    ULONG tableSize = sizeof(WIN7_SHADOW_SSDT_FUNCTIONS) / sizeof(WIN7_SHADOW_SSDT_FUNCTIONS[0]);

    Log("[XM] 开始Win7 ShadowSSDT恢复，映射表大小: %d\n", tableSize);

    for (ULONG i = 0; i < tableSize; i++) {
        ULONG shadowIndex = i;
        const char* functionName = WIN7_SHADOW_SSDT_FUNCTIONS[i];

        // 3. 通过PDB获取函数原始地址
        ULONG_PTR pdb_ssdtItemVA = win32k.GetPointer(functionName);
        if (!pdb_ssdtItemVA) {
            Log("[XM] 无法获取函数地址: %s\n", functionName);
            continue;
        }

        // 4. 获取当前ShadowSSDT中的地址
        ULONG_PTR mem_ssdtItemVA = SSDT_GetPfnAddr(shadowIndex, pShadowSSDT->Base);

        // 5. 对比检测Hook
        if (pdb_ssdtItemVA != mem_ssdtItemVA) {
            // 发现Hook，进行恢复
            ULONG ssdtItem = SSDT_EncodePfnAddr(pdb_ssdtItemVA, pShadowSSDT->Base);

            // 写回SSDT
            EnableWrite();
            pShadowSSDT->Base[shadowIndex] = ssdtItem;
            DisableWrite();

            restoredCount++;
            Log("[XM] ShadowSSDT[%d] %s Hook已恢复: 0x%p -> 0x%p\n",
                shadowIndex, functionName, mem_ssdtItemVA, pdb_ssdtItemVA);
        }
    }

    Log("[XM] Win7 ShadowSSDT恢复完成，共恢复 %d 个函数\n", restoredCount);
    return status;
}

NTSTATUS RecoverShadowSSDT() {

    // 检测系统版本
    RTL_OSVERSIONINFOW osVersion = { 0 };
    osVersion.dwOSVersionInfoSize = sizeof(osVersion);
    RtlGetVersion(&osVersion);

    if (osVersion.dwMajorVersion == 6 && osVersion.dwMinorVersion == 1) {
        Log("[XM] 检测到Windows 7，使用静态映射表恢复ShadowSSDT\n");
        return RecoverShadowSSDT_Win7();
    }
    else {
        Log("[XM] 使用win32u.dll恢复ShadowSSDT\n");
        return RecoverShadowSSDT_win32u();
    }
}

NTSTATUS RecoverShadowSSDT_win32u() {
    NTSTATUS status;
    HANDLE win32uHandle = NULL;
    PVOID fileBuffer = NULL;
    ULONG fileSize = 0;

    INIT_NTOS;
    INIT_WIN32K;

    do {
        UNICODE_STRING fileName;
        RtlInitUnicodeString(&fileName, L"\\SystemRoot\\system32\\win32u.dll");
        OBJECT_ATTRIBUTES objAttr;
        InitializeObjectAttributes(&objAttr, &fileName,
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

        IO_STATUS_BLOCK ioStatus;
        status = IoCreateFile(&win32uHandle,
            FILE_READ_ATTRIBUTES | SYNCHRONIZE,
            &objAttr, &ioStatus, 0,
            FILE_READ_ATTRIBUTES, FILE_SHARE_READ,
            FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0, CreateFileTypeNone, NULL,
            IO_NO_PARAMETER_CHECKING);

        if (!NT_SUCCESS(status)) {
            Log("[XM] 打开win32u.dll失败: 0x%x", status);
            break;
        }

        // 获取文件大小
        FILE_STANDARD_INFORMATION fileInfo;
        status = ZwQueryInformationFile(win32uHandle, &ioStatus, &fileInfo,
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
        status = ZwReadFile(win32uHandle, NULL, NULL, NULL, &ioStatus,
            fileBuffer, fileSize, &byteOffset, NULL);
        if (!NT_SUCCESS(status)) {
            Log("[XM] 读取文件失败: 0x%x", status);
            break;
        }

        // 解析PE结构
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)fileBuffer;
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)fileBuffer + dosHeader->e_lfanew);
        PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)((ULONG_PTR)ntHeaders +
            sizeof(IMAGE_NT_HEADERS));

        // 定位导出表
        ULONG exportRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        ULONG_PTR exportFOA = 0;

        // RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportRva >= section->VirtualAddress &&
                exportRva < section->VirtualAddress + section->SizeOfRawData) {
                exportFOA = exportRva - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        if (!exportFOA) {
            Log("[XM] 导出表RVA转换失败");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)fileBuffer + exportFOA);

        ULONG_PTR funcAddrFOA = 0, funcNameFOA = 0, ordinalFOA = 0;

        // AddressOfFunctions RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfFunctions >= section->VirtualAddress &&
                exportDir->AddressOfFunctions < section->VirtualAddress + section->SizeOfRawData) {
                funcAddrFOA = exportDir->AddressOfFunctions - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        // AddressOfNames RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfNames >= section->VirtualAddress &&
                exportDir->AddressOfNames < section->VirtualAddress + section->SizeOfRawData) {
                funcNameFOA = exportDir->AddressOfNames - section->VirtualAddress + section->PointerToRawData;
                break;
            }
        }

        // AddressOfNameOrdinals RVA转文件偏移
        for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
            PIMAGE_SECTION_HEADER section = &sectionHeaders[i];
            if (exportDir->AddressOfNameOrdinals >= section->VirtualAddress &&
                exportDir->AddressOfNameOrdinals < section->VirtualAddress + section->SizeOfRawData) {
                ordinalFOA = exportDir->AddressOfNameOrdinals - section->VirtualAddress +
                    section->PointerToRawData;
                break;
            }
        }

        if (!funcAddrFOA || !funcNameFOA || !ordinalFOA) {
            Log("[XM] 导出表数组RVA转换失败");
            status = STATUS_INVALID_IMAGE_FORMAT;
            break;
        }

        PULONG addressOfFunctions = (PULONG)((ULONG_PTR)fileBuffer + funcAddrFOA);
        PULONG addressOfNames = (PULONG)((ULONG_PTR)fileBuffer + funcNameFOA);
        PUSHORT addressOfNameOrdinals = (PUSHORT)((ULONG_PTR)fileBuffer + ordinalFOA);

        // 遍历导出函数，查找Nt*函数
        ULONG ssdtIndex = 0;
        Log("[XM] 开始枚举win32u.dll导出的Nt函数，总计%d个", exportDir->NumberOfNames);

        for (ULONG i = 0; i < exportDir->NumberOfNames; i++) {
            // 获取函数名
            ULONG nameRva = addressOfNames[i];
            ULONG_PTR nameFOA = 0;

            // 函数名RVA转文件偏移
            for (UINT16 j = 0; j < ntHeaders->FileHeader.NumberOfSections; j++) {
                PIMAGE_SECTION_HEADER section = &sectionHeaders[j];
                if (nameRva >= section->VirtualAddress &&
                    nameRva < section->VirtualAddress + section->SizeOfRawData) {
                    nameFOA = nameRva - section->VirtualAddress + section->PointerToRawData;
                    break;
                }
            }

            if (!nameFOA) continue;

            PCHAR functionName = (PCHAR)((ULONG_PTR)fileBuffer + nameFOA);

            // 只处理Nt开头的函数
            if (functionName[0] != 'N' || functionName[1] != 't') {
                continue;
            }

            // 获取函数地址
            USHORT ordinal = addressOfNameOrdinals[i];
            ULONG funcRva = addressOfFunctions[ordinal];
            ULONG_PTR funcFOA = 0;

            // 函数地址RVA转文件偏移
            for (UINT16 j = 0; j < ntHeaders->FileHeader.NumberOfSections; j++) {
                PIMAGE_SECTION_HEADER section = &sectionHeaders[j];
                if (funcRva >= section->VirtualAddress &&
                    funcRva < section->VirtualAddress + section->SizeOfRawData) {
                    funcFOA = funcRva - section->VirtualAddress + section->PointerToRawData;
                    break;
                }
            }

            if (!funcFOA) continue;

            PUCHAR funcBytes = (PUCHAR)((ULONG_PTR)fileBuffer + funcFOA);

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

                PSYSTEM_SERVICE_DESCRIPTOR_TABLE ShadowTableArray =
                    (PSYSTEM_SERVICE_DESCRIPTOR_TABLE)ntos.GetPointer("KeServiceDescriptorTableShadow");
                PSYSTEM_SERVICE_DESCRIPTOR_TABLE pShadowSSDT = &ShadowTableArray[1];

                // 提取系统调用号 
                ULONG syscallNumber = *(PULONG)(funcBytes + 4);
                ULONG shadowIndex = syscallNumber - 0x1000;
                if (shadowIndex >= pShadowSSDT->NumberOfServices) {
                    Log("[XM] shadowIndex: %d, NumberOfServices: %d", shadowIndex, pShadowSSDT->NumberOfServices);
                    continue;
                }

                //pdb获取函数RVA
                ULONG_PTR originalRVA = win32k.GetPointerRVA(functionName);

                //RVA+BASE 得到函数VA   
                ULONG_PTR pdb_ssdtItemVA = win32k.GetModuleBase() + originalRVA;

                // 内存SSDT表项中的函数地址  这个是通过表在内存中的数据得到的VA     
                ULONG_PTR mem_ssdtItemVA = SSDT_GetPfnAddr(shadowIndex, pShadowSSDT->Base);

                if (pdb_ssdtItemVA != mem_ssdtItemVA) {
                    Log("不相等 mem_ssdtItemVA:%p pdb_ssdtItemVA:%p", mem_ssdtItemVA, pdb_ssdtItemVA);

                    //拿正确的VA反向得出偏移
                    ULONG ssdtItem = SSDT_EncodePfnAddr(pdb_ssdtItemVA, pShadowSSDT->Base);

                    //写回SSDT
                    EnableWrite();
                    pShadowSSDT->Base[shadowIndex] = ssdtItem;
                    DisableWrite();


                }
                else {
                    Log("相等  mem_shadowItemVA:%p pdb_shadowItemVA:%p", mem_ssdtItemVA, pdb_ssdtItemVA);
                }


                ssdtIndex++;
            }
        }

        status = STATUS_SUCCESS;

        Log("[XM] RecoverSSDT完成，共找到%d个SSDT函数", ssdtIndex);

    } while (0);


    Log("[XM] RecoverSSDT CLEAN_UP");

    if (fileBuffer) {
        ExFreePoolWithTag(fileBuffer, 'SsDt');
    }

    if (win32uHandle) {
        ZwClose(win32uHandle);
    }


    return status;
}

