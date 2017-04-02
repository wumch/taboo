## taboo

`taboo` is an HTTP/WebSocket based `prefix predict` server with user-customizeable property matching feature.
`taboo` is written in c++03 and makes use of `cedar`(an implementation of double array trie).

### current status:
generally usable if you donot need some features such as `detach`(delete attached `prefixes` and `items` from `trie`).

### requirements:
all requrements are shiped as `git submodule`.
+ boost
+ [RapidJSON](https://github.com/miloyip/rapidjson)
+ [websocketpp](https://github.com/zaphoyd/websocketpp)
+ [stage](https://github.com/wumch/stage)
+ cmake >= 2.8

### installation:
```bash
git clone git@github.com:wumch/taboo.git taboo
cd taboo
git submodule update
mkdir build
cd build
ln -s ../etc etc
cmake ..
make
./taboo --help    # show configure options
./taboo			  # start taboo daemon
../tests/samples.sh		# run sample tests
../tests/bentchmark.sh  # run bentchmark tests
```

### todo lists:
+ `detach` implementation
+ `store/restore`implementation
+ runtime `reload` implementation


_ _ _


contact [wumuch@gmail.com](mailto:wumuch@gmail.com) to submit suggestions or features tickets if you can not wait for issues processing.
