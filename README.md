# Sourcetrail

[Sourcetrail](https://www.sourcetrail.com/) is a free and open-source cross-platform source explorer that helps you get productive on unfamiliar source code.

Windows: [![Build status](https://ci.appveyor.com/api/projects/status/0c258a2opn3loyc2/branch/master?svg=true)](https://ci.appveyor.com/project/mlangkabel/sourcetrail/branch/master)

Linux: [![Build Status](https://travis-ci.org/CoatiSoftware/Sourcetrail.svg?branch=master)](https://travis-ci.org/CoatiSoftware/Sourcetrail)


__Links__
* [Website](https://www.sourcetrail.com/)
* [Download](https://www.sourcetrail.com/downloads)
* [Quick Start Guide](https://www.sourcetrail.com/documentation/#QUICKSTARTGUIDE)
* [Documentation](https://www.sourcetrail.com/documentation)
* [Changelog](CHANGELOG.md)
* [Slack Channel](https://join.slack.com/t/sourcetrail/shared_invite/enQtNDc3MjcyOTk5MTc0LTNhOWZhZjAxZmU1NTM0YjhhMzRhZWE1NTQ2NjQyYjc1ODE2MzViOTliMWUwODBjN2NlMjkwMDc0MjAxMzJkY2E)
* [Mailing List](https://coati.us12.list-manage.com/subscribe/post?u=3dabab4e475b5ed577d1dcd0f&id=cf7301fc53)
* [Patreon](https://www.patreon.com/sourcetrail)

!["Sourcetrail User Interface"](docs/readme/user_interface.png "Sourcetrail User Interface")

Sourcetrail is:
* free
* working offline
* operating on Windows, macOS and Linux
* supporting C, C++, Java and Python
* offering an SDK ([SourcetrailDB](https://github.com/CoatiSoftware/SourcetrailDB)) to write custom language extensions

## Support Sourcetrail via Patreon

The ongoing development and regular software releases are made possible entirely by the support of [these awesome patrons](SPONSORS.md)! If you'd like to join them, please consider [becoming a patron](https://www.patreon.com/sourcetrail) of Sourcetrail.

## Using Sourcetrail

To setup Sourcetrail on your machine, you can either download the respective build for your operating system from our list of [Releases](https://www.sourcetrail.com/downloads) and install it on your machine, or use one of the following package managers:

* Use the [Chocolatey package](https://chocolatey.org/packages/sourcetrail) with `choco install sourcetrail`

After your installation is complete, follow our [Quick Start Guide](https://www.sourcetrail.com/documentation/#QUICKSTARTGUIDE) to get to know Sourcetrail.

## How to Report Issues

You can post all your feature requests and bug reports on our [issue tracker](https://github.com/CoatiSoftware/Sourcetrail/issues).

### Reporting

Use the following template:

* platform version:
* Sourcetrail version:
* description of the problem:
* steps to reproduce the problem:


### Supporting

If you want to support a certain feature request or you have the same bug that another user already reported, please let us know:
* post a comment with "+1" to the issue
* or send an email to support@sourcetrail.com with the issue ID

## How to Contribute

* Please read and follow the steps in [CONTRIBUTING.md](CONTRIBUTING.md) file. 
* You may want to look out for issues labeled [good first issue](https://github.com/CoatiSoftware/Sourcetrail/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22) to find some initial tasks to tackle.
* If you are looking for more information about Sourcetrail software development, please refer to our [wiki](https://github.com/CoatiSoftware/Sourcetrail/wiki).

# How to Build

Building Sourcetrail requires several dependencies to be in place on your machine. However, our CMake based setup allows to disable indexing support for specific languages which reduces the number of dependencies to a minimum.

## Building the bare minimum

### Required Tools

#### Git

This is required for generating the Sourcetrail version number. Get it from: https://git-scm.com/download.

Make sure `git` is available in added to your `PATH` environment variable.

#### CMake v3.12

This is used for generating a build configuration. Get it from: https://cmake.org/download/

### Additional tools for Windows

#### Visual Studio

### Additional tools for Unix

#### ccache

This is optionally used to speed up rebuilds if found in `PATH`.

### Required dependencies

#### Boost 1.67
For the __msvc__ compiler pre-built binaries can be downloaded from [sourceforge.net/projects/boost/files/boost-binaries](https://sourceforge.net/projects/boost/files/boost-binaries/)

For building on Unix:
```
$ ./bootstrap.sh --with-libraries=filesystem,program_options,system,date_time
$ ./b2 --link=static --variant=release --threading=multi --runtime-link=static --cxxflags=-fPIC
```

#### Qt 5.12

__Note__: still causes style issues on Windows, resort to Qt 5.10

For the __msvc__ compiler pre-built binaries can be downloaded from [download.qt.io/official_releases/qt](http://download.qt.io/official_releases/qt/)

### Building

#### For Windows / Visual Studio
```
$ cd Sourcetrail
$ mkdir -p build/win64
$ cd build/win64
$ cmake -G "Visual Studio 15 2017 Win64" -DBOOST_ROOT=<path/to/boost_1_68_0> -DQt5_DIR=<path/to/Qt/version/platform/compiler/lib/cmake/Qt5> ../..
```
If you are using the gui, we recommend that you activate advanced mode. Also you may be required to add some of the defines via the "Add Entry" button.

After generating the build configuration, just open the Sourcetrail.sln file that was generated by CMake and build the Sourcetrail project.

##### If using clang-cl.exe and Ninja (such as via CLion)

Here's what I ended up using for Clang/Ninja builds via CLion:

* [Build Tools for Visual Studio 2019](https://aka.ms/buildtools) (as an alternative to installing Visual Studio)
* [Windows 10 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/) for released versions of Windows. If running a Windows Insider preview, you'll need the [Preview SDK](https://insider.windows.com/en-us/for-developers/) instead.
* Boost 1.72.0 compiled [following Windows instructions](https://www.boost.org/doc/libs/1_72_0/more/getting_started/windows.html) from within [a Build Tools command prompt](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019#developer_command_file_locations) (or you can download a [pre-compiled set of binaries](https://bintray.com/boostorg/release/boost-binaries)).
* Qt 5.15 Beta for official VS 2019 support (download the standard [Qt Community installer](https://www.qt.io/download-open-source) and then [install Qt 5.15 from the Preview package category](https://www.qt.io/blog/2019/04/11/updated-qt-installer-released).)
* Clang v10 compiler binaries for Windows (on the [LLVM Download page](https://releases.llvm.org/download.html) , install **Windows (64-bit)** under **Pre-Built Binaries**)
* [CMake 3.17.0](https://cmake.org/download/) for Windows x64 (from installer .msi)
* Clang source code (follow the instructions under **Enable C/C++ Language Support** though in my case I had success compiling via CLion with out-of-the-box settings)
* [Ninja for Windows](https://github.com/ninja-build/ninja/releases) extracted somewhere (in my case C:\Ninja\ninja.exe)
* [CLion EAP](https://www.jetbrains.com/clion/nextversion/) (optional, but makes running CMake builds with different compilers really easy...)

Configure CLion as follows:

From **Settings** in the **File** menu, click **Toolchains** under **Build, Execution, Deployment** on the left.

Then click the **+** button to create a new Toolchain, as follows:

| Toolchain Field | Value                                                            |
|:----------------|:-----------------------------------------------------------------|
| Name            | CN (or something equally short)                                  |
| Environment     | `C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools` |
| Architecture    | amd64                                                            |
| CMake           | `C:\Program Files\CMake\bin\cmake.exe`                           |
| Make            | `C:\Ninja\ninja.exe`                                             |
| C Compiler      | `C:\Program Files\LLVM\bin\clang-cl.exe`                         |
| C++ Compiler    | `C:\Program Files\LLVM\bin\clang-cl.exe`                         |

Then adjust project-specific **CMake** settings by clicking **CMake** below **Toolchains** on the left.

Make sure to select the **Toolchain** you created above in **CMake** settings. You can click the **+** button to add more CMake build folders, each can have a different set of Release/Debug compile options.

Note: Your **Build type** must match the same build type you may have used previously for your dependencies (Clang, Boost) or you may have build errors when linking to them.

The following **CMake options** were useful in compiling with the above:

```
-G
Ninja
-DBoost_DEBUG=ON
-DBOOST_INCLUDEDIR=C:\local\boost_1_72_0
-DBOOST_LIBRARYDIR=C:\local\boost_1_72_0\stage\x64\lib
-DBOOST_LIB_TOOLSET=vc142
-DBoost_COMPILER=-vc142
-DQt5_DIR=C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5
-DLLVM_DIR=C:\llvm-project\llvm\build\lib\cmake\llvm
-DClang_DIR=C:\llvm-project\llvm\build\lib\cmake\clang
-DBUILD_CXX_LANGUAGE_PACKAGE=ON
```

Adjust paths as necessary.
* **-G Ninja** tells CMake to use Ninja for the build. 
* **-DBoost_DEBUG** helps with troubleshoot finding Boost when you generate a CMake build.
* **-DBOOST_INCLUDEDIR** tells CMake where to find your Boost header files
* **-DBOOST_LIBRARYDIR** tells CMake where to find your precompiled Boost libs
* **-DBOOST_LIB_TOOLSET** is a custom CMake variable, it passes through to Boost's autolinker what compiler to expect in the lib file name, if different from the current compiler. (In this case, Boost was compiled with MSVC from VS 2019 but as we're compiling here with Clang, Boost tries to link against a file with clangw10 in its name.)
* **-DBoost_COMPILER** like above, is used by CMake to build filenames for required pre-compiled lib binaries for Boost.
* **-DQt5_DIR** should be set to Qt5 cmake files, in this case Qt 5.15 files for MSVC 2019 x64 environments.
* **-DLLVM_DIR** and **-DClang_DIR** and **-DBUILD_CXX_LANGUAGE_PACKAGE** For these, see the **Enable C/C++ Language Support** section later in this file.

If using **Java** you might want to fill in the **Environment** box in **CMake** settings with JAVA_HOME, M2_HOME and MAVEN_HOME.

#### For Unix

```
$ cd Sourcetrail
$ mkdir -p build/Release
$ cd build/Release
$ cmake -DCMAKE_BUILD_TYPE="Release" -DBOOST_ROOT=<path/to/boost_1_68_0> -DQt5_DIR=<path/to/Qt/version/platform/compiler/lib/cmake/Qt5> ../..
$ make Sourcetrail
```

### Running

Run Sourcetrail from within the build directory. During execution Sourcetrail needs resources from `bin/app/data` and `bin/app/user`. Cmake creates symlinks to these directories within the build directory.


## Enable C/C++ Language Support

### Required dependencies

#### LLVM/Clang 9.0.0

##### Windows
For the __msvc__ compiler, follow [these steps](https://clang.llvm.org/get_started.html) to build the project. Make sure to check out the correct tag and to run the cmake command exactly as described.

###### Building

Run CMake with these additional options:
```
-DLLVM_DIR=C:\llvm-project\llvm\build\lib\cmake\llvm
-DClang_DIR=C:\llvm-project\llvm\build\lib\cmake\clang
-DBUILD_CXX_LANGUAGE_PACKAGE=ON
```

##### Unix
For Unix, follow this [installation guide](http://clang.llvm.org/docs/LibASTMatchersTutorial.html)

Build with `-DLLVM_ENABLE_RTTI=ON`.

###### Building

Run CMake with these additional options:
```
-DClang_DIR=<path/to/llvm_build/lib/cmake/clang>
-DBUILD_CXX_LANGUAGE_PACKAGE=ON
```

## Enable Java Language Support

### Required dependencies

#### JDK 1.8

Install JDK and make sure the `JAVA_HOME` environment variable is set:
```
JAVA_HOME=.../Java/jdk1.x.x_xxx
```

Also make sure `<jdk_root>/bin` is available in your `PATH` environmen variable.

#### Maven
Install Maven and make sure both `M2_HOME` and `MAVEN_HOME` environment variables are set:
```
M2_HOME=.../apache-maven-x.x.x
MAVEN_HOME=.../apache-maven-x.x.x
```

Also make sure `.../apache-maven-x.x.x/bin` is available in your `PATH` environmen variable.

### Building

Run CMake with these additional options:
```
-DBUILD_JAVA_LANGUAGE_PACKAGE=ON
```

## Enable Python Language Support

### Required dependencies

#### WinRAR (for Windows only)

### Building

Run CMake with these additional options:
```
-DBUILD_PYTHON_LANGUAGE_PACKAGE=ON
```

## Packaging

### Windows

#### Required Tools

##### Visual Studio

##### Wix 3.11

##### Wix extension for Visual Studio

##### WinRAR

Make sure to append these directories to your `PATH` environment variable:
* VisualStudio/Common7/Tools
* VisualStudio/Common7/IDE
* .../Microsoft SDKs/Windows/v7.1A/Bin (for uuidgen in deploy script)
* .../WiX Toolset v3.11/bin
* .../WinRAR

Run `./script/deploy_windows.sh` script which will generate 32bit/64bit builds and packages these into a portable `.zip` file and a Wix-based Windows installer, each.

### macOS

After building, run the `bundle_install.sh` script within the build directory which will create a `Sourcetrail.app` bundle and generate a `Sourcetrail_<version>.dmg` container.

### Linux

Run `./script/buildonly.sh package`


# How to Run the Tests

The automated test suite of Sourcetrail is powered by [Catch2](https://github.com/catchorg/Catch2). To run the tests, simply execute the `Sourcetrail_test` binary. Before executing, please make sure to set the working directory to `./bin/test`.


# License

Sourcetrail is licensed under the [GNU General Public License Version 3](LICENSE.txt).

# Trademark

The "Sourcetrail" name is a trademark owned by Coati Software and is not included within the assets licensed under the GNU GPLv3.
