# FilmTicketBox
REST API training service which allows manage cinemas and buy tickets

### Issues

- add errors description along with status code and reason
- add usage documentation
- add unit test and integration tests
- add logging by levels and log all errors

### Install

1. Install deps for Poco
```
sudo apt-get install openssl libssl-dev
```
2. Get the Poco
```
cd ~
git clone -b master https://github.com/pocoproject/poco.git
```
3. Build and Install Poco
```
cd poco
mkdir build_cmake
cmake -DCMAKE_INSTALL_PREFIX=/usr ../
make -j4
sudo make install
```
4. Build filmTicketBox
```
mkdir build
cd build
cmake ../
make -j4
```
