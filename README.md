# Faith
Faith is a request redirector that reroutes traffic from Sony's scehttp lib to your own custom server.

## How Does This Work?
We hook into sceHttpCreateConnectionWithURL and sceHttpCreateRequestWithURL, then modify the URL passed to those functions with a custom one.

## How to Compile

Get VMWare or Use a real Linux machine  
Download the OpenOrbis toolchain from this [link](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/releases/tag/v0.5.3) and GoldHen SDK from this [link](https://github.com/GoldHEN/GoldHEN_Plugins_SDK) and
Store them in a location such as home/(YourUserName)/ in my case I made a folder called ps4jb with both of those folders    
You would also need to Install the dependenies needed by the OO Toolchain
```
sudo apt update
sudo apt install clang
sudo apt install lld
```
If you are on a arch based distro use:  
```
sudo pacman -Sy clang
sudo pacman -Sy lid
```
So after you have that installed you would need to actually expose both the toolchain and the goldhen SDK, simply run nano ~/.bashrc, dont be scared its just a simple text editor, use your arrow keys to go to the botton of the file and you would need to add these lines
```
# Change those based on where you put both toolchain and sdk in
export OO_PS4_TOOLCHAIN=~/ps4jb/PS4Toolchain
export GOLDHEN_SDK=~/ps4jb/GoldHEN_Plugins_SDK
```
press Ctrl + X then press Y and then enter. run source ~./bashrc to refresh it      
Now the fun part comes, head into plugin_src/Faith/Source and edit the main.cpp, edit the `RedirectURL` to what ever server you want the requests to go to as an example I choose `http://192.168.14:3551`.    
After thats done you can finally compile! open your command prompt and type in `make` in the Faith Directory where the Makefile Is located at, After a few seconds it should be done building, all build output is stored in bin/plugins/prx_final/ in the root
of the folder    
Okay so after you successfully build your own Faith.prx you can simply load it up with goldhen!, simply open up your FTP client of your choice and select your PS4, then Head to /data/GoldHEN/plugins/ and just drag in the Faith.prx  
after thats done you would need to edit your Plugins.ini, if the plugins folder and the Plugins.ini is missing from the goldhen folder, simply just create them and the content of the Plugins.ini should look like this  

```
[default]
/data/GoldHEN/plugins/game_patch.prx
/data/GoldHEN/plugins/no_share_watermark.prx

[CUSA07669] <-- Change the CUSA Id depending on what Region version of fortnite you have
/data/GoldHEN/plugins/FaithDebug.prx=true
```

and now you are done! simply load up fortnite and it should automatically redirect all traffic to your own backend


# License Disclosure
Portions of the materials used are trademarks and/or copyrighted works of Epic Games, Inc. All rights reserved by Epic. This material is not official and is not endorsed by Epic.