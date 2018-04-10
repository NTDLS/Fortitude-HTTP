@Echo Off

attrib -s -h -r

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

attrib -s -h -r

rd "Optimized Debug" /S /Q
rd "Release" /S /Q
rd "Debug" /S /Q

RD "Setup\Output" /S /Q

Del Resources\*.aps

Cls
