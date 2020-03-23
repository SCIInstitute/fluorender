Write-Host "Building OpenCL..."

mkdir OpenCL

Move-Item OpenCL-Headers OpenCL
Move-Item OpenCL-ICD-Loader OpenCL
cd OpenCL

mkdir OpenCL-ICD-Loader/inc/CL

mv OpenCL-Headers/CL/*.h OpenCL-ICD-Loader/inc/CL

cd OpenCL-ICD-Loader ; mkdir lib ; cd lib
cmake ..
cmake --build .

Write-Host "Done."
