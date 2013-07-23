#!/usr/bin/python

import urllib2
from bs4 import BeautifulSoup
import getopt, sys

class Downloader(object):

    def __init__(self):
        pass

    def open_url(self, url, username=None, passwd=None):
        authinfo = urllib2.HTTPPasswordMgrWithDefaultRealm()
        authinfo.add_password(None, url, username, passwd)
        handler = urllib2.HTTPBasicAuthHandler(authinfo)
        myopener = urllib2.build_opener(handler)
        opened = urllib2.install_opener(myopener)
        output = urllib2.urlopen(url)
        return output


    def extract_urls(self, url, username=None, passwd=None):
        u = self.open_url(url, username, passwd)
        source = u.read()
        soup = BeautifulSoup(source)
        if not soup:
            return

        urls = []
        for link in soup.find_all('a'):
            href=link.get('href')
            if href.startswith("http://") or href.startswith("https://"):
                urls.append([href])
            else:
                urls.append(url+"/"+href)

        return urls

    def download(self, url, namemod=None, username=None, passwd=None ):
        print "Downloading %s: " % url
        file_name = url.split('/')[-1]
        if namemod:
            file_name = namemod+"_"+file_name
        u = self.open_url(url, username, passwd)
        f = open(file_name, 'wb')
        meta = u.info()

        file_size_dl = 0
        block_sz = 8192
        while True:
            buffer = u.read(block_sz)
            if not buffer:
                break

            file_size_dl += len(buffer)
            f.write(buffer)

        print "Downloaded %d bytes" % file_size_dl
        f.close()

    def bulkdownload(self, urls, namemod=None, extension=None, username=None, passwd=None ):
        for url in urls:
            if extension and url.split('/')[-1].endswith(extension):
                self.download(url, namemod, username=username, passwd=passwd)

    def scantree(self, url, levels=1, username=None, passwd=None ):
        urls = {}
        for i in range(levels+1):
            urls[i]=[]

        urls[0] = [url]
        for i in range(levels):
            for address in urls[i]:
                urls_dug = self.extract_urls(address, username=username, passwd=passwd)
                urls[i+1].extend(urls_dug)


        return urls

def usage():
     pass

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ha:u:p:d:e:", ["help", "url=","username=","password=","depth=","extension="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)

    url = None
    username = None
    password = None
    depth = 1
    extension = None
    for o, a in opts:
        if o in ("-a", "--url"):
            url = a
        elif o in ("-u", "--username"):
            username = a
        elif o in ("-p", "--password"):
            password = a
        elif o in ("-d", "--depth"):
            depth = int(a.strip())
        elif o in ("-e", "--extension"):
            extension = a
        else:
            assert False, "unhandled option"


    d = Downloader()
    urls = d.scantree(url, levels=depth, username=username, passwd=password)
    urls = list(set(urls[depth]))

    #grooming a bit
    target_urls = {}
    for link in urls:
        if link.endswith(extension):
            tokenized = filter(None,link.split("/"))
            target_urls[link] = tokenized[-2]

    for link in target_urls.keys():
        d.download(link,namemod=target_urls[link],username=username,passwd=password)


if __name__ == "__main__":
    main()
