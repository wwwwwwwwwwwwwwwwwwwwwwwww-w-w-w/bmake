# config.json



*config.json is a file that every project on bmake needs to have(more on that later).*
*it contains a lot of settings that tell how to bmake should build the project.*
<a name="settings">

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
</a>

