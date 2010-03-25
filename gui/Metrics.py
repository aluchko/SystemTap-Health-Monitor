#!/usr/bin/env python
import math

class MetricType:
    def __init__(self, id, name, min=None, max=None, default=None):
        self.id = id
        self.name = name
        self.min = min
        self.max = max
        self.default = default
        self.timeout = 2 # default, set by db later

class Metric:
    def __init__(self, id, metricType, name, mean, numSamples, m2):
        self.id = id
        self.metricType = metricType
        self.name = name
        self.mean = mean
        self.numSamples = numSamples
        self.m2 = m2

    def get_std(self):
        # don't worry about the single sample case
        return math.sqrt(self.m2/(self.numSamples -1))

    std = property(get_std)

