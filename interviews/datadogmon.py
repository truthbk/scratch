#!/usr/bin/env python

import time
import urlparse

try:
    import scapy.all as scapy
except ImportError:
    import scapy

try:
    # This import works from the project directory
    import scapy_http.http
except ImportError:
    # If you installed this package via pip, you just need to execute this
    from scapy.layers import http

MON_TIME = 3600

class Counter(object):
    def __init__(self, sz):
        self.size = sz
        self.counts = [0] * sz
        self.last_ts = time.time()

    def zero(self, start, end):
        start_idx = start % self.sz
        end_idx = end % self.sz

        #circle round
        if end_idx < start_idx:
            self.zero(start, sz-1)
            self.zero(0, end)
        else:
            for i in xrange(start_idx, end_idx):
                self.counts[i] = 0


    def inc(self):
        ts = time.time()
        if self.last_ts != ts:
            self.zero(self.last_ts + 1, ts)
            self.last_ts = ts

        self.counts[ts % self.sz] = self.count[ts % self.sz] + 1

    def sum(self):
        res = 0
        for i in xrange(start_idx, end_idx):
            res = res + self.counts[i]

        return res

class DatadogMon(object):

    def __init__(self):
        self.sites = {}

    def monitor_site(self, url):
        spliturl = urlparse.urlsplit(url)
        url_root = spliturl.netloc
        url_subsite = spliturl.path.split('/')

        sitekey = None
        if len(url_subsite) < 2:
            sitekey = '/'
        else:
            sitekey = url_subsite[1]

        if url_root in self.sites:
            if sitekey in self.sites[url_root]:
                self.sites[url_root][site_key].inc()
            else:
                self.sites[url_root][site_key] = Counter(MON_TIME)
                self.sites[url_root][site_key].inc()
        else:
            self.sites[url_root] = {sitekey: Counter(MON_TIME)}
            self.sites[url_root][site_key].inc()

    def process(self, packet):
        url = packet.http.Host + packet.http.Path
        monitor_site(url)


def sniff_cb(packet):
    global datadogmon
    datadogmon.process(packet)

packets = scapy.rdpcap('example_network_traffic.pcap')
for p in packets:
    print '=' * 78
    datadogmon.process(p)


