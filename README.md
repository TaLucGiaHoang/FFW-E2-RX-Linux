# FFW E2 RX Linux
### Prerequisite
Need to have libusb-1.0 installed.
```
$ sudo apt-get install libusb-1.0-0-dev
```
### Build libcommuni.so
To build and install libcommuni.so use:
```
$ cd communi/
$ make install
```
### Build libffw.so
To build and install libffw.so use:
```
$ cd FFW/Src/FFWE20RX_EML
$ mkdir build
$ make
$ make install
```
### After installation
The header files will be installed to
```
/usr/include/communi
/usr/include/FFW
```
The library will be installed to
```
/usr/lib
```

## Test libcommuni.so and libffw.so
### Build the test app
```
$ cd FFW/Src/FFWE20RX_EML
$ make examples

```
### Run the test app
We need the root right to access the usb device.
```
$ sudo ./main
```

The test app will do below tests:
```
1. GetEINF and print the result

2. Test writing to address 0x3000 using EBYTE_ACCESS
  + Set all bytes from 0x3000 to 0x3028 to 'f' (CPUW)
  + Read 8 bytes from 0x3000 using EBYTE_ACCESS (CPUR): "ffffffff"
  + Write 8 bytes "12345678" to address 0x3000 (CPUW)
  + Read 8 bytes from 0x3000 using EBYTE_ACCESS (CPUR): "12345678"

3. Test writing to address 0x3008 using EWORD_ACCESS
  + Read 8 bytes from 0x3008 using EWORD_ACCESS (CPUR): "ffffffff"
  + Write 8 bytes "12345678" to address 0x3008 (CPUW)
  + Read 8 bytes from 0x3008 using EWORD_ACCESS (CPUR): "12345678"

4. Test writing to address 0x3010 using ELWORD_ACCESS
  + Read 8 bytes from 0x3010 using ELWORD_ACCESS (CPUR): "ffffffff"
  + Write 8 bytes "12345678" to address 0x3010 (CPUW)
  + Read 8 bytes from 0x3010 using ELWORD_ACCESS (CPUR): "12345678"

5. Test writing to address 0x3018 using EBYTE_ACCESS
  + Read 16 bytes from 0x3018 using EBYTE_ACCESS (CPUR): "ffffffffffffffff"
  + Write 16 bytes "0123456789ABCDEF" to address 0x3018 (CPUW)
  + Read 16 bytes from 0x3018 using EBYTE_ACCESS (CPUR): "0123456789ABCDEF"
```
The output log file is located at FFW\Src\FFWE20RX_EML\main.log

## NDEB
### Build NDEB sample app
```
$ cd NDEB/src
$ make all

```
### Run NDEB sample app
We need the root right to access the usb device.
```
$ sudo ./init_ndeb_sample
```

(*) Make sure to have these files in the same directory with NDEB sample app:
```
WTRRX230.bin
E2_RXRL.bit
00000000_0000ffff_inc_00.mot

```
The NDEB sample app will do below tests:
```
1. Initialize Sequence E2
2. Target memory access
2.1. Target memory write
  + Write a file of incremental data (00000000_0000ffff_inc_00.mot) from 00 to the address 0 to ffff
  + Write incremental data from 0 to F to addresses 0 to f in byte size

2.2 Target memory read
  + Read data of address 0 to ff in byte size:
  ADDRESS    +0 +1 +2 +3 +4 +5 +6 +7   +8 +9 +A +B +C +D +E +F   ASCII
  00000000   00 01 02 03 04 05 06 07   08 09 0A 0B 0C 0D 0E 0F   ................
  00000010   10 11 12 13 14 15 16 17   18 19 1A 1B 1C 1D 1E 1F   ................
  00000020   20 21 22 23 24 25 26 27   28 29 2A 2B 2C 2D 2E 2F    !"#$%&'()*+,-./
  00000030   30 31 32 33 34 35 36 37   38 39 3A 3B 3C 3D 3E 3F   0123456789:;<=>?
  00000040   40 41 42 43 44 45 46 47   48 49 4A 4B 4C 4D 4E 4F   @ABCDEFGHIJKLMNO
  00000050   50 51 52 53 54 55 56 57   58 59 5A 5B 5C 5D 5E 5F   PQRSTUVWXYZ[\]^_
  00000060   60 61 62 63 64 65 66 67   68 69 6A 6B 6C 6D 6E 6F   `abcdefghijklmno
  00000070   70 71 72 73 74 75 76 77   78 79 7A 7B 7C 7D 7E 7F   pqrstuvwxyz{|}~.
  00000080   80 81 82 83 84 85 86 87   88 89 8A 8B 8C 8D 8E 8F   ................
  00000090   90 91 92 93 94 95 96 97   98 99 9A 9B 9C 9D 9E 9F   ................
  000000A0   A0 A1 A2 A3 A4 A5 A6 A7   A8 A9 AA AB AC AD AE AF   ................
  000000B0   B0 B1 B2 B3 B4 B5 B6 B7   B8 B9 BA BB BC BD BE BF   ................
  000000C0   C0 C1 C2 C3 C4 C5 C6 C7   C8 C9 CA CB CC CD CE CF   ................
  000000D0   D0 D1 D2 D3 D4 D5 D6 D7   D8 D9 DA DB DC DD DE DF   ................
  000000E0   E0 E1 E2 E3 E4 E5 E6 E7   E8 E9 EA EB EC ED EE EF   ................
  000000F0   F0 F1 F2 F3 F4 F5 F6 F7   F8 F9 FA FB FC FD FE FF   ................

  + Read data of address 0 to 3f in word size:
  ADDRESS    +1+0 +3+2 +5+4 +7+6   +9+8 +B+A +D+C +F+E   ASCII
  00000000   0100 0302 0504 0706   0908 0B0A 0D0C 0F0E   ................
  00000010   1110 1312 1514 1716   1918 1B1A 1D1C 1F1E   ................
  00000020   2120 2322 2524 2726   2928 2B2A 2D2C 2F2E   ! #"%$'&)(+*-,/.
  00000030   3130 3332 3534 3736   3938 3B3A 3D3C 3F3E   1032547698;:=<?>

  + Read data of address 0 to 3f in lword size:
  ADDRESS    +3+2+1+0 +7+6+5+4   +B+A+9+8 +F+E+D+C   ASCII
  00000000   03020100 07060504   0B0A0908 0F0E0D0C   ................
  00000010   13121110 17161514   1B1A1918 1F1E1D1C   ................
  00000020   23222120 27262524   2B2A2928 2F2E2D2C   #"! '&%$+*)(/.-,
  00000030   33323130 37363534   3B3A3938 3F3E3D3C   32107654;:98?>=<

```
The output log file is located at NDEB\bin\init_ndeb_sample.log


## Run NDEB sample app (portable)
(*) No need to build and install libcommuni.so, libffw.so

### Extract binaries
Copy FFWE2RX.tar.gz to Linux host

Extract tar ball
```
$ tar -zxvf FFWE2RX.tar.gz
```

### Run NDEB sample app
Plug in E2 ICE and board to Linux host, then run below commands to start the sample app:
```
$ cd FFWE2RX/
$ sudo ./run.sh 
```

