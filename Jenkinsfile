pipeline {
  agent none
  stages {
    stage("buildall") {
      matrix {
        agent {
          label "${label}"
        }
        axes {
          axis { name "label"; values "linux", "macos", "windows", "freebsd" }
          axis { name "compiler"; values "gcc", "clang_llvm", "clang_stdc", "default" }
        }
        excludes {
          exclude {
            axis { name "label"; values "windows" }
            axis { name "compiler"; values "gcc", "llvm" }
          }
          exclude {
            axis { name "label"; values "macos", 'linux' }
            axis { name "compiler"; values "default" }
          }
        }

        stages {
          stage("clean") { steps { sh '''git clean -dfx''' } }
          stage("windows-build") {
            when { environment name: 'label', value: 'windows' }
            steps { 
sh '''#!/bin/bash
set -e
. /c/local/msvcenv.sh
make vc
./build/Debug/unittests.exe
'''
            }
          }
          stage("unix-build") {
            when { not { environment name: 'label', value: 'windows' } }
            steps { 
sh '''#!/bin/bash
set -e
case "$label" in
  freebsd)
    MAKE=gmake
    CLANGSUFFIX=90
    GCCSUFFIX=10
  ;;

  macos)
    MAKE=make
    CLANGSUFFIX=
    GCCSUFFIX=-10
  ;;

  linux)
    MAKE=make
    CLANGSUFFIX=-10
    GCCSUFFIX=-10
  ;;
esac

case "$compiler" in
  clang*)
    MAKEARGS=(CXX=clang++$CLANGSUFFIX CC=clang$CLANGSUFFIX)
  ;;
  gcc*)
    MAKEARGS=(CXX=g++$GCCSUFFIX CC=gcc$GCCSUFFIX)
  ;;
esac

case "$compiler" in
  clang_stdc)
    PREFIXARGS+=(CFLAGS=-stdlib=libstdc++)
    PREFIXARGS+=(LDFLAGS=-stdlib=libstdc++)
    ;;
  clang_llvm)
    PREFIXARGS+=(CFLAGS=-stdlib=libc++)
    PREFIXARGS+=(LDFLAGS=-stdlib=libc++)
    ;;
esac
MAKEARGS+=(cmake)
eval "${PREFIXARGS[@]}" "$MAKE" "${MAKEARGS[@]}"


./build/unittests
'''
            }
          }
        }
      }
    }
    stage("toplevel-end")
    {
      steps { echo ".... the end ....." }
    }
  }
}


