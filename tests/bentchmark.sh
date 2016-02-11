#!/bin/sh

# curl -vvv -d '{"aaa":"AAA"}' 'http://192.168.139.7:1079/fdffsdf/?fdsfdfad=&fdfdf&fd?fdf=fsdfd'

url='http://192.168.139.7:1079/item/add'
data='sign=6970bd0208c684fe6f4fdd373c9d0382&data={"aaa":"AAA"}'
tmpfile=/tmp/taboo.post.data

echo "${data}" > "${tmpfile}"

ab -c 20 -n 100000 -T 'application/x-www-form-urlencoded' -p "${tmpfile}" "${url}"

curl -v -d "${data}" "${url}"

