#!/bin/bash

# Install the development version of libmetal
echo "################### get libmetal #####################"
#git clone "https://github.com/OpenAMP/libmetal/libmetal.git" || {
#  echo "fails to get libmetal"
#  exit 1
#}

dirs_to_check=$PWD
echo "################### build libmetal #####################"
cd libmetal
rm -r build
cmake . -Bbuild || {
  exit 1
}
cd build
make || {
  exit 1
}

cd $dirs_to_check

echo "################### build openamp #####################"
rm -r build
cmake . -DWITH_APPS=on -DWITH_PROXY=on -Bbuild -DCMAKE_INCLUDE_PATH="./libmetal/build/lib/include" -DCMAKE_LIBRARY_PATH="./libmetal/build/lib" || {
  exit 1
}

cd build
make install DESTDIR=./bin || {
  exit 1
}

echo "################### run ping test #####################"
`LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/rpmsg-echo-static` &
sleep 1
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/msg-test-rpmsg-ping-static 1 || {
  exit 1
}

exit 0

echo "################### run update test #####################"
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/msg-test-rpmsg-update-shared &
sleep 1
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/msg-test-rpmsg-ping-shared 1 || {
  exit 1
}
pkill pmsg-echo-shared

echo "################### run matrix test #####################"
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/matrix_multiplyd-shared &
sleep 1
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/matrix_multiply-shared 1 || {
  exit 1
}
pkill pmsg-echo-shared

echo "################### run sample test #####################"
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/rpmsg-sample-echo-shared &
sleep 1
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/rpmsg-sample-ping-shared 1 || {
  exit 1
}
pkill pmsg-echo-shared

echo "################### run flood test #####################"
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/rpmsg-echo-shared &
sleep 1
LD_LIBRARY_PATH=./bin/usr/local/lib:../libmetal/build/lib/  ./bin/usr/local/bin/msg-test-rpmsg-flood-ping-shared 1 || {
  exit 1
}
pkill pmsg-echo-shared
