##taboo

taboo is an HTTP/WebSocket based prefix predict server with user-customizeable property matching feature.

###requirements:
all requrements are shiped as `git submodule`.
+ [RapidJSON](https://github.com/wumch/rapidjson.git)
+ [websocketpp](https://github.com/wumch/websocketpp.git)
+ [stage](https://github.com/wumch/stage.git)
+ cmake >= 2.8

###installation:
```bash
git clone git@github.com:wumch/taboo.git taboo
mkdir taboo taboo/build
cd taboo/build
ln -s ../etc etc
cmake ..
make
./taboo --help    # show configure options
./taboo			  # start taboo daemon
../tests/samples.sh		# run sample tests
../tests/bentchmark.sh  # run bentchmark tests
```

###todo lists:
+ funnelId bug
+ detach implementation
+ store/restore
+ runtime reload