#!/bin/bash
apt-get install --force-yes -y libxml++2.6-dev libxml++2.6-doc
apt-get install --force-yes -y libboost1.49-all-dev
echo -e "\nCode compilation begin.\n"
cd ../src/index
make clean
make
cp parse ../../bin
make clean
cd ../search
make clean
make
cp search ../../bin
make clean
cd ../../bin
echo -e "\nCode compilation completed.\n"
