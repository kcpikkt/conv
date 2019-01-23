# conv
Simple cli number system converter written in C.

## Installation
### Linux
```console
git clone https://github.com/kcpikkt/conv
cd conv
make
sudo make install
```

## Usage
```console
$ conv 7 81
7       111
81      1010001
$ conv -f=16 -t=10 1FA 1fa
1FA     506
1fa     506
$ conv -f=16 -t=10 1FA -t=2 1FA
1FA     506
1FA     111111010
$ conv -f=10 -t=36 46655 -c=1 46655
46655   zzz
46655   ZZZ
```
 
