
# Project compilation
> #### Note:
> At the moment, to compile the C API dependencies it's still needed python as a build dependency.

## 1. Create python virtual env
> Run this in the root of the project
```
python3 -m venv env
```

## 2. Source the env
> Run this in the root of the project
```
source ./env/bin/activate
```

# Demo code compilation

## 1. Set library paths
> Run this in the root of the project
```
export DYLD_LIBRARY_PATH=$PWD/qiskit/dist/c/lib:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=$PWD/qiskit/python/lib:$DYLD_LIBRARY_PATH
export PYO3_PYTHON=$PWD/env/bin/python3
```

## 2. Compile the demo code
> Run this in the root of the project
```
mkdir -p build && cd build && $(cmake .. && make) || true && cd ..    
```
## 3. Execute demo code
```
./build/demo
```