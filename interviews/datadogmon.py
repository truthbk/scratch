#!/usr/bin/env python

import time
import datetime
import urlparse
import sys
import threading
import urwid

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

alerts_widget = urwid.Text("Alerts:\n")
updates_widget = urwid.Text("Updates:\n")
frame_widget = urwid.Frame(
    header=updates_widget,
    body=urwid.Filler(alerts_widget, valign='bottom'),
    focus_part='header')

refreshthread = None
sniffthread = None
running = True

def exit_on_q(key):
    global running

    if key == 'q':
        if refreshthread:
            running = False
            refreshthread.join(2)
        if sniffthread:
            running = False
            sniffthread.join(0)
        raise urwid.ExitMainLoop()

loop = urwid.MainLoop(frame_widget, unhandled_input=exit_on_q)

class Alert(object):
    def __init__(self, threshold, duration):
        self.threshold = threshold
        self.duration = duration
        self.triggered = False
        self.ack = False
        self.ts = None

    def trigger(self):
        self.ack = True

    def trigger(self):
        self.triggered = True
        self.ts = int(time.time())

    def reset(self):
        self.triggered = False
        self.ts = None
        self.ack = False


class Counter(object):
    def __init__(self, sz):
        self.size = sz
        self.alltime = 0
        self.counts = [0] * sz
        self.last_ts = int(time.time())
        self.alerts = []

    def zero(self, start, end):
        if (end-start) > self.size:
            start_idx = 0
            end_idx = self.size - 1
        else:
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

    def csum(self, start=0, end=None):
        res = 0
        if end is None:
            end = self.size
        for i in xrange(start, end+1):
            res = res + self.counts[i]

        return res

    def csum_lapse(self, lapse):
        start_idx = (self.last_ts - lapse) % self.size
        end_idx = self.last_ts % self.size

        res = 0
        if end_idx < start_idx:
            res = self.csum(start_idx, self.size - 1)
            res = res + self.csum(0, end_idx)
        else:
            res = self.csum(start_idx, end_idx)

        return res

    def add_alert(self, alert):
        self.alerts.append(alert)

    def process_alerts(self):
        active = []
        global alerts_widget
        for alert in self.alerts:
            ts = int(time.time())
            if self.last_ts < ts:
                # zero some stuff out...
                self.zero(self.last_ts+1, ts)
                self.last_ts = ts

            timestr = datetime.datetime.fromtimestamp(
                    self.last_ts).strftime('%Y-%m-%d %H:%M:%S')

            if alert.triggered:
                if self.last_ts >= alert.ts + alert.duration:
                    avg = self.csum_lapse(alert.duration) / alert.duration
                    if avg < alert.threshold:
                        alert.reset()
                        alerts_widget.set_text(alerts_widget.text +
                                "Traffic restored to normalcy - hits %d, reset at: %s\n" \
                                        % (self.csum_lapse(alert.duration), timestr))
            else:
                avg = self.csum_lapse(alert.duration) / alert.duration
                if avg >= alert.threshold:
                    alert.trigger()
                    alerts_widget.set_text(alerts_widget.text +
                            "High traffic generated an alert - hits %d, triggered at: %s\n" \
                                    % (self.csum_lapse(alert.duration), timestr))

            if alert.triggered:
                active.append(alert)

        return active


class DatadogMon(object):

    def __init__(self, threshold=20):
        self.sites = {}
        self.leading = None
        self.leadcnt = 0
        self.threshold = threshold
        self.total = Counter(MON_TIME)
        self.total.add_alert(Alert(threshold, 120))

        #Global interpreter lock would do?
        self.lock = threading.Lock()

    #call holding lock.
    def count_hits(self, host):
        cnt = 0

        for site in self.sites[host]:
            cnt = cnt + self.sites[host][site].alltime

        return cnt

    def general_stats(self):
        #print 'Total hit volume: %d' % self.total.alltime
        pass

    def explore_leading(self):
        self.lock.acquire()
        try:
            output = ""
            if self.leading is not None:
                output = "Leading load URL: %s\n" % self.leading

                for site in self.sites[self.leading]:
                    output += "----%s/%s : %d hits\n" \
                          % (self.leading, site, self.sites[self.leading][site].alltime)
        finally:
            updates_widget.set_text(output)
            self.lock.release()

    def monitor_site(self, host, path):
        url_subsite = path.split('/')
        url_subsite = filter(None, url_subsite)

        sitekey = None
        if url_subsite and (url_subsite[0] == 'http:' or url_subsite[0] == 'https:'):
            url_subsite = url_subsite[1:]

        if len(url_subsite) < 2:
            sitekey = '/' #root
        else:
            sitekey = url_subsite[1] #sub-url

        self.lock.acquire()
        try:
            if host in self.sites:
                if sitekey in self.sites[host]:
                    self.sites[host][sitekey].inc()
                else:
                    self.sites[host][sitekey] = Counter(MON_TIME)
                    self.sites[host][sitekey].inc()
            else:
                self.sites[host] = {sitekey: Counter(MON_TIME)}
                self.sites[host][sitekey].inc()

            self.total.inc()

            hits = self.count_hits(host)
            if hits > self.leadcnt:
                self.leadcnt = hits
                self.leading = host

            self.total.process_alerts()
        finally:
            self.lock.release()


    def check_loaders(self):
        # do our thing = these should be fine.
        self.general_stats()
        self.explore_leading()
        self.total.process_alerts()
        threading.Timer(10.0, self.check_loaders).start()

    def process(self, packet):
        if http.HTTPRequest in packet:
            host = packet[http.HTTPRequest].Host
            path = packet[http.HTTPRequest].Path
            self.monitor_site(host, path)


datadogmon = DatadogMon(1)

def sniff_cb(packet):
    global datadogmon
    datadogmon.process(packet)

def sniffhttp(ifc):
    global datadogmon
    scapy.sniff(iface=ifc, filter="port 80", prn=lambda x : datadogmon.process(x))

def threaded_sniff(iface):
    sniffer = threading.Thread(target = sniffhttp, args = (iface,))
    sniffer.start()
    return sniffer;

def threaded_refresh():
    refresher = threading.Thread(target = refresh)
    refresher.start()
    return refresher;

def refresh():
    global loop
    global running
    while running:
        time.sleep(1)
        loop.draw_screen()

def main(argv):
    ifc = argv[0]

    global datadogmon
    global refreshthread
    global sniffthread
    datadogmon.check_loaders()
    sniffthread = threaded_sniff(ifc)
    refreshthread = threaded_refresh()
    loop.run()

if __name__ == '__main__':
    main(sys.argv[1:])

