import sys
import getopt

import time
from random import random as rand

from pylsl import StreamInfo, StreamOutlet, local_clock


NAME = "EEG Cap Example"
SAMPLING_RATE = 250
N_CHANNELS = 19
TYPE = 'EEG'


def main():
    
    info = StreamInfo(NAME, TYPE, N_CHANNELS, SAMPLING_RATE, 'float32', 'eegcap1')

    outlet = StreamOutlet(info)

    start_time = local_clock()
    sent_samples = 0
    while True:
        elapsed_time = local_clock() - start_time
        required_samples = int(SAMPLING_RATE * elapsed_time) - sent_samples
        for sample_ix in range(required_samples):
            mysample = [float(200*(rand() - 0.5)) for _ in range(N_CHANNELS)]
            outlet.push_sample(mysample)
        sent_samples += required_samples

        # time.sleep(0.01)


if __name__ == '__main__':
    main()