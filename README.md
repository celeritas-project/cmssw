# CMSSW (Celeritas experiment fork)

To clone a sparse version of cmssw with just this branch
```sh
git clone --branch celeritas-integration --no-checkout https://github.com/celeritas-project/cmssw.git
git config core.sparsecheckout true
cat > .git/info/sparse-checkout <<EOF
/.gitignore
/.clang-tidy
/.clang-format
/README.md
/CMakeLists.txt
/CommonTools
/DataFormats
/DetectorDescription
/FWCore
/SimG4CMS
/SimG4Core
EOF
git checkout -f HEAD
```

A simple CMakeLists.txt file at the top directory lets you create a build
directory with compile commands, enabling vscode usage:
```
mkdir build
cd build
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON ..
```
