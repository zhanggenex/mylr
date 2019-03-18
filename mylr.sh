#!/bin/bash
./analyzer/build/lib/lrsan -lrc ./bison-3.0.5/src/bison.bc
cp ./my.cfg /home/hunter-zg/gopath/src/github.com/google/syzkaller/
cd /home/hunter-zg/gopath/src/github.com/google/syzkaller/
./bin/syz-manager -config=my.cfg
