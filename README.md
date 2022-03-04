# RedBase

## Introduction

It's a project of stanford class [CS346](https://web.stanford.edu/class/cs346/2015/).

## Dependencies

This project depend on googletest and readline. you can install it by running the following command.

### macOS

```shell
brew install googletest readline
```

## Build project

In workspace folder, run the following command to generate executable file in `build/bin`.

```shell
make
```

## Test

This command will run all test cases of this project.
```shell
make run_test
```

List all test cases and run the specified test cases only by the following commands.

```shell
make run_test cmd_args="--gtest_list_tests"
make run_test cmd_args="--gtest_filter=*testCaseName"
```