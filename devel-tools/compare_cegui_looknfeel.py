#!/usr/bin/python3

import xml.etree.ElementTree as xml
import re, sys, os, tempfile

class CommentedTreeBuilder(xml.TreeBuilder):
	def comment(self, data):
		self.start(xml.Comment, {})
		self.data(data)
		self.end(xml.Comment)

def getXML(inFile):
	txt = inFile.read()
	txt = re.sub('WindowsLook/*', '', txt)
	txt = re.sub('MGE_GUI[^/"]*/*', '', txt)
	return xml.XML(txt, xml.XMLParser(target=CommentedTreeBuilder()))

def saveXML(xmlTag, name):
	outFile = tempfile.NamedTemporaryFile(mode="w", prefix="  "+name+"  ", dir="/dev/shm")
	outFile.file.write( xml.tostring(xmlTag, encoding="unicode") )
	outFile.file.close()
	return outFile

if len(sys.argv) < 3:
	print("USAGE: " + sys.argv[0] + "  path.to/MGE/looknfeel.file  path.to/CEGUI_WindowsLook/looknfeel.file", file=sys.stderr)
	exit(1)

in1_xml = getXML( open(sys.argv[1]) )
in2_xml = getXML( open(sys.argv[2]) )

for widget in in1_xml:
	if widget.tag != "WidgetLook" or not "name" in widget.attrib:
		continue
	
	widgetName = widget.attrib["name"]
	
	widget2 = None
	for tmp in in2_xml:
		if "name" in tmp.attrib and tmp.attrib["name"] == widgetName:
			widget2 = tmp
			break
	
	if widget2:
		out1 = saveXML(widget, widgetName)
		out2 = saveXML(widget2, widgetName)
		
		print("Compare " + widgetName, file=sys.stderr)
		cmd = 'diff -uwB "' + out1.name + '" "' + out2.name + '"'
		os.system(cmd)
	else:
		print("Skip " + widgetName, file=sys.stderr)

