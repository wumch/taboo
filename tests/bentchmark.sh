#!/bin/sh

# curl -vvv -d '{"aaa":"AAA"}' 'http://192.168.139.7:1079/fdffsdf/?fdsfdfad=&fdfdf&fd?fdf=fsdfd'

host=127.0.0.1
url="http://${host}:1079/manage/attach"
data='key=fsdfdsf&sign=6970bd0208c684fe6f4fdd373c9d0382&prefixes=["abcdefg","晕倒"]&item={"aaa":"AAA","id":10086}'

tmpfile=/dev/shm/taboo.post.data
echo "${data}" > "${tmpfile}"

ab -c 20 -n 10000 -T 'application/x-www-form-urlencoded' -p "${tmpfile}" "${url}"

echo '\n------------------------------------------------'
curl -v -d "${data}" "${url}"

