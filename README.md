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
### parameters in the encoder's configuration file
  
-lf <string>  : path to input light field directory  
-o <string> : path to output compressed file  
-nv <integer value> : number ov vertical views  
-nh <integer value> : number ov horizontal views  
-off_h <integer value> : first horizontal view index  
-off_v <integer value> : first vertical view index  
  
-lambda <float value> : Lagrange multiplier that controls the operation point. Increasing lambda reduces the rate while increasing the distortion  
-u <integer value> : maximum transform length at u coordinate  
-v <integer value> : maximum transform length at u coordinate  
-s <integer value> : maximum transform length at u coordinate  
-t <integer value> : maximum transform length at u coordinate  
-min_u <integer value> : minimum transform length at u coordinate  
-min_v <integer value> : minimum transform length at v coordinate  
-min_s <integer value> : minimum transform length at s coordinate  
-min_t <integer value> : minimum transform length at s coordinate  
-t_scale <float value> : gain of the DCT transform applied at direction t (defaults to 1.0)  
-s_scale <float value> : gain of the DCT transform applied at direction s (defaults to 1.0)  
-v_scale <float value> : gain of the DCT transform applied at direction v (defaults to 1.0)  
-u_scale <float value> : gain of the DCT transform applied at direction u (defaults to 1.0)  
   
-max_slant <integer value> : search range for the optimum slant is [-max_slant, max_slant]  
-min_slant_block_u <integer value> : blocks with less than this size in the u coordinate will not be slanted   
-min_slant_block_v <integer value> : blocks with less than this size in the v coordinate will not be slanted   
   
-lenslet13x13 : to be used when the light field was captured with a lenslet camera  
   
 Any one of these commands in the configuration file can be overriden by placing them in command line when invoking the encoder  
   
 ### parameters in the decoder's configuration file  
  
-lf <string>  : path to the output reconstructed light field directory  
-i <string> : path to the input compressed file  
-nv <integer value> : number ov vertical views  
-nh <integer value> : number ov horizontal views  
-off_h <integer value> : first horizontal view index  
-off_v <integer value> : first vertical view index  
  
-t_scale <float value> : gain of the DCT transform applied at direction t (must be set as the inverse of the value used when encoding)  
-s_scale <float value> : gain of the DCT transform applied at direction s (must be set as the inverse of the value used when encoding)  
-v_scale <float value> : gain of the DCT transform applied at direction v (must be set as the inverse of the value used when encoding)  
-u_scale <float value> : gain of the DCT transform applied at direction u (must be set as the inverse of the value used when encoding)  
   
-lenslet13x13 : to be used when the light field was captured with a lenslet camera  
   
 Any one of these commands in the configuration file can be overriden by placing them in command line when invoking the encoder   
  

### License

&copy; 2023 Murilo Bresciani de Carvalho.

Open sourced under MIT license, the terms of which can be read here — [MIT License](http://opensource.org/licenses/MIT).

