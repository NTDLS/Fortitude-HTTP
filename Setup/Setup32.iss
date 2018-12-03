[Setup]
;-- Main Setup Information
 AppName                         = Fortitude HTTP
 AppVerName                      = Fortitude HTTP 1.0.4.2
 AppCopyright                    = Copyright © 1995-2018 NetworkDLS.
 DefaultDirName                  = {pf}\NetworkDLS\Fortitude HTTP
 DefaultGroupName                = NetworkDLS\Fortitude HTTP
 UninstallDisplayIcon            = {app}\FortitudeMgt.Exe
 WizardImageFile                 = ..\SetupResources\Setup\LgSetup.bmp
 WizardSmallImageFile            = ..\SetupResources\Setup\SmSetup.bmp
 PrivilegesRequired              = PowerUser
 Uninstallable                   = Yes
 LicenseFile                     = ..\SetupResources\Setup\License\EULA.txt
 Compression                     = ZIP/9
 OutputBaseFilename              = HTTPServer32
 MinVersion                      = 0.0,5.0
 ArchitecturesAllowed            = x86

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.4.2

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
 Source: "..\SetupResources\Setup\License\EULA.txt";          DestDir: "{app}";             Components: Base;
 Source: "..\AutoUpdate\Win32\Release\AutoUpdate.Exe";        DestDir: "{app}";         Components: Base\AutoUpdate;      Flags: RestartReplace;
 Source: "AutoUpdate.xml";                                    DestDir: "{app}";         Components: Base\AutoUpdate;      Flags: IgnoreVersion;
 Source: "..\Source\Console\Release\Win32\FortitudeMgt.exe";  DestDir: "{app}";         Components: Base\Management;      Flags: IgnoreVersion;
 Source: "..\Source\Service\Release\Win32\FortitudeSvc.exe";  DestDir: "{app}";         Components: Server;               Flags: IgnoreVersion;
 Source: "..\Source\Console\Release\Win32\FortitudeMgt.pdb";  DestDir: "{app}";         Components: Base\Debugging;       Flags: IgnoreVersion;
 Source: "..\Source\Service\Release\Win32\FortitudeSvc.pdb";  DestDir: "{app}";         Components: Base\Debugging;       Flags: IgnoreVersion;
 Source: "Resources\Start.ico";                               DestDir: "{app}";             Components: Server;
 Source: "Resources\Stop.ico";                                DestDir: "{app}";             Components: Server;
 Source: "Messages\*.*";                                      DestDir: "{app}\Messages";    Components: Server\CustomErrors;  Flags: RecurseSubDirs OnlyIfDoesntExist;
 Source: "Filters\*.*";                                       DestDir: "{app}\Filters";     Components: Server\PreConfigured; Flags: RecurseSubDirs OnlyIfDoesntExist;
 Source: "IndexImages\*.*";                                   DestDir: "{app}\IndexImages"; Components: Server\IndexImages;   Flags: RecurseSubDirs OnlyIfDoesntExist;
 Source: "Data\Server\*.*";                                   DestDir: "{app}\Data";        Components: Server;               Flags: OnlyIfDoesntExist;
 Source: "Data\Server\preConfigured\32\*.*";                  DestDir: "{app}\Data";        Components: Server\PreConfigured; Flags: OnlyIfDoesntExist;
 Source: "Data\Server\preConfigured\*.*";                     DestDir: "{app}\Data";        Components: Server\PreConfigured; Flags: OnlyIfDoesntExist;
 Source: "Data\Server\nonConfigured\*.*";                     DestDir: "{app}\Data";        Components: Server\nonConfigured; Flags: OnlyIfDoesntExist;
 Source: "Data\Interface\*.*";                                DestDir: "{app}\Data";        Components: Base\Management;      Flags: OnlyIfDoesntExist;
 Source: "WebSites\*.*";                                      DestDir: "{app}\WebSites";    Components: Server\PreConfigured; Flags: RecurseSubDirs OnlyIfDoesntExist;
 
[Dirs]
 Name: "{app}\Logs";  Components: Server;
 Name: "{app}\Cache"; Components: Server;

[Icons]
 Name: "{group}\Manage Fortitude HTTP";        Filename: "{app}\FortitudeMgt.Exe"; WorkingDir: "{app}";     Components: Base\Management;
 Name: "{group}\Fortitude HTTP Documentation"; Filename: "http://FortitudeHTTP.com/";

[UninstallDelete]
 Type: FilesAndOrDirs; Name: "{app}\Logs";    Components: Server;
 Type: FilesAndOrDirs; Name: "{app}\Cache";   Components: Server;
 Type: FilesAndOrDirs; Name: "{app}\ErrorLog";

[Run]
 Filename: "{app}\FortitudeSvc.exe"; Parameters: "/Install"; StatusMsg: "Installing service...";                               Components: Server;
 Filename: "{app}\FortitudeSvc.exe"; Parameters: "/Start";   StatusMsg: "Starting Service...";                                 Components: Server;
 Filename: "{app}\FortitudeMgt.exe"; Description: "Open Management Console"; Flags: PostInstall NoWait shellexec skipifsilent; Components: Base\Management;

[UninstallRun]
 Filename: "{app}\FortitudeSvc.exe"; Parameters: "/Stop";   RunOnceId: "StopService";   MinVersion: 0, 4.0; Components: Server;
 Filename: "{app}\FortitudeSvc.exe"; Parameters: "/Delete"; RunOnceId: "DeleteService"; MinVersion: 0, 4.0; Components: Server;

