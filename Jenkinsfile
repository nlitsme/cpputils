pipeline {
  agent { label "windows" }
  stages {
    stage("clean") { steps { sh '''git clean -dfx''' } }
    stage("windows-build") {
      steps { 
sh '''#!/bin/bash
set -e
. /c/local/msvcenv.sh
make vc BOOST_ROOT=c:/local/boost_1_74_0 idasdk=c:/local/idasdk75
'''
      }
    }
  }
}
