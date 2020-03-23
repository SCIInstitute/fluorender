Write-Host "Building OpenCL..."

cd OpenCL-ICD-Loader ; mkdir lib ; cd lib
cmake ..
cmake --build

Write-Host "Done."
