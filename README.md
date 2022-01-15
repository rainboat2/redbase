# RedBase

## Introduction

it's a project of stanford class [CS346](https://web.stanford.edu/class/cs346/2015/).

## Dependencies

this project depend on googletest. you can install it by running the follow command.

### macOS
 
```shell
brew install googletest
```

## Build project

in workspace folder, run the following command to generate executable file `bin/redbase`.

```shell
mkdir build bin lib
make
```

## Test

run the following command to run testcase of this project.
```shell
make run_test
```