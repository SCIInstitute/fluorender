Write-Host "Configuring OpenCL into a Library..."
mkdir ../OpenCL
cd ../OpenCL

Write-Host "Creating Include and Lib directories..."
mkdir lib ; mkdir include ; cd lib ; mkdir x64 ; cd .. ; cd include ; mkdir CL ; cd ../../
Write-Host "Done!"

Write-Host "Moving Libraries..."
mv fluorender/OpenCL/lib/Release/OpenCL.lib OpenCL/lib/x64
mv fluorender/OpenCL/lib/Release/OpenCL.dll OpenCL/lib/x64
Write-Host "Done!"

Write-Host "Moving Include Files..."
mv fluorender/OpenCL/OpenCL-ICD-Loader/inc/CL/*.h OpenCL/include/CL
Write-Host "Done!"

cd OpenCL

Write-Host "Setting Enviornment Variable..."
$Root=(Get-Item -Path ".\").FullName

echo "::set-env name=OCL_ROOT::$Root"

Write-Host "Done! Finished Configuring!"
