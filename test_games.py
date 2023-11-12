#! /bin/python

import os
import random
import subprocess
import sys


def generate_input():
	lst = []
	for i in range(100): #big value because there could be two much wasted values when trying to play on a full column
		lst.append(random.randint(0,6))

	return lst

def input_to_bytes(lst):
	l = b'\n'.join(str(i).encode() for i in lst)
	return l + b'\n'


def play_game(inp):
	proc = subprocess.run(["./p4"],input=input_to_bytes(inp), capture_output=True)
	lines = proc.stdout.decode("utf-8").split('\n')
	if lines[-3] != "FIN DE PARTIE":
		print(f"error on game {inp}")

	return lines[-2]

def generate_games(n,filename):
	f = open(filename,'w')
	for i in range(n):
		inp = generate_input()
		res = play_game(inp)
		f.write(' '.join(str(i) for i in inp))
		f.write('\n')
		f.write(res)
		f.write('\n')

def check_games(filename):
	f = open(filename,'r')
	lines = f.readlines()
	ok = True
	for i in range(0,len(lines),2):
		inp = [int(i) for i in lines[i].split()]
		res = play_game(inp)
		if res != lines[i+1][:-1]: #we remove the '\n'
			ok = False
			print('\033[91m' + "Error" + '\033[0m' + f" on input {lines[i]}Expected {lines[i+1][:-1]}, got {res}\n")

	if ok:
		print('\033[92m' + "SUCCESS" + '\033[0m')


if __name__ == '__main__':
	print("Please make sure game is in interactive mode, with DEPTH=5")
	if len(sys.argv) < 2 or (sys.argv[1] == "gen" and len(sys.argv) != 4):
		print("Usage : ./test_games.py <filename>\n        ./test_games.py gen <n> <filename>")
		exit(1)
	if sys.argv[1] == "gen":
		generate_games(int(sys.argv[2]),sys.argv[3])
	else:
		check_games(sys.argv[1])