#!/bin/bash

declare -a urls=(
'http://192.168.11.60:1079/manage/attach?key=username&sign=signature&prefixes=["hejinyu","hjy","何金玉"]&item={"id":10086,"name":"何金玉","we_account_id":100100209}'
'http://192.168.11.60:1079/manage/attach?key=username&sign=signature&prefixes=["hejinyu","hjy","何金鱼"]&item={"id":10087,"name":"何金鱼","we_account_id":100100209}'
'http://192.168.11.60:1079/manage/attach?key=username&sign=signature&prefixes=["hejinyu","hjy","何禁欲"]&item={"id":10088,"name":"何禁欲","we_account_id":200100209}'
'http://192.168.11.60:1079/manage/attach?key=username&sign=signature&prefixes=["hejinyu","hjy","何金隅"]&item={"id":10089,"name":"何金隅","we_account_id":200100209}'
'http://192.168.11.60:1079/manage/attach?key=username&sign=signature&prefixes=["hejiangyou","hjy","何酱油"]&item={"id":10090,"name":"何酱油","we_account_id":100100209}'
'http://192.168.11.60:1079/manage/attach?key=username&sign=signature&prefixes=["dajiangyou","djy","打酱油"]&item={"id":10091,"name":"打酱油","we_account_id":100100209}'
'http://192.168.11.60:1079/query/predict?data={"prefix":"heji","num":2,"filters":{"we_account_id":100100209}}'
'http://192.168.11.60:1079/query/predict?data={"prefix":"hejia","num":2,"filters":{"we_account_id":100100209}}'
'http://192.168.11.60:1079/query/predict?data={"prefix":"hjy","num":2,"filters":{"we_account_id":100100209}}'
'http://192.168.11.60:1079/query/predict?data={"prefix":"何","num":10,"filters":{"we_account_id":100100209}}'
'http://192.168.11.60:1079/manage/get_access_token?tid=17951&filters={"we_account_id":100100209}'
)

for url in ${urls[@]};
do
	echo "${url}"
	curl -g ${url}
	echo
done


