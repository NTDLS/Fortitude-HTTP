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

DeleteTree IPCH /Y /DelRoot

attrib -s -h -r

Del Release\x64\*.* /Q
Del Release\Win32\*.* /Q
Del Release\*.* /Q
Rd Release\x64
Rd Release\Win32
Rd Release

Del Debug\x64\*.* /Q
Del Debug\Win32\*.* /Q
Del Debug\*.* /Q
Rd Debug\x64
Rd Debug\Win32
Rd Debug

Del Setup\Output\*.* /Q
RD Setup\Output

Del Resources\*.aps

Cls