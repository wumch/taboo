#coding:utf-8

from taboo.client import Client


class Test(object):

    host = '127.0.0.1'
    port = 1079
    key = 'your-key'
    secret = 'your-secret'

    def __init__(self, dataFile):
        self.client = Client(host=self.__class__.host, port=self.__class__.port,
             key=self.__class__.key, secret=self.__class__.secret)
        self.dataFile = dataFile

    def run(self):
        n  = 0
        for line in open(self.dataFile):
            self._processLine(line)
            n += 1
            if n > 100000:
                break

    def _processLine(self, line):
        info = line.split('\t')
        item = {
            'id': int(info[1]),
            'we_account_id': info[0],
            'name': info[2],
        }
        if len(info) == 6:
            item['avatar'] = info[5]
        prefixes = filter(None, info[2:5])
        return self.client.attach(prefixes=prefixes, item=item)


if __name__ == '__main__':
    t = Test(dataFile='/data/code/test/R/data/taboo.txt')
    t.run()
