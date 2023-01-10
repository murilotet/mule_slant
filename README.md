# Slanted 4D-TM

### Proposed Slanted 4D-Transform mode

 When you build it using CMake/make (see below) it generates:

 1. **A Slanted 4D-TM encoder command-line executable: `bin/encoder`**
 2. **A Slanted 4D-TM decoder command-line executable: `bin/decoder`**


## Usage

### Prerequisites

You will need:

 * A modern C/C++ compiler
 * CMake 3.1+ installed (on a Mac, run `brew install cmake`)
 * If you prefer to code in a IDE, recommend [Visual Studio Code](https://code.visualstudio.com/) on Mac OS and Linux and [Visual Studio 2022](https://visualstudio.microsoft.com/) on Windows. The IDEs are compatible with this project.

### Building The Project

#### Git Clone

First we need to check out the git repo:

```bash
❯ mkdir ~/workspace
❯ cd ~/workspace
❯ git clone git@github.com:gutolive/mule-slant.git my-project
❯ cd my-project
```

#### Project Structure

There is a empty folder  `bin`, populated by `make install` commnad.

The rest should be obvious: `src` is the sources, and `examples` is where we put examples of scripts for encoding and decoding our unit tests.

Now we can build this project, and below we show three separate ways to do so.

#### Building Commands

On Unix-based operating systems, buiding is performed via the following commands:

```bash
❯ mkdir build
❯ cd build
❯ cmake ..
❯ make && make install
❯ cd ../bin
```

On Windows, the CMake GUI can be used to build the executables. Some [TUTORIALS](https://preshing.com/20170511/how-to-build-a-cmake-based-project) can be found on the web.

### Running the CLI Executables

### Commands 

The encoder and decoder executables must be executed passing the path od conf file as argument. 
Example:

```bash
❯ bin/encoder -cf examples/BIKES/mule_slant_BIKES_lambda2048_encode.conf

❯ bin/decoder -cf examples/BIKES/mule_slant_BIKES_lambda2048_decode.conf

```

### License

&copy; 2023 Murilo Bresciani de Carvalho.

Open sourced under MIT license, the terms of which can be read here — [MIT License](http://opensource.org/licenses/MIT).

