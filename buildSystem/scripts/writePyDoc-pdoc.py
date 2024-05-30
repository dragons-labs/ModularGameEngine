import pdoc
import re

class Spec:
	name=None
	loader=None
	origin=None

def writeDoc(thing, fakeSpec = True):
	if fakeSpec:
		thing.__spec__ = Spec()
	doc = pdoc.doc.Module(thing)
	file = open("Documentation/python/" + doc.modulename + ".html", 'w')
	file.write(pdoc.render.html_module(module=doc, all_modules={doc.modulename: doc}))
	file.close()

writeDoc(MGE)

print("DONE")
MGE.Engine.get().shutdown()
