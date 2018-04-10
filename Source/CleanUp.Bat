@Echo Off

attrib *.* -s -h -r

Del *.Ncb
Del *.Opt
Del *.Plg
Del *.Aps
Del *.Scc
Del *.suo
Del *.xml
Del *.old
Del *.user
Del *.sdf

rd IPCH /S /Q

attrib *.* -s -h -r /D /S

rd "Optimized Debug" /S /Q
rd "Release" /S /Q
rd "Debug" /S /Q
rd ".vs" /S /Q

RD "Setup\Output" /S /Q

Del Resources\*.aps

REM ---

attrib Service\*.* -s -h -r /D /S

Del Service\*.Ncb
Del Service\*.Opt
Del Service\*.Plg
Del Service\*.Aps
Del Service\*.Scc
Del Service\*.suo
Del Service\*.xml
Del Service\*.old
Del Service\*.user
Del Service\*.sdf

rd Service\IPCH /S /Q

attrib Service\*.* -s -h -r /D /S

rd "Service\Optimized Debug" /S /Q
rd "Service\Release" /S /Q
rd "Service\Debug" /S /Q
rd "Service\.vs" /S /Q

RD "Service\Setup\Output" /S /Q

Del Service\Resources\*.aps

REM ----------------------------

attrib Console\*.* -s -h -r /D /S

Del Console\*.Ncb
Del Console\*.Opt
Del Console\*.Plg
Del Console\*.Aps
Del Console\*.Scc
Del Console\*.suo
Del Console\*.xml
Del Console\*.old
Del Console\*.user
Del Console\*.sdf

rd Console\IPCH /S /Q

attrib Console\*.* -s -h -r /D /S

rd "Console\Optimized Debug" /S /Q
rd "Console\Release" /S /Q
rd "Console\Debug" /S /Q
rd "Console\.vs" /S /Q

RD "Console\Setup\Output" /S /Q

Del Console\Resources\*.aps
