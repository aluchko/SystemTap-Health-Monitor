import pygtk
pygtk.require('2.0')
import gtk
import datetime

class DateTimeChooser(gtk.HBox):
    def __init__ (self, metricGraph):
        super(DateTimeChooser, self).__init__(False, 0)

        currentTime = datetime.datetime.now()
        self.add_field("hour", currentTime.hour, 0, 23)
        self.add_field("minute", currentTime.minute, 0, 59)
        self.add_field("second", currentTime.second, 0, 59)
        self.add_field("day", currentTime.day, 1, 31)
        self.add_field("month", currentTime.month, 1, 31)
        self.add_field("year", currentTime.year, 1970, 2038)

    def add_field(self, name, value, lower, upper):
        
        vbox = gtk.VBox(False, 0)
        self.pack_start(vbox, True, True, 0)

        label = gtk.Label(name)
        vbox.pack_start(label, False, True, 0)

        hbox = gtk.HBox(False, 0)
        adj = gtk.Adjustment(value, lower, upper, 1.0, 5.0, 0.0)
        spinner = gtk.SpinButton(adj, 0, 0)
        spinner.set_wrap(True)
        vbox.pack_start(spinner, False, True, 0)
