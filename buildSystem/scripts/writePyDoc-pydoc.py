import pydoc
import re

def doc(thing):
	object, name = pydoc.resolve(thing, False)
	infoTxt = pydoc.text.document(object, name)
	infoTxt = re.sub('.\b', '', infoTxt)
	return infoTxt

def docHTML(thing):
	object, name = pydoc.resolve(thing, False)
	infoHTML = pydoc.html.page(pydoc.describe(object), pydoc.html.document(object, name))
	return infoHTML

def writeDoc(thing):
	object, name = pydoc.resolve(thing, False)
	file = open("Documentation/python/" + name + ".txt", 'w')
	file.write(doc(thing))
	file.close()
	file = open("Documentation/python/" + name + ".html", 'w')
	file.write(docHTML(thing))
	file.close()

writeDoc(MGE)

print("DONE")
MGE.Engine.get().shutdown()
