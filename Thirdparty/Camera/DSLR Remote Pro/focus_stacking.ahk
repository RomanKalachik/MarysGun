;-----------------------------------------------------------------------
; AutoHotKey script to run DSLR Remote Pro for Windows, select live view and take
; a series of photos with different focus settings so that these can
; be combined using CombineZM to give extended depth of field.
; Please note: this script will only work with Canon EOS cameras that
; support live view e.g. Canon EOS 40D, Canon EOS-1D Mark III
;
; Usage: focus the lens on the nearest object to be in focus and then
; run this script. The number of photos in the sequence is defined by
; the variable NumberOfImagesInSequence below and can be overridden in
; the command line e.g. focus_stacking.ahk 30
; After taking the pictures run CombineZM to combine them into a single
; image with extended DoF.
; The shooting sequence can be aborted by closing the live view window.
;
; DSLR Remote Pro for Windows can be downloaded from
; http://www.breezesys.com
;
; AutoHotKey is free and can be downloaded from:
; http://www.autohotkey.com
;
; CombineZM is free and can be downloaded from:
; http://www.hadleyweb.pwp.blueyonder.co.uk/CZM/combinezm.htm
;
; This script comes with no warranty or support whatsoever and may
; be freely copied or modified as required.
;
; Written by Chris Breeze, www.breezesys.com
;-----------------------------------------------------------------------
SetTitleMatchMode,1
NumberOfImagesInSequence = 30

; button to press in live view window to adjust the focus after each shot:
; <<< - focus nearer, large step
; <<  - focus nearer, medium step
; <   - focus nearer, small step
; >   - focus further away, small step
; >>  - focus further away, medium step
; >>> - focus further away, large step
FocusCommand := ">>"

if %0% > 0 and %1% > 1
{
	NumberOfImagesInSequence = %1%
}

; First check DSLR Remote Pro for Windows is running and a camera is connected
IfWinExist, DSLR Remote Pro for Windows
{
	IfWinExist, DSLR Remote Pro for Windows - Connected to
	{
		WinActivate, DSLR Remote Pro for Windows
	}
	else
	{
		MsgBox, 48, Focus Stacking Script, DSLR Remote Pro for Windows is not connected to a camera
		ExitApp
	}
}
else
{
	MsgBox, 48, Focus Stacking Script, DSLR Remote Pro for Windows is not running
	ExitApp
}

; Turn on live view (this will fail if the camera doesn't support live view)
Sleep 1000
IfWinNotExist, Live View
{
	WinMenuSelectItem, DSLR Remote Pro for Windows,,Camera,Live View
	WinWait, Live View:,,6
	Sleep 1000
}
IfWinNotExist, Live View
{
	MsgBox, 48, Focus Stacking Script, Unable to select live view
	ExitApp
}

; make sure live view window has input focus
WinActivate, Live View
Sleep 250

; Take a series of pictures and adjust the focus after each shot
ImageCounter := 0
Loop, %NumberOfImagesInSequence%
{
	ImageCounter := ImageCounter + 1
	SplashTextOn, 200, 30, Focus Stack Progress , Image %ImageCounter% of %NumberOfImagesInSequence%

	; Make sure DSLR Remote Pro for Windows is still running. 
	IfWinExist, DSLR Remote Pro for Windows
	{
		IfWinNotExist, DSLR Remote Pro for Windows - Connected to
		{
			SplashTextOff
			MsgBox, 48, Focus Stacking Script, No camera connected in loop!`n`n%ImageCounter% frames shot.
			ExitApp
		}
	}
	else
	{
		SplashTextOff
		MsgBox, 48, Focus Stacking Script, DSLR Remote Pro for Windows is not running.`n`n%ImageCounter% frames shot.
		ExitApp
	}
        IfWinNotExist, Live View
	{
		SplashTextOff
		MsgBox, 48, Focus Stacking Script, Live View window is not open.`n`n %ImageCounter% frames shot.
		ExitApp
	}

	ControlClick, Release, Live View
	Sleep 4000	; give DSLR Remote Pro for Windows time to take the picture and download

	ControlClick, %FocusCommand%, Live View
	Sleep 400
}
SplashTextOff
