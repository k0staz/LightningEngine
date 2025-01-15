@echo off

pushd ..
3rdParty\Premake\Bin\premake5.exe --file=Build.lua vs2022
popd
pause