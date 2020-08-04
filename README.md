# SysMainView

This tool is designed to interact with the prefetch files: both databases files (.db)
and scenario files (.pf). It includes many uses:
- Compression/decompression
- Parsing information
- Editing the data 


Feel free to give any suggestions !

Contributors : Mathilde Venault ; Baptiste David

## 1. Run

Prerequisites: having Visual Studio 2019 

- Dowload the sources files:
``` 
 git clone https://github.com/MathildeVenault/SysMainView.git
``` 
- Open  ``SysMainView.sln`` with Visual Sutdio 2019
- Build the project with ``x64`` & ``Release`` option
- Run ``C:\%YourDirectory%\x64\Release\SysMainView.exe`` 
- Select on the menu what you want to do and follow the guide.

/!\ Make sure you have your .db/.pf files ready to be used. Otherwise, you can use the sample files attached on the directory ``Sample``.


## 2. Uses and opportunities

SysMainView will help you through your forensic experience for educationnal purposes. This is not only about having the list of all the 
programs executed on a machine because it also gives you the count of executions, the dates and times of the 8 last executions,
the names and locations of the files each program uses, and so on.. Therefore it helps you : 
- Prooving software installs
- Profiling the schedule and habbits of the user
- Finding out names and locations of personal files
- Falsifying evidences

To find more about SysMain and Superfetch you can dowload the slides from our [talk at Black Hat USA 2020](https://www.blackhat.com/us-20/briefings/schedule/#fooling-windows-through-superfetch-20201)


## 3. Details about the prefetch files

The prefetch files are divided in two categories :
- the scenario files (.pf) related to applications
- the databases files (.db) related to the agent's activities.

### 3.1 Scenarios 
Scenarios are the traces of each program's activity. Some of their properties such as their maximum size or the maximum number of prefetch files recorded could be found within the registry keys :
 ``HKML\SOFTWARE\Microsoft\WindowsNT\CurrentVersion\Prefetcher``

 ##### 3.1.1 Scenario names 
 Their names are composed by :
 ``ProgramNames-8DIGITS.pf``
 The 8 digits correspond to a hash value, which is calculated for one application given one execution context. Therefore, one application might have more than one scenario associated. Please find SysMain's hash pseudo code under Windws 10 below. But be careful, this algorithm depends on your Windows version and might change on the future!
```c
StringHashed = 314159;
for (i = 0; i< length(StringHashed); i++)
{
    Char = StringToHash[i];
    UpChar = RtlUpcaseUnicodeChar(Char);
    StringHashed = (StringHashed * 37 + UpChar ) * 37;
}
```

#### 3.1.2 Scenario Format 

Scenarios files are compressed, within the  `` XPRESS_HUFFMAN `` algorithm, and here is their format decompressed.

| Offset        |    Size         |  Content   |
| ------------- | -------------   | ---------  |
| 0x00     |       4       |     OS id |
| 0x04  |        8        |      "SCCA" ; prefetch signature |
 |0x0C     |        4       |    Total decompressed size |
| 0x10     |        60      |     Program's name |
| 0x4C      |        4        |      Hash value (name) |
| 0x50      |        4        |      Flag  |
| 0x58        |        4       |      Count of entries  |
| 0x64      |        4        |      Offset of entries block |
| 0x6C      |        4        |      Offset of volume bloc |
| 0x70      |        4        |      Flag  |
| 0x80        |       64       |      8 last execution dates & times   |
| 0xC8     |        4        |      Count of executions |

### 3.2 Databases
Databases file correspond to SysMain's agents names, components dedicated to specific tasks. Please note the databases are not always present within the ``C:\Windows\Prefetch`` directory.
| Database name     |   Related to  |  
| ------------ | ------------- | 
|AgAppLaunch.db    |  Agent ApplicationLaunch      |   
| AgRobust.db   | Agent Robust performance  |    
| AgCx_%SID%.db        |   Agent Context for a given user |    
| AgGlFaultHistory.db    | Agent Global; fault history      |  
| AgGlFgAppHistory.db  |  Agent Global : foreground history   | 
| AgGlGlobalHistory.db | Agent Global |      value |
| AgGlUAD_P_%SID%.db | Agent Global; User Active Days (assumption)     | 
| dynrespri.7db       | "dynamic reserved priority"  |
| cadrespri.7db   |  ... |   

#### 3.2.1 Database format 
Databases are almost all compressed (except AgRobsut.db, AgAppLaunch.db, dynrespri.7db and cadrespri.7db), within the `` XPRESS_HUFFMAN `` algorithm, and here is their common format decompressed. Each file has its specialities, this is a summary of the common characteristics.

| Offset        |    Size         |  Content   |
| ------------- | -------------   | ---------  |
| 0x00     |       4       |     Magic Number |
| 0x04  |        4        |    Total size |
| 0x08     |       4       |     Header Size |
 |0x0C     |        4        |   Filetype number (index from SysMain's internal array)|
| 0x10     |        4       |     Param 1 |
| 0x14     |        4        |    Param 2 |
| 0x18      |        4        |    Param 3 |
| 0x1C        |        4       |    Param 4 |
| 0x20      |        4        |    Param 5|
| 0x24      |        1        |    Param 6 |
| 0x28      |        4        |     Param 7 |
| 0x2C      |        4        |     Param 8 |
| 0x34        |       64       |      Count of volumes   |
| 0x38     |        4        |      Count of entries|

###### Magic Number 
This is an ID related to groups of databases. 

###### Header Size 
This size must be higher than 72. It is used for offset calculation purposes on the file. 

###### FileType number & Params
Filetype number refers to an index within an internal table on SysMain. This is a table defining 16 sets of 8 numbers which are parameters indicated after the filetype number. The filetype allows SysMain to check that the params from the file match the internal params referenced.

The params have two uses : 
- Offset calculation within the file, such as shown.
- Internal databases size configuration.

## 4. References
[@joachimmetz](https://github.com/joachimmetz), [Windows SuperFetch (DB) format](https://github.com/libyal/libagdb/blob/master/documentation/Windows%20SuperFetch%20(DB)%20format.asciidoc)

## 5. Versions & Licence

 Published under the licence :  [MIT](https://opensource.org/licenses/mit-license.php)

**Initial commit :** version 0.1 (08/2020)

