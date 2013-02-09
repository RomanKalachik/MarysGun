;---------------------------------------------------------------------------
; Sample AutoHotkey script to synchronize the green screen background in
; Green Screen Wizard Pro Batch with the one selected in the full screen
; photobooth window of DSLR Remote Pro for Windows.
;
; AutoHotKey is free and can be downloaded from:
; http://www.autohotkey.com
;
; DSLR Remote Pro for Windows can be downloaded from:
; http://www.breezesys.com/DSLRRemotePro/index.htm
;
; Green Screen Wizard Pro Batch can be downloaded from:
; http://www.greenscreenwizard.com/GreenScreenWizardPro.aspx
;
; This script works with:
; DSLR Remote Pro for Windows v2.0 and Green Screen Wizard Pro Batch 5.0
;
; This script comes with no warranty or support whatsoever and may
; be freely copied or modified as required.
;
; Written by Chris Breeze, Breeze Systems Ltd
; www.breezesys.com
;---------------------------------------------------------------------------

BoothWindowTitle := "Breeze Systems Photobooth"
GswWindowTitle := "Green Screen Wizard Professional"

#Persistent
DetectHiddenText, on
SetTimer, SyncGreenScreenWizard, 500
Return

; this is run every 500ms
SyncGreenScreenWizard:
IfWinExist,%BoothWindowTitle%
{
   ; read the pathname of the green screen background from
   ; the full screen photobooth window
   WinGetText,image,%BoothWindowTitle%,Green screen background:
   image := RegExReplace(image, "Green screen background: ")
   image := RegExReplace(image, "\s*$")

   ; load background into GSW if it has changed
   if (image and image <> currentImage)
   {
      ControlClick,Load Background,%GswWindowTitle%,,Left,,
      WinWait,Select Background Image
      ControlSetText, Edit1,%image%,Select Background Image
      ControlClick,&Open,Select Background Image,,Left,,
      currentImage := image
      WinActivate,%BoothWindowTitle%
   }
}
return
