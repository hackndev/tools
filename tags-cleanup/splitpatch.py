#!/usr/bin/env python
import sys, sre

class Hunk:
    def __init__(self, hunk):
	self.hunk = hunk
	self.lines = self.hunk.split("\n")
	self.linecount = len(self.lines)
	self.insertions = 0
	self.deletions = 0
	self.added = []
	self.removed = []
	self.ID = sre.compile("\$Id:{0,1}.*\$")
	self.Revision = sre.compile("\$Revision:{0,1}.*\$")
	self.Author = sre.compile("\$Author:{0,1}.*\$")
	self.Date = sre.compile("\$Date:{0,1}.*\$")
	self.ok = 0
	for i in range(0, self.linecount):
	    if self.lines[i].startswith("+"):
		self.insertions += 1
		self.added.append(self.lines[i])
	    if self.lines[i].startswith("-"):
		self.deletions += 1
		self.removed.append(self.lines[i])
	self.checkhunk()
    def rm_ident(self, string):
	string = self.Revision.sub("", string)
	string = self.ID.sub("", string)
	string = self.Author.sub("", string)
	string = self.Date.sub("", string)
	return string
    def checkhunk(self):
	if self.insertions != self.deletions:
	    self.ok = 1
	    return self.ok

	for i in range(0, self.insertions):
	    tmp1 = self.rm_ident(self.added[i][1:])
	    tmp2 = self.rm_ident(self.removed[i][1:])
	    if tmp1 != tmp2:
		self.ok = 1
		break
	return self.ok
		

class Patch:
    def __init__(self, patch):
	self.hunks = []
	self.okhunks = []
	self.hunkno = -1
	self.patch = patch
	self.lines = patch.split("\n")
	self.patchhead = ""
	self.ignored = ""
	for i in range(0, len(self.lines)):
	    if self.lines[i].startswith("+++ ") or self.lines[i].startswith("--- ") or self.lines[i].startswith("diff "):
		self.patchhead += self.lines[i] + "\n"
	    elif self.lines[i].startswith("@"):
		self.hunkno += 1
		self.hunks.append(self.lines[i] + "\n")
	    elif self.lines[i].startswith(" ") or self.lines[i].startswith("+") \
		or self.lines[i].startswith("-") or self.lines[i].startswith("\\"):
		self.hunks[self.hunkno] += self.lines[i] + "\n"
	    else:
		if len(self.lines[i]) > 0:
		    self.ignored += "WARNING: |%s|\n" % (self.lines[i])
	for i in range(0, len(self.hunks)):
	    n = Hunk(self.hunks[i])
	    if n.ok:
		self.okhunks.append(n.hunk)
	self.hunks = self.okhunks
	self.okhunks = []

class ParsePatch:
    def __init__(self, patch):
	self.data = ""
	self.head = ""
	self.name = ""
	self.patch = Patch(patch)
	if len(self.patch.hunks) > 0:
	    self.data = self.patch.patchhead + "\n".join(self.patch.hunks)
	self.head = self.patch.patchhead.split("\n")[2].split(" ")[1].split("\t")[0]
	namelist = self.head.split("/")
	self.name = "-".join(namelist[1:2]) + ".diff"
	self.ignored = self.patch.ignored

class PatchCollection:
    def __init__(self):
	self.patches = []

    def append(self, patch):
	self.patches.append(ParsePatch(patch))

    def flushfiles(self):
	data = {}
	for g in self.patches:
	    if data.has_key(g.name):
		data[g.name] += g.data + g.ignored
	    else:
		data[g.name] = g.data + g.ignored
	for f, d in data.items():
	    print "New file " + f
	    fd = open(f, "w")
	    fd.write(d)
	    fd.close()
		
fd = open(sys.argv[1], "r")

lines = fd.read().split("\n")
patches = []
patchno = -1

for i in range(0, len(lines)):
    if lines[i].startswith("diff "):
	patchno += 1
	patches.append(lines[i] + "\n")
    else:
	patches[patchno] += lines[i] + "\n"

pcol = PatchCollection()
for i in range(0, len(patches)):
    pcol.append(patches[i])

pcol.flushfiles()
fd.close()
