# config.json

## main_info  

*config.json is a file that every project on bmake needs to have(more on that later).*
*it contains a lot of settings that tell how to bmake should build the project.*
*as the name says, it is writen in [json](https://json.org)*

[extra info](#extra_info)
## settings
```text
name

>the name of the generated program
>has a string value

files

>any files that have code that the program uses
>has a array value

prebuild

>commands that the program runs before building(in the OS terminal)
>can be either a string or array

afterbuild

>commands that the program runs after building if successful (in the OS terminal)
>can be either a string or array

rebuild

>deletes temp(recompiles the project)
>doesn't have value

dependencies

>downloads any lib the project may need
>probably doesn't works
>has a array value

languages

>tells bmake what languages bmake should use
>has an array value

compile_tags

>tells bmake what the compile tags should be
>needs an string for every language

link_tags

>tells the tags to pass to the linker
>needs to be a string

makelib

>tells bmake to make a lib (.a)
>doesn't need any value
```


## extra_info
[main info](#main_info)
you don't actualy need to have config.json, much less al of the tags. most all of it can be skiped and those that can't bmake will automaticaly chose an value for them\
still it is recommended to have it and use any value that makes sense for your project.\
for internal files in your project, you don't need to manualy create an config.json for them. you can just do:
```json
"name_of_your_file" : {
    "tag1" : "value",
    "tag2" : ["value1","value2"]
}
```




