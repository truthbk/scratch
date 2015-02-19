#!/usr/bin/env python

import time
import urlparse
import sys

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
        self.last_ts = int(time.time())

    def zero(self, start, end):
        start_idx = start % self.size
        end_idx = end % self.size

        #circle round
        if end_idx < start_idx:
            self.zero(start, self.size-1)
            self.zero(0, end)
        else:
            for i in xrange(start_idx, end_idx):
                self.counts[i] = 0


    def inc(self):
        ts = int(time.time())
        if self.last_ts != ts:
            self.zero(self.last_ts + 1, ts)
            self.last_ts = ts

        self.counts[self.last_ts%self.size] = self.counts[self.last_ts%self.size] + 1

    def sum(self):
        res = 0
        for i in xrange(start_idx, end_idx):
            res = res + self.counts[i]

        return res

class DatadogMon(object):

    def __init__(self):
        self.sites = {}

    def monitor_site(self, host, path):
        url_subsite = path.split('/')

        sitekey = None
        if len(url_subsite) < 2:
            sitekey = '/'
        else:
            sitekey = url_subsite[1]

        if host in self.sites:
            if sitekey in self.sites[host]:
                self.sites[host][sitekey].inc()
            else:
                self.sites[host][sitekey] = Counter(MON_TIME)
                self.sites[host][sitekey].inc()
        else:
            self.sites[host] = {sitekey: Counter(MON_TIME)}
            self.sites[host][sitekey].inc()

    def process(self, packet):
        if http.HTTPRequest in packet:
            host = packet[http.HTTPRequest].Host
            path = packet[http.HTTPRequest].Path
            self.monitor_site(host, path)
            print 'packet processed'

datadogmon = DatadogMon()

def sniff_cb(packet):
    global datadogmon
    datadogmon.process(packet)

def main(argv):
    ifc = argv[0]
    scapy.sniff(iface=ifc, filter="port 80", prn=sniff_cb)



if __name__ == '__main__':
    main(sys.argv[1:])


packets = scapy.rdpcap('example_network_traffic.pcap')
for p in packets:
    print '=' * 78
    datadogmon.process(p)


