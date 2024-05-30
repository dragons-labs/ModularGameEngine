#
# console debug commands
#

# mark / unmark scene object by its scene node name
def markCmd(cmd, name):
	sn = MGE.LoadAndSaveSystem.get().getSceneNode(name)
	if sn:
		vmm = MGE.VisualMarkersManager.get()
		if cmd == "mark":
			vmm.showMarker(sn, None, MGE.VisualMarker.OBBOX | MGE.VisualMarker.FULL_BOX | MGE.VisualMarker.NO_THICKNESS, MGE.OgreUtils.getColorDatablock(Ogre.ColourValue(0.7, 1, 0, 1)), 0)
		elif cmd == "unmark":
			vmm.hideMarker(sn)
	else:
		print("can't find node:", name)
	return True

MGE.GUIConsole.get().addScript("mark", "mark object", "markCmd")
MGE.GUIConsole.get().addScript("unmark", "unmark object", "markCmd")


# put 3D marker at point
def putMarker(x, z, y = 1.0):
	try:
		putMarker.Id += 1
	except AttributeError:
		putMarker.Id = 0

	MGE.LoadAndSaveSystem.get().loadDotSceneXML(
		'<scene><nodes><node name="Marker_' + str(putMarker.Id) + '">' +
			'<position x="' + str(x) + '" y="' + str(y) + '" z="' + str(z) + '"/>' +
			'<item meshFile="Axis.mesh"/><scale x="3" y="3" z="3"/>' +
		'</node></nodes></scene>',
		False
	);

def putMarkerCmd(cmd, argsStr):
	args = argsStr.split()
	if len(args) == 2:
		putMarker(float(args[0]), float(args[1]))
		return True
	elif len(args) == 3:
		putMarker(float(args[0]), float(args[1]), float(args[2]))
		return True
	else:
		print("invalid syntax")
		return False

MGE.GUIConsole.get().addScript("putMarker", "put 3d marker on point: (x z) OR (x z y)", "putMarkerCmd")


# put actor at point
def putActor(actorName, prototypeName, prototypeFile = "Actors.xml", x = 0, z = 0, y = 1.0):
	MGE.ActorFactory.get().createActor(
		MGE.PrototypeFactory.get().getPrototype(prototypeName, prototypeFile, ""),
		actorName,
		Ogre.Vector3(x, y, z)
	)

def putActorCmd(cmd, argsStr):
	args = argsStr.split()
	if len(args) == 5:
		putActor(args[0], args[1], "Actors.xml", float(args[2]), float(args[3]), float(args[4]))
		return True
	elif len(args) == 6:
		putActor(args[0], args[1], args[2], float(args[3]), float(args[4]), float(args[5]))
		return True
	else:
		print("invalid syntax")
		return False

MGE.GUIConsole.get().addScript("putActor", "put actor: actorName prototypeName x y z prototypeFile", "putActorCmd")
	# example: putActor qwe FireTruck_A Actors.xml 20 20 3


# print info on messageBar:
def printOnMsgBarCmd(cmd, argsStr):
	MGE.TextMsgBar.get().addMessage(argsStr, 3, 7)
	return True

MGE.GUIConsole.get().addScript("printOnMsgBar", "print message (3 times) on MessageBar", "printOnMsgBarCmd")


# list all actors to log:
def listAllActorsCmd(cmd, argsStr):
	allActors = MGE.ActorFactory.get().allActors
	for actorName in allActors:
		actor = allActors[actorName]
		print(actorName, actor.getName(), type(actor))
	return True

MGE.GUIConsole.get().addScript("listAllActors", "list all actors to log", "listAllActorsCmd")


#
# sample console script and usage of timers:
#

def timerCallback(timer_id, delay, args):
	try:
		timerCallback.cnt += 1
	except AttributeError:
		timerCallback.cnt = 0

	console = MGE.GUIConsole.get()
	console.addText("timer id is: " + str(timer_id))
	console.addText("timer delay is: " + str(delay))
	console.addText("timer args is: " + str(args))
	
	print("timer counter is: ", timerCallback.cnt)
	print("console: ", console.isVisible())

def scriptTestCmd(cmd, argsStr):
	args = argsStr.split()
	print("Komenda: ", cmd)
	print("Argumenty (Napis): ", argsStr)
	print("Argumenty (Tablica): ", args)
	MGE.GUIConsole.get().addText("Wykonałem się")
	MGE.realtimeTimer.addTimer(2000, "timerCallback", "13", False, False, "qwer")
	return 1

MGE.GUIConsole.get().addScript("scriptTest", "testowy skrypt konsoli", "scriptTestCmd")



#
# input listener test:
#

def keyReleased(key):
	print("keyReleased:", key, " (most of the elements use keyPressed action and block next keyPressed ... so this is almost always work, i.e. in active GUI input text)")
	if key == 25: # P
		if visualPathFinder:
			visualPathFinder.showNextGridPoints(1)
			if MGE.InputSystem.get().isModifierDown(0x0000001):
				visualPathFinder.showNextGridPoints(300)
		else:
			print("no visualPathFinder")
	return False

inputListener = MGE.InputListener("", "keyReleased", "", "", "", "")
MGE.InputSystem.get().registerListener(inputListener, -1, -1, -1, -1, 55, -1)
