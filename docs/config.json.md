# config.json

## main_info  

*config.json is a file that every project on bumake needs to have (more on that later).*
*it contains a lot of settings that tell how bumake should build the project.*
*as the name says, it is written in [json](https://json.org)*

[extra info](#extra_info)
## settings
```text
name

>the name of the generated program
>has a string value

files

>any files that have code that the program uses
>have an array value

prebuild

>set of commands that the program runs before building (in the OS terminal)
>can be either a string or an array

afterbuild

>commands that the program runs after building if successful (in the OS terminal)
>can be either a string or array

rebuild

>deletes temp (recompiles the project)
>doesn't have value

dependencies

>downloads any lib the project may need
>probably doesn't work
>has an array value

languages

>tell bumake what languages bumake should use
>has an array value

compile_tags

>tell bumake what the compile tags should be
>needs a string for every language

link_tags

>tell the tags to pass to the linker
>needs to be a string

makelib

>tells bumake to make a lib (.a)
>doesn't need any value
```


## extra_info
[main info](#main_info)\
you don't actually need to have config.json, much less all of the tags. most of it can be skipped and those that can't bumake will automatically choose a value for them\
still it is recommended to have it and use any value that makes sense for your project.\
for internal files in your project, you don't need to manually create an config.json for them. you can just do:
```json
"name_of_your_file" : {
    "tag1" : "value",
    "tag2" : ["value1","value2"]
}
```




