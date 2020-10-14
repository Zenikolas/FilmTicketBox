# FilmTicketBox
REST API training service which allows manage cinemas and buy tickets



### Build and Install

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

### Usage
See usage scenario [here](./usage.http)

### Testing
- install the following packages to run test in `cinema_test.py` file
```
pip3 install -U requests Flask pytest pytest-html
```

- run the application
```
./filmTicketBox -p 20322
```
- run tests
```
cd <PROJECT_DIRECTORY>
pytest
```

### Issues

- add unit tests
- add logging by levels and log all errors
- provide pytest port arg