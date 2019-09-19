# How to install C++17 in Ubuntu

## Install the essential
The developer must first make sure that they have `build-essential` installed. This will give a load of developer tools needed. Simply run:

```
sudo apt update
sudo apt install build-essential
```
Once that has finished, make sure you are able to run `g++ --version` in your command line. At the time this was written, gcc and g++ versions 7 are installed. However, gcc and g++ 8 need to be installed. In order to do that simply run:

```
sudo apt install gcc-8
sudo apt install g++-8
```
Once those have finished installing you will need to update their priorities in their linkers. This is simply done:

```
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8
```

Now run `g++ --version` and version 8 should now be installed.
