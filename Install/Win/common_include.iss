; common script functions and strings for windows 7+ installers    

#define ISSI_Splash_T 1
#define ISSI_Splash_X 600
#define ISSI_Splash_Y 200
;#define ISSI_Splash_CORNER 100
#define ISSI_IncludePath "C:\ISSI"
;#include "C:\ISSI\_issi.isi"  

#include "scripts\products\stringversion.iss"
#include "scripts\products\winversion.iss"  
#include "scripts\products\msiproduct.iss"
#include "scripts\products\iis.iss"
#include "scripts\products.iss"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{#MyAppID}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes  
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayIcon={app}\{#MyAppExeName}
DisableDirPage=auto
DisableProgramGroupPage=auto

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "corsican"; MessagesFile: "compiler:Languages\Corsican.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "de"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
;Name: "greek"; MessagesFile: "compiler:Languages\Greek.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
;Name: "serbiancyrillic"; MessagesFile: "compiler:Languages\SerbianCyrillic.isl"
;Name: "serbianlatin"; MessagesFile: "compiler:Languages\SerbianLatin.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"
               
[Files]               
; MSVC 2017 Redistributable
Source: ".\bin\VC_redist.x64.exe"; DestDir: {app}; Flags: deleteafterinstall

[Run]
Filename: "{app}\VC_redist.x64.exe"; Check: VCRedistNeedsInstall; Parameters: "/passive /norestart"; StatusMsg: Installing VC++ 2022 Redistributable...; Flags: waituntilterminated
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent 
    
[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon
                     
[Code]   
procedure DeleteFilesInFolder(const FolderPath, Extension: string);
var
  FindRec: TFindRec;
begin
  if FindFirst(FolderPath + '\*' + Extension, FindRec) then
  begin
    repeat
      DeleteFile(FolderPath + '\' + FindRec.Name);
    until not FindNext(FindRec);
    FindClose(FindRec);
  end;
end;

const
	vcredist2022_productcode_x64 = '{d8bbe9f9-7c5b-42c6-b715-9ee898a2e515}';
  msvc_hkey_location = 'SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\';
  fr64_hkey_location = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\';function VCRedistNeedsInstall: Boolean;
begin
  // here the Result must be True when you need to install your VCRedist
  // or False when you don't need to, so now it's upon you how you build
  // this statement, the following won't install your VC redist only when
  // the Visual C++ 2010 Redist (x86) and Visual C++ 2010 SP1 Redist(x86)
  // are installed for the current user
  Result := not msiproduct(vcredist2022_productcode_x64) and
        not RegKeyExists(HKEY_LOCAL_MACHINE, msvc_hkey_location + vcredist2022_productcode_x64);
end;
function BeginSetup(MyAppName: string; MyAppID: string; MyAppVersion: string): boolean;      
var
Version: String;
begin
	  //init windows version
	  initwinversion();
    //InitializeSetupISSI(); 
         
	  Result := true;     
    //if the program is already on the machine
    if RegKeyExists(HKEY_LOCAL_MACHINE, fr64_hkey_location + '{#MyAppID}_is1') then 
    begin
      RegQueryStringValue(HKEY_LOCAL_MACHINE, fr64_hkey_location + '{#MyAppID}_is1', 
        'DisplayVersion', Version);
      //if this is a newer version, update older version.
      if Version < ExpandConstant('{#MyAppVersion}') then begin  
          if MsgBox(ExpandConstant('The Setup detected an older version of {#MyAppName} on your machine.' +
          ' Would you like to update to {#MyAppVersion}?'), mbConfirmation, MB_YESNO) = IDYES then
          begin
            Result := True;   
          end
          else
          begin
            Result :=False;
          end;
      end
      // this is the same version. exit.
      else if Version = ExpandConstant('{#MyAppVersion}') then begin
          if MsgBox(ExpandConstant('The version of {#MyAppName} you are installing ' +
           ' ({#MyAppVersion}) is already on your machine. Would you like ' +
           'to re-install anyway?'), mbConfirmation, MB_YESNO) = IDYES then
          begin
            Result := True;   
          end
          else
          begin
            Result :=False;
          end;
      end
      // this is an older version. exit or continue.               
      else begin      
         if MsgBox(ExpandConstant('The version of {#MyAppName} you are installing' +
          ' ({#MyAppVersion}) is older than the version on your machine. Continue ' +
          'with installation?'), mbConfirmation, MB_YESNO) = IDYES then
         begin
            Result := True;
         end
         else
         begin
            Result :=False;
         end;
      end;  
    end
    //this is a fresh install, proceed normally.
    else begin          
      Result := True;
    end;
end;
