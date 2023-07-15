# SAPHERE

This repository contains the code for the experimental analysis within the paper "Secure Plaintext Acquisition of Homomorphically Encrypted Results for Remote Processing" (SAPHERE) by Pia Baumstark, David Monschein, and Oliver Waldhorst. It is part of the 48th [IEEE Conference on Local Computer Networks](https://www.ieeelcn.org/) (LCN).

## Contents
- [Acknowledgments](#acknowledgments)
- [Getting Started](#getting-started)
  - [Running SAPHERE on Windows](#running-saphere-on-windows)
  - [Frequent errors](#frequent-errors)
- [Experimental analysis](#experimental-analysis)
  - [Implementation remarks](#implementation-remarks)
  - [Experiment Results](#experiment-results)

## Acknowledgments
Funded by the German Federal Ministry of Education and Research (BMBF), RefNr. 16KIS1142K

## Getting Started
For easy usage, we recommend the package manager [vcpkg](https://github.com/microsoft/vcpkg) to install the library [SEAL](https://github.com/microsoft/SEAL) as well as the [Visual Studio](https://visualstudio.microsoft.com/de/) IDE.

### Running SAPHERE on Windows
1. Make sure you have Visual Studio installed. If not, install from [here](https://visualstudio.microsoft.com/de/). In the Visual Studio Installer, we recommend adding the workload "Desktop development with C++" as well as the individual components "Windows 10 SDK" and "C++-CMake-Tools for Windows". Additionally, selecting the English language pack is typically the best option.

2. Make sure you have the package manager ```vcpkg``` installed. If not, follow the instructions [here](https://github.com/microsoft/vcpkg#quick-start-windows). If you are on an x64 machine, consider creating the system variable ```VCPKG_DEFAULT_TRIPLET=x64-windows```, which will result in ```vcpkg``` installing the x64 version of libraries by default.
3. Use ```vcpkg``` to install SEAL as described [here](https://github.com/microsoft/seal#installing-from-vcpkg). 
4. Clone this repo and open the contained project by clicking the ```SAPHERE.sln``` file.
5. Build and execute the code by pressing Ctrl + F5. If an error occurs consider the list below.

### Frequent errors

#### Visual Studio cannot resolve ```#include "seal/seal.h"```
Do the following: 
- Right-click on the project in the solution explorer > Properties > VC++ Directories. To "Include Directories" add the path to your SEAL installation within ```vcpkg```. For example, if you have version 4.1 installed, this will be something like ```$(VCPKG_ROOT)\installed\x64-windows\include\SEAL-4.1```. Here, ```VCPKG_ROOT``` is a system variable set to the path of your vcpkg root directory (e.g. ```C:\dev\vcpkg```).


#### For_each_n is not a part of std
You have to use a newer version of C++. Do the following:
- Right-click on the project in the solution explorer > Properties > General. Click on the dropdown beside "C++ Language Standard" and select "ISO C++ 20 Standard".


#### Error: zstd.ll not found
This error typically occurs after terminating the program incorrectly. Consider not force-terminating with Ctrl + C. 
In most cases, performing a clean and rebuilding the project solves this error. Do the following:
- Right-click on the project in the solution explorer > Clean. Now, build and execute the code anew by clicking Ctrl + F5.

## Experimental analysis

### Implementation remarks
The implementation is written in C++ and uses the library [SEAL](https://github.com/microsoft/SEAL) by Microsoft.
All plaintexts are saved as elements of the ```seal::Plaintext``` class.
All ciphertexts, regardless of the number of contained encryptions, are saved as elements of the ```seal::Ciphertext``` class.
In CKKS, a ciphertext is a 2-tuple, with each tuple element having the same size as a plaintext.
Thus, whenever a ciphertext is split into its two elements, we fill two dummy plaintexts with data from the original ciphertext.
When concatenating two plaintexts, we fill a dummy ciphertext with data from the original plaintexts.
For filling, we use the ```data()```-function provided by both the ```seal::Plaintext``` and the ```seal::Ciphertext``` class in combination with simple ```for```-loops.

Having the data always be a ```seal::Plaintext``` or a ```seal::Ciphertext``` enables us to utilize the standard SEAL functions for encoding, decoding, encryption, and decryption provided by the ```CKKSEncoder```, ```Encryptor```, and ```Decryptor``` classes.
Therefore, we do not have to implement them ourselves.

We utilize the following encryption parameters:
- Scheme type: CKKS
- Polynomial modulus degree: 8192
- Bit sizes (coefficient modulus creation): {60, 40, 40, 60}
- Scale: 2^40

### Experiment Results
We executed the code on an Intel Core i7--7500U CPU @ 2.90 GHz of 2 cores and 16 GB of RAM running with 2133 MHz.
This yielded the output shown in [exemplary_output.txt](exemplary_output.txt).
Consider the following tables for a concise overview.

#### Absolute runtime values of the performed steps:
| Runtime (in ms)         | min    | M      | max    | σ     | μ      |
|-------------------------|--------|--------|--------|-------|--------|
| **Standard version**    |        |        |        |       |        |
| Step 2 (data owner)     | 55.89  | 56.74  | 68.62  | 1.29  | 57.21  |
| **SAPHERE**             |        |        |        |       |        |
| Step 1 (processor)      | 151.93 | 152.95 | 215.86 | 4.01  | 154.13 |
| Step 3 (data owner)     | 13.68  | 13.75  | 23.73  | 0.50  | 13.88  |
| Step 5 (processor)      | 57.51  | 58.09  | 84.40  | 1.70  | 58.48  |
* $min$: minimum; $M$: median; $max$: maximum; $σ$: sample standard deviation; $μ$: mean

#### Aggregated absolute runtime values at the processor, the data owner, and in total:
| Runtime (in ms)    | min    | M      | max    | σ     | μ      |
|--------------------|--------|--------|--------|-------|--------|
| **Processor**      |        |        |        |       |        |
| Standard           | 0.00   | 0.00   | 0.00   | 0.00  | 0.00   |
| SAPHERE            | 209.72 | 211.24 | 296.44 | 5.19  | 212.61 |
| **Data owner**     |        |        |        |       |        |
| Standard           | 55.89  | 56.74  | 68.62  | 1.29  | 57.21  |
| SAPHERE            | 13.68  | 13.75  | 23.73  | 0.50  | 13.88  |
| **Total**          |        |        |        |       |        |
| Standard           | 55.89  | 56.74  | 68.62  | 1.29  | 57.21  |
| SAPHERE            | 223.43 | 225.01 | 320.17 | 5.56  | 226.49 |
* $min$: minimum; $M$: median; $max$: maximum; $σ$: sample standard deviation; $μ$: mean

#### Transmitted elements and their respective data sizes:
- Standard version:
  - Step 1: ```seal::Ciphertext``` (524.29 kilobytes)
  - Step 3: ```std::vector<double>``` (32.77 kilobytes)

- SAPHERE:
  - Step 2: ```seal::Ciphertext``` (524.29 kilobytes)
  - Step 4: ```seal::Plaintext``` (262.14 kilobytes)

#### Absolute error introduced to the ciphertext:
| Introduced Error (Scale: E-10) | min      | M     | max     | σ     | μ      |
|---------------------------------------|----------|-------|---------|-------|--------|
| Standard version                      | 0.0093   | 6.38  | 77.68   | 9.02  | 9.04   |
| SAPHERE                               | < 0.0001 | 10.21 | 159.38  | 11.77 | 13.38  |
* $min$: minimum; $M$: median; $max$: maximum; $σ$: sample standard deviation; $μ$: mean