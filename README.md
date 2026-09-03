# bmake
just a simple build system.
disclaimer: i admit i coded part of the project with AI and i just tested with arch linux. 

basic project structure:
```text
my-app/
|──config.json
|── bmake
├── c_code/
|    └──main.c
└── cpp_code/
     └──functions.cpp
```

so i will use the project mentioned above as an example. 
because this is just an small example i will assume that main.c contains:
```c
int returnzero();

int main(){
    return returnzero();
}
```
and functions.cpp contains:
```cpp
extern "C"{
    int returnzero(){
        return 0;
    }
}
```

okay, what do we do now? write config.json!
a very basic config .json that would work for this example is:
```json
{
    "name" : "github_example",
    "files" : ["c_code","cpp_code"],
    "c_code" : {
        "languages" : ["c"]   
    },
    "cpp_code" : {
        "languages" : ["c++"]
    }

}
```
the json is simply a way to describe how bmake should behave. in the documentation folder there is more detail about it. by now this should work
so how do we compile it? simple! just use ./bmake on the terminal(with the program in the current folder). you can also give the path to the project you want to compile.
