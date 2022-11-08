
from pylsl import StreamInlet, resolve_stream
import sys
from vispy import scene
from vispy import app
from vispy.scene.visuals import Text
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

CHUNK_SIZE = 30
CHANNEL_COUNT = 19

data = np.ones(shape=(CHANNEL_COUNT, CHUNK_SIZE))


canvas = scene.SceneCanvas(keys='interactive')
canvas.size = 600, 600
canvas.show()

grid = canvas.central_widget.add_grid()


def plot_impedance():
    streams = resolve_stream('type', 'EEG')
    inlet = StreamInlet(streams[0])
    impedances, timestamp = inlet.pull_sample()    
    
    ch_locs = pd.read_csv('standard_1020_channel_locations.csv').values
    x = ch_locs[:,1]
    y = ch_locs[:,2]
    
    THRESHOLD = 0.9
    plt.scatter(x, y, c=[(0,1,0,1) if i < THRESHOLD else (1,0,0,1) for i in impedances], s=[100 for i in range(CHANNEL_COUNT)])
    plt.show()


N = 500
lines = []
for i in range(CHANNEL_COUNT):
    vb = grid.add_view(row=i, col=0)
    vb.camera = 'panzoom'
    vb.camera.rect = (0, -5), (100, 10)

    t1 = Text('C' + str(i+1), parent=canvas.scene, color=(1,1,1,0.8))
    t1.font_size = 4
    t1.pos = 5, 0

    vb.add(t1)

    pos = np.empty((N, 2), dtype=np.float32)
    pos[:, 0] = np.linspace(10, 110, N)
    pos[:, 1] = np.ones((N,))
    line = scene.visuals.Line(pos=pos, color=(1, 1, 1, 1), method='gl')
    lines.append(line)

    vb.add(line)


def normalize(x):
    MAX = 100
    MIN = -100
    return float(x - MAX) / float(MAX - MIN)

def get_stream(ev):
    global lines

    streams = resolve_stream('type', 'EEG')
    inlet = StreamInlet(streams[0])

    sample, timestamp = inlet.pull_sample()
    for i in range(CHANNEL_COUNT):
        lines[i].pos[:, 1] = np.append(lines[i].pos[:,1], normalize(sample[i]))[1:]        
        lines[i].set_data(pos=pos, color=(1, 1, 1, 1))



timer = app.Timer()
timer.connect(get_stream)
timer.start(0)



if __name__ == '__main__' and sys.flags.interactive == 0:
    plot_impedance()
    app.run()






