; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{BBD0A264-C224-4AC4-853A-2D269948A8AE}
;version number appears three times AppName shows application name in installer
AppName=Words 4.4
AppVersion=4.4
AppPublisher=Alexey Slovesnov
AppPublisherURL=http://slovesnov.users.sf.net/?words
AppSupportURL=http://slovesnov.users.sf.net/?words
AppUpdatesURL=http://slovesnov.users.sf.net/?words
DefaultDirName={commonpf}\words
DefaultGroupName=words
OutputBaseFilename=words44Setup
SetupIconFile=words\addon\words.ico
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Messages]
english.BeveledLabel=English
russian.BeveledLabel=Russian

[Files]
Source: "Release\words.exe"; DestDir: "{app}\bin"; Flags: ignoreversion

Source: "C:\soft\msys64\mingw64\share\icons\Adwaita\16x16\ui\pan-up-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
Source: "C:\soft\msys64\mingw64\share\icons\Adwaita\16x16\ui\pan-down-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
Source: "C:\soft\msys64\mingw64\share\icons\Adwaita\16x16\ui\pan-end-symbolic.symbolic.png"; DestDir: "{app}\share\icons\Adwaita\16x16\actions"; Flags: onlyifdoesntexist
;new gtk version so ignoreversion for dlls
Source: "C:\soft\msys64\mingw64\bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
;gspawn-win32-helper.exe needs for gtk_show_uri_on_window() function
Source: "C:\soft\msys64\mingw64\bin\gspawn-win64-helper.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\soft\msys64\mingw64\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0"; Flags: ignoreversion
Source: "C:\soft\msys64\mingw64\lib\gdk-pixbuf-2.0\2.10.0\loaders\*.dll"; DestDir: "{app}\lib\gdk-pixbuf-2.0\2.10.0\loaders"; Flags: ignoreversion
Source: "words.css"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "words\*"; DestDir: "{app}\bin\words"; Flags: onlyifdoesntexist
Source: "words\images\*"; DestDir: "{app}\bin\words\images"; Flags: onlyifdoesntexist
Source: "words\en\*"; DestDir: "{app}\bin\words\en"; Flags: ignoreversion
Source: "words\ru\*"; DestDir: "{app}\bin\words\ru"; Flags: ignoreversion

[Icons]
Name: "{group}\words"; Filename: "{app}\bin\words.exe"
Name: "{commondesktop}\words"; Filename: "{app}\bin\words.exe"
Name: "{group}\{cm:UninstallProgram,words}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\bin\words.exe"; Description: "{cm:LaunchProgram,words}"; Flags: nowait postinstall skipifsilent
