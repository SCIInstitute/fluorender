Write-Host "Building OpenCL..."

mkdir OpenCL

Move-Item OpenCL-Headers OpenCL
Move-Item OpenCL-ICD-Loader OpenCL
cd OpenCL

mkdir OpenCL-ICD-Loader/inc/CL

mv OpenCL-Headers/CL/*.h OpenCL-ICD-Loader/inc/CL

ls OpenCL-ICD-Loader/inc/CL

mkdir lib ; cd lib
cmake ../OpenCL-ICD-Loader
cmake --build . --config Release

Write-Host "Done."

ls ; cd .. ; ls ; cd .. ; ls
