# Quick and dirty script to run clang-format over all our C++ source files.
# Just set the path to clang-format for your machine. I know less than nothing
# about PowerShell and am certain that this could be improved.

$formatter = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"
$includefiles = @(".cpp", ".h")

# Assume that the root directory is one directory up from the current script
# and that source code is all there
$srcdir = (Get-Item $PSScriptRoot).Parent
foreach ($file in (Get-ChildItem -Path $srcdir.FullName -File).Where({$_.Extension -in $includefiles})) {
    & $formatter -i $file.FullName
}
