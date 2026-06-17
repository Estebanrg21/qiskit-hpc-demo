
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
export QISKIT_IBM_TOKEN=<your API key>
export QISKIT_IBM_INSTANCE=<your CRN>
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

### Output example:
```
➜  demo_ct git:(main) ✗ ./build/demo                                                                      
QRMI connecting : ibm_fez
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0o0oq90bc73e75h5g
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0oi68aqlc73eh5ch0
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0ooe8aqlc73eh5cng
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0ovuhm1is739msfq0
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0p4oq90bc73e75i9g
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0pa29m3dc738p877g
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0pf68aqlc73eh5df0
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0pmehm1is739msgng
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0ptm8aqlc73eh5e10
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0q4m8aqlc73eh5e90
 QRMI Estimator job submitted to ibm_fez, JOB ID = d8p0qc0q90bc73e75jkg
 ```