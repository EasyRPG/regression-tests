import sys

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} [FILENAME]")
    print("Extracts all saves out of a concatenated regression test save")
    sys.exit(1)

chunks = list(range(0x64, 0x72+1))

save_data = []
cur_save_data = bytearray(b"")

def read(stream, count):
	b = stream.read(count)
	cur_save_data.extend(b)
	return b

def read_ber(stream):
	value = 0;
	temp = 0x80;
	while temp & 0x80:
		value <<= 7
		temp = int.from_bytes(read(stream, 1), byteorder='little')
		value |= (temp & 0x7F)
	return value

def read_save(stream):
	saves = len(save_data) + 1

	if read(f, 11).decode("ascii") != "LcfSaveData":
		raise ValueError(f"Save {saves} corrupted: Expected LcfSaveData")

	while (True):
		chunk = read_ber(f)
		if chunk not in chunks:
			if chunk == 0xB:
				# Start of new save
				del cur_save_data[-1]
				return True
			elif chunk == 200:
				# Likely not what you want for a compare
				print(f"Warning: Save {saves} contains EasyRPG chunk")
			elif chunk == 0:
				if not stream.read(1):
					# EOF
					return False
			raise ValueError(f"Save {saves} corrupted: Bad chunk {chunk}")
		chunk_size = read_ber(f)
		read(f, chunk_size)

with open(sys.argv[1], "rb") as f:
	if read(f, 1) != b"\x0b":
		raise ValueError(f"Not a save file")
	while (read_save(f)):
		save_data.append(cur_save_data)
		cur_save_data = bytearray(b"\x0b")
	save_data.append(cur_save_data)

print(f"Found {len(save_data)} savegames")

# Example: Extract them
for i, save in enumerate(save_data):
	with open(f"Save{i+1:04d}.lsd", "wb") as f:
		f.write(save)

