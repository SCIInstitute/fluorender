1) To generate the JavaToC.h file use  the fillowing commands:
> javac JavaToC.java -- class file generator
> javah JavaToC -- header file generator

2) Place this header file in the C++ project in this directory.

3) Write the implementation of the methods given in generated header file inside the file JavaToC.cpp

4) Create a DLL out of the MS Visual Studio project and place that dll in the folder: "<path to fluorender>/fluorender/Java_Code/ImageJ_Reader/target/classes"

5) Once Java is able to find the JavaToC.dll/JavaToC.so file you can then call the allocateDirect methods from JAVA.
   This allocated memory outside of JVM and is accessible by C++.
   
6) You will have to test out if it is viable to do so using the metadata. I have written the function for metadata in JAVA and if everything works you should
   be able to remove the JVM memory and use shared memory.
   

------ What is working --------
1) Allocation of memory outside the JVM and accessing it from JAVA is working. <<See JAvaToC.cpp>>
2) See the method getMappedDataInt inside ImagejReader.java

------ What is left -----------
To check if this memory allocated by JAVA outside the JVM is actually accesible by the Fluorender C++ main.
