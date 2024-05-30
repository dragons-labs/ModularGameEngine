MGE.GUIConsole.get().addScript("resetCamera", "reset camera orientation and position", "resetCamera")
def resetCamera(c, a):
	MGE.getCurrentCamera().setOrientation(Quaternion(1, 0, 0, 0))
	MGE.getCurrentCamera().setPosition(Vector3(0, 0, 0))
	return True;

# makeOwnerOfCamera action callback script
def makeOwnerOfCamera(actor, action, gameTimeSinceLastFrame):
	w3do = MGE.World3DObject.getFromActor(actor) # actor.getComponent(1)
	MGE.getCurrentCamera().setOwner(  w3do.getOgreSceneNode(), True, True  )
	return True;
