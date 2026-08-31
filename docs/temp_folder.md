#temp

temp is an folder that bmakes automaticaly makes for every projects it compiles.

***do not mess with temp unless you know what you are doing. if you need bmake to not use it delete it or use -recompile***


if an project has just a single code file called ol.cpp, and the project name is ol the temp folder structure should look this:
```text
├ol
├ol.o
├ol.a
└hash
  └ol.cpp.hash
```
so basicaly:
hash/ol.cpp.hash mantains a hash(a number generated from a file) that it generates a new one every time and compares it.
if they match, nothing happens, the file did not change. if they don't match, it recompiles the file.
ol alone is the executable. it will be copied to the parent folder and the version in temp can be used as a backup.
ol.a and ol.o are just temporary files and can be ignored.
