#!/bin/bash

set -Eeuo pipefail

script_dir="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd)"
project_dir="${script_dir}/.."
output_dir="${project_dir}/coverage"
cpp_cov_output_dir="${output_dir}/cpp-coverage"
cpp_cov_target_file="${project_dir}/build/Debug/nodemgclient.node"

# Cleanup all existing build files.
rm -rf "${project_dir}/build" "${project_dir}/coverage" "${project_dir}/node_modules"

# Generage CPP coverage report based on debug build.
mkdir -p "${cpp_cov_output_dir}"
npm install --prefix "${project_dir}"
npm run build:debug --prefix "${project_dir}"
pushd "${project_dir}/build"
cmake -DENABLE_COVERAGE=True .. && make
popd
LLVM_PROFILE_FILE="${cpp_cov_output_dir}/cpp.profraw" npm run test --prefix "${project_dir}"
llvm-profdata merge -sparse "${cpp_cov_output_dir}/cpp.profraw" -o "${cpp_cov_output_dir}/cpp.profdata"
llvm-cov show "${cpp_cov_target_file}" \
    -format html -instr-profile "${cpp_cov_output_dir}/cpp.profdata" \
    -o "${cpp_cov_output_dir}" \
    -show-line-counts-or-regions -Xdemangler c++filt -Xdemangler -n

# Generate JS coverage report based on release build.
rm -rf "${project_dir}/build" "${project_dir}/node_modules"
npm install --prefix "${project_dir}"
npm run build:release --prefix "${project_dir}"
npm run test:coverage --prefix "${project_dir}"
