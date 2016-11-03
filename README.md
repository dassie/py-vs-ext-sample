# py-vs-ext-sample
A project demonstrating how to write, build, and debug python extension modules inside Visual Studio using Boost.Python.

I created this project because the instructions for building boost.python are a bit scattered and those for using the libs and headers inside Visual Studio are non-existent. I figured, perhaps others would benefit from a brief guide on how to set it up.

# How To

These are roughly the steps I had to take in order to create this project. My environment looked like this:
* Boost 1.62.0
* Python 3.5.2 64-bit
* VS2015 Community Edition

## Building the Boost.Python Libraries
After extracting boost-1.62.0 and setting up my python environment and whatnot, I compiled the 64-bit static and shared boost.python libraries using the following commands (2 bjam commands for debug/release and 2 for shared/static). 

    cd <your boost root dir>
    bootstrap
    bjam --toolset=msvc-14.0 address-model=64 --with-python python-debugging=off threading=multi variant=debug toolset=msvc-14.0 link=static --stagedir=stage\x64-static-python35 stage
    bjam --toolset=msvc-14.0 address-model=64 --with-python python-debugging=off threading=multi variant=release toolset=msvc-14.0 link=static --stagedir=stage\x64-static-python35 stage
    bjam --toolset=msvc-14.0 address-model=64 --with-python python-debugging=off threading=multi variant=debug toolset=msvc-14.0 link=shared --stagedir=stage\x64-shared-python35 stage
    bjam --toolset=msvc-14.0 address-model=64 --with-python python-debugging=off threading=multi variant=release toolset=msvc-14.0 link=shared --stagedir=stage\x64-shared-python35 stage
    
### Notes
* You can omit the ones you don't need.
* I created x64 libraries because my python installation was 64-bit. You shouldn't have any trouble building these for 32-bit, just omit the address-model=64 argument from the commands (it will default to 32 bit).
* Before running these commands I created a `user-config.jam` file in the root boost directory with the following content: `using python : 3.5 ;`
* I chose the output directories based on shared vs. static link types so that Release and Debug libraries would go to the same folder but shared and static would be separated. It seems when building with the boost.python headers later on they have a way of detecting which one they will link with (the ones with 'gd' in the name indicate debug). Actually all the files are able to reside in the same folder and the build will work but I wanted to separate them.
    
(By the way, does anyone know what is the difference between python and python3 in the names of the output lib and dll files?)

## Creating a Visual C++ Project

Create an empty Visual C++ project. These are the important changes you need to make.
 * General -> Output Configuration = Dynamic Library 
 * General -> Target Extension = .pyd 
 * VC++ Directories -> Includes Directories - Added the path to the boost root directory and C:\Python35\include 
 * VC++ Directories -> Library Directories - Added the path with either the shared or static libraries created previously and C:\Python35\libs 
 * C++ -> Preprocessor -> Preprocessor Definitions - Add `BOOST_PYTHON_STATIC_LIB` (Skip this if you intend to use shared libraries)


Put a source file in your project with the following content (or whatever you choose):
	#include <boost/python.hpp>
	#include <string>

	std::string sayhello()
	{
		return std::string("Hello");
	}

	BOOST_PYTHON_MODULE(PySampleExt)
	{
		using namespace boost::python;
		def("sayhello", sayhello);
	}

**Important**: Make sure the argument to BOOST_PYTHON_MODULE (in this case 'PySampleExt') matches the name of the output .pyd file.

Hit build and hopefully it succeeds. If you used shared libraries, you must also copy the appropriate .dll to the same directory as your .pyd file.

You can now launch a python interpretor and the following script/lines should work:

    import PySampleExt
    PySampleExt.sayhello()

More importantly, you can debug your extension inside VS. To do so make the following changes:

* Debugging -> Command = C:\Python35\python.exe
* Debugging -> Working Directory = (your output directory)

Place a breakpoint somewhere (like the return statement inside `sayhello()`) and hit debug. The breakpoint will be greyed out at first because launching python.exe doesn't load your .pyd file. But it will get loaded when you execute the import statement. When `PySampleExt.greet()` is called your breakpoint should get triggered.

## What's special about this project?

I automated all the above steps so that it is better integrated with Visual Studio. The main changes you will notice are:

* A python project has been added with a small .py script file.
* The output directory for the C++ project uses the following scheme: `$(SolutionDir)bin\$(Configuration)-$(Platform)\PySampleExt\`. This a personal preference for me as I don't like the default. Basically it creates a top-level 'bin' folder that is a sibling of the .sln file where the project outputs go to. The intermediate directory follows a similar scheme.
* The default filters for the .vcxproj were removed and instead I created 2 custom ones named 'Include' and 'Source'. I also created actually folders next to the .vcxproj with the same names. This is so that the filters correspond to actual directories and .cpp and .h files don't end up being cluttered into a single folder.
* A custom msbuild file called PySampleExt.Targets has been added. This file is imported by the .vcxproj file. The python root, boost headers, and boost libs directories are defined as properties in here. 
   * There is a 'SanityCheck' target that runs before 'Build' and makes sure above properties have values.
   * There is a 'CopyPyd' target that runs after 'Build' and copies the output .pyd file to the python project's directory. This way the module is always up-to-date.
* A preprocessor directive has been added: `MODULE_NAME=$(TargetName)`. `MODULE_NAME` is then used instead of 'PySampleExt' inside the .cpp file. This way the module name always matches the output .pyd file's name.
* The `BOOST_PYTHON_STATIC_LIB` directive has also been added because this project uses the static libraries. If you intend to use the shared libraries, you can remove this directive. You can also add an extra copy step to the CopyPyd target to copy your dll next to the python script as well.
* The `PySampleExt` project is the start up project and when it is launched with F5, it launches your python executable and runs the python script in the afformentioned python project.

*Note: the `PythonRoot`, `BoostHeadersDir`, and `BoostLibsDir` properties in the PySampleExt.Targets file are purposely left empty so you can fill them in according to your environment.*