#!/bin/sh

if [ -d $WORKSPACE/build ]
then
  rm -rf $WORKSPACE/build
fi
if [ -d $WORKSPACE/install ]
then
  rm -rf $WORKSPACE/install
fi
mkdir $WORKSPACE/build
mkdir $WORKSPACE/install
cd $WORKSPACE/build

cmake $WORKSPACE
make
make dist
make distcheck
tar xzf `cat VERSION`.tar.gz
cd `cat VERSION`

if [ "$build_mode" = "Debug" ]
then
cmake -Denable_coverage=ON -Denable_model-checking=OFF -Denable_lua=ON -Denable_compile_optimizations=OFF .
fi

if [ "$build_mode" = "ModelChecker" ]
then
cmake -Denable_coverage=ON -Denable_model-checking=ON -Denable_lua=ON -Denable_compile_optimizations=OFF .
fi

if [ "$build_mode" = "DynamicAnalysis" ]
then
cmake -Denable_lua=OFF -Denable_tracing=ON -Denable_smpi=ON -Denable_compile_optimizations=OFF -Denable_compile_warnings=ON -Denable_lib_static=OFF -Denable_model-checking=OFF -Denable_latency_bound_tracking=OFF -Denable_gtnets=OFF -Denable_jedule=OFF -Denable_mallocators=OFF -Denable_memcheck=ON .
fi
make