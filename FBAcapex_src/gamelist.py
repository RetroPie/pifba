#!/usr/bin/python
# -*- coding: utf-8 -*-
# 
# Takes gamelist.txt (output from the compilation of fba) and generates
# zipname.fba and rominfo.fba for capex for fba2x
#
# Copyright (C) 2007 Juanvi juanvvc@hotmail,com
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import re

gl = open('gamelist.txt','r')
zn = open('zipname.fba','w')
ri = open('rominfo.fba','w')

started = False
for l in gl:
	if l.startswith('+-----'):
		started = not started
		if started:
			print 'Gamelist starts'
		else:
			print 'Gamelist ends'
		continue
	if started:
		fields = l.split('|')
		# removes first field (always empty since line starts with |)
		fields.pop(0)
		# remove leading and trailing spaces in fields
		for i in range(0, len(fields)): fields[i] = fields[i].strip(' \t')
		print 'Data for ' + fields[2].replace(',','')
		# save data if the rom works (no info in second column)
		if not fields[1]:
			zn.write('%s,%s,%s %s\n'%(fields[0], fields[2].replace(',',''), fields[4], fields[5]))
			parent='fba'
			if fields[3]: parent=fields[3]
			ri.write('FILENAME( %s %s %s "%s" )\n'%(fields[0], parent, fields[4], fields[5]))

gl.close()
zn.close()
ri.close()
