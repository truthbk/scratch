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

class Alert(object):
    def __init__(self, threshold, duration):
        self.threshold = threshold
        self.duration = duration
        self.triggered = False
        self.ts = None

    def trigger(self):
        self.triggered = True
        self.ts = int(time.time())

    def reset(self):
        self.triggered = False
        self.ts = None


class Counter(object):
    def __init__(self, sz):
        self.size = sz
        self.alltime = 0
        self.counts = [0] * sz
        self.last_ts = int(time.time())
        self.alerts = []

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
        self.alltime = self.alltime + 1 # this will overflow sooner or later.

    def sum(self, start=0, end=self.size-1):
        res = 0
        for i in xrange(start, end):
            res = res + self.counts[i]

        return res

    def sum(self, lapse):
        start_idx = (self.last_ts - lapse) % self.size
        end_idx = self.last_ts

        res = 0
        if end_idx < start_idx:
            res = sum(start_idx, self.size-1)
            res = res + sum(0, end_idx)
        else:
            res = sum(start_idx, end_idx)

        return res

    def add_alert(self, alert):
        self.alerts.append(alert)

    def process_alerts(self):
        for alert in self.alerts:
            if alert.triggered:
                if self.last_ts >= alert.ts + alert.duration:
                    avg = sum(alert.duration) / alert.duration
                    if avg < alert.threshold:
                        alert.reset()
            else:
                avg = sum(alert.duration) / alert.duration
                if avg >= alert.threshold:
                    alert.trigger()
                        alert.reset()


class DatadogMon(object):

    def __init__(self):
        self.sites = {}
        self.leading = None
        self.leadcnt = 0

    def count_hits(self, host):
        cnt = 0
        for site in self.sites[host]:
            cnt = cnt + self.sites[host][site].alltime

        return cnt

    def explore_leading(self):
        if self.leading is None:
            return

        for site in self.sites[self.leading]:
            print '%s : %d hits' % (self, self.sites[leading][site].alltime)

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

        hits = self.count_hits(host)
        if hits > self.leadcnt:
            self.leadcnt = hits
            self.leading = host

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

