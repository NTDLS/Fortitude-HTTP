::32Bit
RD "Output\32-Bit-Portable" /S /Q
MD "Output\32-Bit-Portable"
MD "Output\32-Bit-Portable\Cache"
MD "Output\32-Bit-Portable\Data"
MD "Output\32-Bit-Portable\ErrorLog"
MD "Output\32-Bit-Portable\Logs"
MD "Output\32-Bit-Portable\WebSites"
copy "..\Source\Console\Release Portable\Win32\*.exe" "Output\32-Bit-Portable\" /Y
copy "..\Source\Service\Release Portable\Win32\*.exe" "Output\32-Bit-Portable\" /Y
copy "Data\Server\Portable\*.xml" "Output\32-Bit-Portable\Data\" /Y
copy "Data\Server\Portable\32\*.xml" "Output\32-Bit-Portable\Data\" /Y
copy "Data\Server\*.xml" "Output\32-Bit-Portable\Data\" /Y
copy "Data\Interface\*.xml" "Output\32-Bit-Portable\Data\" /Y
XCopy "IndexImages" "Output\32-Bit-Portable\IndexImages\" /E /Y
XCopy "Filters" "Output\32-Bit-Portable\Filters\" /E /Y

::64Bit
RD "Output\64-Bit-Portable" /S /Q
MD "Output\64-Bit-Portable"
MD "Output\64-Bit-Portable\Cache"
MD "Output\64-Bit-Portable\Data"
MD "Output\64-Bit-Portable\ErrorLog"
MD "Output\64-Bit-Portable\Logs"
MD "Output\64-Bit-Portable\WebSites"
copy "..\Source\Console\Release Portable\x64\*.exe" "Output\64-Bit-Portable\" /Y
copy "..\Source\Service\Release Portable\x64\*.exe" "Output\64-Bit-Portable\" /Y
copy "Data\Server\Portable\*.xml" "Output\64-Bit-Portable\Data\" /Y
copy "Data\Server\Portable\64\*.xml" "Output\64-Bit-Portable\Data\" /Y
copy "Data\Server\*.xml" "Output\64-Bit-Portable\Data\" /Y
copy "Data\Interface\*.xml" "Output\64-Bit-Portable\Data\" /Y
XCopy "IndexImages" "Output\64-Bit-Portable\IndexImages\" /E /Y
XCopy "Filters" "Output\64-Bit-Portable\Filters\" /E /Y
Pause

