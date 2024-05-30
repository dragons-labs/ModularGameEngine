#
# "setControllable" and "unsetControllable" actions for vehicles based on "Car" component
#

def setControllable(actor, action, gameTimeSinceLastFrame):
	MGE.setControlledCar(actor);
	return True;

def unsetControllable(actor, action, gameTimeSinceLastFrame):
	MGE.setControlledCar(None);
	return True;
