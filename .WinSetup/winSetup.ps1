Write-Host "Setup for Windows"

Write-Host "Installing Qt Creator..."
choco install qtcreator

mkdir OpenCL
cd OpenCL
Write-Host "Downloading the OpenCL Headers..."
git
Write-Host "Done, cloning into the Main OpenCL library."
git clone --branch v2020.03.13 https://github.com/KhronosGroup/OpenCL-ICD-Loader
Write-Host "Done, creating CL dir in Loader and moving files over..."
mkdir OpenCL-ICD-Loader/inc/CL
mv OpenCL-Headers/CL/*.h OpenCL-ICD-Loader/inc/CL
ls OpenCL-ICD-Loader/inc/CL
Write-Host "Done. Creating lib..."

cd OpenCL-ICD-Loader ; mkdir lib ; cd lib
cmake ..
cmake --build

Write-Host "Done."


