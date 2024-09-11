## example server plugin for Deadlock

adds two console commands: `example` which gives the player 100000 souls and `giveme` which gives the player the named item (eg giveme upgrade_magic_carpet)

### Usage:

1. create folder `addons/example/bin/win64` in the `citadel` game folder
2. add compiled dll to the new folder named as `server.dll`
3. add file named `example_x64.vdf` to `addons` with contents:

```
"Plugin"
{
	"file"	"addons/example/bin/win64/server"
}
```

4. in `citadel/gameinfo.gi` add `Game citadel/addons/example` at around line 40 under `SearchPaths`
5. launch `project8.exe -dedicated`

### Building:

1. clone repository with submodules
2. build `hl2sdk/tier1/tier1.sln` and place `tier1.lib` in `hl2sdk/lib/public/win64`
3. build solution
4. open an issue if theres more steps i forgot 😺

### Other things:

- metamod is only included as a submodule for sourcehook as standalone. this doesnt use metamod
- hl2sdk is checked out on the cs2 branch since everything ive run into so far is equivalent between cs2 and deadlock (except CGlobalVars)
