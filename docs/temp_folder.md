# temp

temp is a folder that bmake automaticaly makes for every project it compiles.

***do not mess with temp unless you know what you are doing. if you need bmake to not use it delete it or use -recompile***


if a project has just a single code file called ol.cpp, and the project name is ol the temp folder structure should look this:
```text
├ol
├ol.o
├ol.a
└hash
  └ol.cpp.hash
```
so basically:
hash/ol.cpp.hash mantains a hash (a number generated from a file) that is generated every time bmake runs and compares to the previous one.\
if they match, nothing happens, the file did not change. if they don't match, it recompiles the file.
ol alone is the executable. it will be copied to the parent folder and the version in temp can be used as a backup.
ol.a and ol.o are just temporary files and can be ignored.
