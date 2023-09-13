# ForzaTech-encryption-tool
You can use this tool to decrypt any file from any game that uses ForzaTech engine, from Forza Motorsport 6: Apex to Forza Horizon 5. It automatically guesses the right game title and key type using MAC verification. It can also be used to re-encrypt user content (game saves, photos, event lab routes).  
This repository is lack of files containing decryption keys. These files and binaries can be found on [the XeNTaX thread](https://forum.xentax.com/viewtopic.php?t=19015).

## Features
* Arxan TransformIT (GuardIT) encryption, decryption
* Message authentication code (MAC) calculation
* gamedb.slt deobfuscation
* .zip method 22 support

## Keys
* SFS **(decryption)**
  * media\sfsdata
* GameDB **(decryption)**
  * media\Stripped\gamedbRC.slt
* File / ConfigFile **(decryption)**
  * .zip
  * .xml
  * .ini
* Profile **(encryption, decryption)**
  * User_69C2EF99.ProfileData
  * User_69C2EF99.VersionFlags
* ForzaProfileProdContainer.LevelRewardCache **(encryption, decryption)**
* Photo **(encryption, decryption)**
  * Photo_0000_20211112155819.header
  * Photo_0000_20211112155819.image *-- jpg*
  * Photo_0000_20211112155819.metadata
  * Photo_0000_20211112155819.thumb *-- jpg*
* Dynamic **(encryption, decryption)**
  * CustomRoute_0000_20211112181230.header
  * CustomRoute_0000_20211112181230.RouteData
* Telemetry **(encryption)**

## TFIT file structure
* u8[16] IV
* u32 padding_size (unencrypted)
* u8[16] header's MAC (unencrypted)
* DataBlock
  * u8[0x200 or 0x20000] data_block (encrypted)
  * u8[16] MAC of decrypted data_block including padding (encrypted)

### Forza Horizon header
* u32 data_size
* u8[16] IV
* u32 padding_size

### Forza Motorsport header
* u32 data_size
* u8[16] IV

### Layers
1. file content
1. data (includes padding)
1. data_block 0x200 or 0x20000
1. block 0x10

### Example
File: media\sfsdata  
Block size: 0x20000  
Padding: 0x6797  
File size: 0x4F827E4  
Data size: 0x4F80000  
Decrypted file size: 0x4F79869

## Usage
```
> .\EncryptionTool.exe -h

Options:
  -h [ --help ]                         Print this help message.
  -v [ --verbose ] number (=1)          Set verbosity level. (0 - no output, 1 - default, 2 - verbose)
  -m [ --mode ] number (=1)             (0 - encryption, 1 - decryption)
  -i [ --input ] string                 Set the input file path.
  -o [ --output ] string                Set the output file path. ('-' - stdout)
  -y [ --yes ]                          Overwrite output file, if exists.

Encryption options:
  -g [ --game ] string                  Game title. (FM6Apex, FH3, FM7, FH4, FH5)
  -k [ --key ] string                   Key type. (SFS, GameDB, File, ConfigFile, Profile, Reward, Photo, Dynamic)
  --iv string (=00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00)
                                        Initialization vector. Hex string of length 32.
```

### Decryption
```
.\EncryptionTool.exe -i"C:\Program Files (x86)\DODI-Repacks\Forza Horizon 5\media\Stripped\gamedbRC.slt" -o"gamedbRC.slt"
```
```
.\EncryptionTool.exe -i"C:\Program Files (x86)\DODI-Repacks\Forza Horizon 5\media\ProfileSchema.zip" -o"ProfileSchema.zip"
```
```
.\EncryptionTool.exe -i"C:\Users\Public\Documents\EMPRESS\1551360\remote\1551360\remote\1774383001\User_69C2EF99.ProfileData" -o"User_69C2EF99.ProfileData"
```

### Encryption
```
.\EncryptionTool.exe -m0 -gFH5 -kProfile --iv="0C CF 15 0C A7 23 A0 23 7A A2 45 63 38 E0 4A 0C" -i"User_69C2EF99.ProfileData" -o"C:\Users\Public\Documents\EMPRESS\1551360\remote\1551360\remote\1774383001\User_69C2EF99.ProfileData"
```

## Requirements
1. [boost 1.82.0](https://boostorg.jfrog.io/artifactory/main/release/1.82.0/binaries/)

## Links
1. [TransformIT decryptor 1](https://github.com/Neodymium146/gta-toolkit/blob/master/RageLib.GTA5/Cryptography/GTA5Encryption.cs)
1. [TransformIT decryptor 2](https://github.com/0x1F9F1/Swage/blob/master/src/crypto/tfit.cpp)
1. [TransformIT file pattern](https://github.com/Nenkai/010GameTemplates/blob/main/Forza/TFIT.bt)
