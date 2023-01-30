## Workflow

Output files will be located, by default, in `build`. You can change that with `BUILD_DIR` parameter in the [Makefile](Makefile).

Just run cmake:

```shell
make cmake
```

Run cmake and build:

```shell
make -j<number of threads>
```

Clean:

```shell
make clean
```

Format all source files:

```shell
make format -j<number of threads>
```