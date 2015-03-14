Install from source
```
# Extract the archive and change to the directory
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

Run the following command to install the necessary dependencies
```
sudo apt-get install build-essential automake autoconf libtool libgtkmm-2.4-dev libsqlite3-dev libtag1-dev libmagick++-dev libxdg-basedir-dev libnotify-dev libflac++-dev libmad0-dev libavformat-dev libavcodec-dev libavutil-dev libpulse-dev intltool
```