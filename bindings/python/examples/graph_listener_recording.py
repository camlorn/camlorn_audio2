#Synthesize binaural beats, play for 5 seconds.
#But also record this using a graph listener.
#Note: this is not production quality.
#If this example is made too long, it will run out of ram; the solution is to write the wave file in a thread.
import libaudioverse
import wave
import time
import queue
import struct

libaudioverse.initialize()
server = libaudioverse.Server()
server.set_output_device()

w1 = libaudioverse.SineNode(server)
w2 = libaudioverse.SineNode(server)
listener = libaudioverse.GraphListenerNode(server, channels = 2)
merger=libaudioverse.ChannelMergerNode(server, channels = 2)
w1.connect(0, merger, 0)
w2.connect(0, merger, 1)
merger.connect(0, listener, 0)

#settings for binaural beats: 300 and 305.
w1.frequency = 300
w2.frequency = 305

#the queue and callback for recording.
audio_queue = queue.Queue()

def callback(obj, frames, channels, buffer):
    #Copying is not optional. The memory belongs to libaudioverse.
    audio_queue.put([buffer[i] for i in range(channels*frames)])

listener.set_listening_callback(callback)

print("beginning synthesis and playing for 5 seconds...")
listener.connect(0, listener.server)

time.sleep(5.0)

#We're done with Libaudioverse, so we can kill it now.
#This has the added effect of making sure all audio is stopped.
libaudioverse.shutdown()

print("Writing audio data to out.wav in the current directory:")
f = wave.open("out.wav", "w")
f.setnchannels(2)
f.setframerate(44100)
f.setsampwidth(2)
try:
    while True:
        frame = audio_queue.get(block = False)
        frame = [int(i*32767) for i in frame] #python wants integer samples, Libaudioverse gives float.
        frame_string = struct.pack(str(len(frame))+"h", *frame)
        f.writeframes(frame_string)
except queue.Empty:
        pass
f.close()

