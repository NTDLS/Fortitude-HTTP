[Setup]
;-- Main Setup Information
 AppName                         = Fortitude HTTP
 AppVerName                      = Fortitude HTTP 1.0.4.1
 AppCopyright                    = Copyright © 1995-2018 NetworkDLS.
 DefaultDirName                  = {pf}\NetworkDLS\Fortitude HTTP
 DefaultGroupName                = NetworkDLS\Fortitude HTTP
 UninstallDisplayIcon            = {app}\Bin\FortitudeMgt.Exe
 WizardImageFile                 = ..\..\@Resources\Setup\LgSetup.bmp
 WizardSmallImageFile            = ..\..\@Resources\Setup\SmSetup.bmp
 PrivilegesRequired              = PowerUser
 Uninstallable                   = Yes
 LicenseFile                     = ..\..\@Resources\Setup\License\EULA.txt
 Compression                     = ZIP/9
 OutputBaseFilename              = HTTPServer64
 MinVersion                      = 0.0,5.0
 ArchitecturesInstallIn64BitMode = x64
 ArchitecturesAllowed            = x64

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.4.1

[Components]
 Name: Base;                 Description: "Base Install";              Types: full compact custom; Flags: Fixed;
 Name: Base\AutoUpdate;      Description: "Auto-Update Utility";       Types: full compact custom;
 Name: Base\Management;      Description: "Management Console";        Types: full compact custom;
 Name: Base\Debugging;       Description: "Native Debugging";
 Name: Server;               Description: "Web Server";                Types: full compact custom;
 Name: Server\PreConfigured; Description: "Configured with Examples";  Types: full;                Flags: exclusive;
 Name: Server\nonConfigured; Description: "Minimal pre-Configuration"; Types: compact;             Flags: exclusive;
 Name: Server\IndexImages;   Description: "Index Images";              Types: full compact custom;
 Name: Server\CustomErrors;  Description: "Custom Errors";             Types: full compact custom;

[Files]
 Source: "..\..\@Resources\Setup\License\EULA.txt";           DestDir: "{app}";             Components: Base;
 Source: "..\..\@AutoUpdate\x64\Release\AutoUpdate.Exe";      DestDir: "{app}\Bin";         Components: Base\AutoUpdate;      Flags: RestartReplace;
 Source: "AutoUpdate.xml";                                    DestDir: "{app}\Bin";         Components: Base\AutoUpdate;      Flags: IgnoreVersion;
 Source: "..\Source\Console\Release\x64\FortitudeMgt.exe";    DestDir: "{app}\Bin";         Components: Base\Management;      Flags: IgnoreVersion;
 Source: "..\Source\Service\Release\x64\FortitudeSvc.exe";    DestDir: "{app}\Bin";         Components: Server;               Flags: IgnoreVersion;
 Source: "..\Source\Console\Release\x64\FortitudeMgt.pdb";    DestDir: "{app}\Bin";         Components: Base\Debugging;       Flags: IgnoreVersion;
 Source: "..\Source\Service\Release\x64\FortitudeSvc.pdb";    DestDir: "{app}\Bin";         Components: Base\Debugging;       Flags: IgnoreVersion;
 Source: "Resources\Start.ico";                               DestDir: "{app}";             Components: Server;
 Source: "Resources\Stop.ico";                                DestDir: "{app}";             Components: Server;
 Source: "Messages\*.*";                                      DestDir: "{app}\Messages";    Components: Server\CustomErrors;  Flags: RecurseSubDirs OnlyIfDoesntExist;
 Source: "Filters\*.*";                                       DestDir: "{app}\Filters";     Components: Server\PreConfigured; Flags: RecurseSubDirs OnlyIfDoesntExist;
 Source: "IndexImages\*.*";                                   DestDir: "{app}\IndexImages"; Components: Server\IndexImages;   Flags: RecurseSubDirs OnlyIfDoesntExist;
 Source: "Data\*.*";                                          DestDir: "{app}\Data";        Components: Server;               Flags: OnlyIfDoesntExist;
 Source: "Data\preConfigured\64\*.*";                         DestDir: "{app}\Data";        Components: Server\PreConfigured; Flags: OnlyIfDoesntExist;
 Source: "Data\preConfigured\*.*";                            DestDir: "{app}\Data";        Components: Server\PreConfigured; Flags: OnlyIfDoesntExist;
 Source: "Data\nonConfigured\*.*";                            DestDir: "{app}\Data";        Components: Server\nonConfigured; Flags: OnlyIfDoesntExist;
 Source: "WebSites\*.*";                                      DestDir: "{app}\WebSites";    Components: Server\PreConfigured; Flags: RecurseSubDirs OnlyIfDoesntExist;

[Dirs]
 Name: "{app}\Logs";  Components: Server;
 Name: "{app}\Cache"; Components: Server;

[Icons]
 Name: "{group}\Manage Fortitude HTTP";        Filename: "{app}\Bin\FortitudeMgt.Exe"; WorkingDir: "{app}\Bin";     Components: Base\Management;
 Name: "{group}\Fortitude HTTP Documentation"; Filename: "http://FortitudeHTTP.com/";
 
[Registry]
 Root: HKLM; Subkey: "Software\NetworkDLS\Fortitude HTTP"; Flags: uninsdeletekey noerror;
 Root: HKLM; Subkey: "Software\NetworkDLS\Fortitude HTTP"; ValueName: "DataPath";        ValueType: String; ValueData: "{app}\Data";    Flags: CreateValueIfDoesntExist; Components: Server;
 Root: HKLM; Subkey: "Software\NetworkDLS\Fortitude HTTP"; ValueName: "Path";            ValueType: String; ValueData: "{app}";         Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Fortitude HTTP"; ValueName: "ManagementPort";  ValueType: DWORD;  ValueData: "35634";         Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Fortitude HTTP"; ValueName: "LastConnectHost"; ValueType: String; ValueData: "localhost";     Flags: CreateValueIfDoesntExist; Components: Base\Management;
 Root: HKLM; Subkey: "Software\NetworkDLS\Fortitude HTTP"; ValueName: "LastConnectUser"; ValueType: String; ValueData: "Administrator"; Flags: CreateValueIfDoesntExist; Components: Base\Management;

[UninstallDelete]
 Type: FilesAndOrDirs; Name: "{app}\Logs";    Components: Server;
 Type: FilesAndOrDirs; Name: "{app}\Cache";   Components: Server;
 Type: FilesAndOrDirs; Name: "{app}\ErrorLog";

[Run]
 Filename: "{app}\Bin\FortitudeSvc.exe"; Parameters: "/Install"; StatusMsg: "Installing service...";                               Components: Server;
 Filename: "{app}\Bin\FortitudeSvc.exe"; Parameters: "/Start";   StatusMsg: "Starting Service...";                                 Components: Server;
 Filename: "{app}\Bin\FortitudeMgt.exe"; Description: "Open Management Console"; Flags: PostInstall NoWait shellexec skipifsilent; Components: Base\Management;

[UninstallRun]
 Filename: "{app}\Bin\FortitudeSvc.exe"; Parameters: "/Stop";   RunOnceId: "StopService";   MinVersion: 0, 4.0; Components: Server;
 Filename: "{app}\Bin\FortitudeSvc.exe"; Parameters: "/Delete"; RunOnceId: "DeleteService"; MinVersion: 0, 4.0; Components: Server;

