#!/usr/bin/env python


import midi
import os
from matplotlib import pyplot as plt
import sys

sys.path.insert(0, '/home/alexeyche/prog/alexeyche-junk/bio')
from util import *

base_notes = ['C', 'Db', 'D', 'Eb', 'E', 'F', 'Gb', 'G', 'Ab', 'A', 'bB', 'B']


file = "/home/alexeyche/Music/midi/Reinhardt,_Django_-_Blues_Minor.mid"
pat = midi.read_midifile(file)

first_octave = 48


class Note(object):
	def __init__(self, name, octave, duration=0):
		self.name = name
		self.octave = octave
		self.duration = duration


	def __repr__(self):
		return str(self)
	def __str__(self):
		return "{}{}({})".format(self.name, self.octave, self.duration)

class Pause(object):
	def __init__(self, duration):
		self.duration = duration

	def __repr__(self):
		return str(self)
	def __str__(self):
		return "Pause({})".format(self.duration)

notes = {}

start = first_octave - len(base_notes)*2
end = first_octave + len(base_notes)*4

for i in xrange(start, end):
	notes[i] = Note(base_notes[i % len(base_notes)], i/ len(base_notes))


on_notes = {}
song = []


tick = 0

for event in pat[1]:
	new_tick = 0

	if hasattr(event, "tick"):
		new_tick = event.tick

	if isinstance(event, midi.NoteOnEvent):
		pitch = event.get_pitch()

		assert pitch not in on_notes, "Duplicate: {}".format(pitch)

		if event.tick > 0:
			song.append(Pause(new_tick))

		on_notes[pitch] = (notes[pitch], tick + new_tick)
	elif isinstance(event, midi.NoteOffEvent):
		pitch = event.get_pitch()

		assert pitch in on_notes, "Not found: {}".format(pitch)

		note, start_tick = on_notes[pitch]

		note.duration = tick + new_tick - start_tick
 		assert note.duration > 0, "{}".format(event)

		del on_notes[pitch]

		song.append(note)

	tick += new_tick
