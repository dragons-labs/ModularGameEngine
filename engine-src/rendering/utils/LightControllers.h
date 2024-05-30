/*
Copyright (c) 2019-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma   once

#include "data/utils/OgreUtils.h"

#include <OgreController.h>
#include <OgreLight.h>
#include <OgreBillboard.h>

namespace MGE {

/**
 * @brief base class for light controllers
 */
class LightControllerValue : public Ogre::ControllerValue<Ogre::Real> {
public:
	/// enable light
	virtual void on()  = 0;
	
	/// disable light
	virtual void off() = 0;
	
protected:
	/// return controller value, unused in this case
	virtual Ogre::Real getValue() const override {
		return 0;
	}
};

/**
 * @brief class for creating controllers for rotating light
 */
class RotationLightControllerValue : public MGE::LightControllerValue {
	Ogre::Light*   light;
	Ogre::Real     lightPowerScale;
	Ogre::Vector3  initDir;
	
public:
	/**
	 * @brief constructor
	 * 
	 * @param l  pointer to controlled light object
	 */
	RotationLightControllerValue(Ogre::Light* l) : light(l), lightPowerScale(l->getPowerScale()), initDir( l->getDirection() ) {}
	
	/// receive value from ControllerFunction and do action (rotate light)
	virtual void setValue (Ogre::Real val) override {
		auto rotated = MGE::OgreUtils::rotateVector2(
			Ogre::Vector2(initDir.x, initDir.z),
			Ogre::Radian(val * Ogre::Math::TWO_PI)
		);
		Ogre::Vector3 newDir(rotated.x, initDir.y, rotated.y);
		light->setDirection(newDir);
	}
	
	/// @copydoc LightControllerValue::on
	virtual void on() override {
		light->setPowerScale( lightPowerScale );
	}
	
	/// @copydoc LightControllerValue::off
	virtual void  off() override {
		light->setPowerScale( 0.0 );
	}
};

/**
 * @brief class for creating controllers for flashing light
 */
class FlashingLightControllerValue : public MGE::LightControllerValue {
	Ogre::Light*         light;
	Ogre::Real           lightPowerScale;
	Ogre::v1::Billboard* billboard;
	Ogre::ColourValue    billboardColour;
	Ogre::Real           limitOn;
	Ogre::Real           limitOff;
	bool                 isOn;
	
public:
	/**
	 * @brief constructor
	 * 
	 * @param l      pointer to controlled light object (can be NULL)
	 * @param lp     light power scale for "on" state
	 * @param b      pointer to controlled SFX billboard object for light flare (can be NULL)
	 * @param bc     billboard flare colour value for "on" state
	 * @param onVal  switching "on" value (controller value are in range [0, 1], when controller value is greater then @a onVal light will be on, when less light will be off)
	 * @param offVal switching "off" value (when controller value is greater then @a onVal an @a offVal light will be off)
	 */
	FlashingLightControllerValue(
		Ogre::Light* l, Ogre::Real lp, Ogre::v1::Billboard* b, const Ogre::ColourValue& bc, Ogre::Real onVal = 0.8, Ogre::Real offVal = 1.0
	) : light(l), lightPowerScale(lp), billboard(b), billboardColour(bc), limitOn(onVal), limitOff(offVal) {}
	
	/// receive value from ControllerFunction and do action (turn on or off light)
	virtual void setValue (Ogre::Real val) override {
		if (isOn && (val < limitOn || val > limitOff)) {
			isOn = false;
			if (light)
				light->setPowerScale( 0.0 );
			if (billboard)
				billboard->setColour( Ogre::ColourValue::Black );
		} else if (!isOn && (val > limitOn && val < limitOff)) {
			isOn = true;
			if (light)
				light->setPowerScale( lightPowerScale );
			if (billboard)
				billboard->setColour( billboardColour );
		}
	}
	
	/// @copydoc LightControllerValue::on
	virtual void on() override {
		// will be enabled in setValue()
	}
	
	/// @copydoc LightControllerValue::off
	virtual void off() override {
		isOn = false;
		if (light)
			light->setPowerScale( 0.0 );
		if (billboard)
			billboard->setColour( Ogre::ColourValue::Black );
	}
};


/**
 * @brief class for creating ControllerFunction for flashing light.
 *        ControllerFunction calculate value passed to ControllerValue class.
 *        This class using random function to generate rising values from range [random, 1.0],
 *        after reach 1.0 got o begin with new random value.
 */
class RandomScaleControllerFunction : public Ogre::ControllerFunction<Ogre::Real> {
protected:
	Ogre::Real mScale;
	Ogre::Real mRandomMax;
public:
	/**
	 * @brief constructor
	 * 
	 * @param factor     scale factor for incoming values (~ speed of this function output)
	 * @param randomMax  maximum value of random value used as minimal output vale.
	 */
	RandomScaleControllerFunction(Ogre::Real factor, Ogre::Real randomMax) :
		ControllerFunction<Ogre::Real>(true), mScale(factor), mRandomMax(randomMax)
	{}
	
	/// calculate value to pass to ControllerValue::setValue
	virtual Ogre::Real calculate(Ogre::Real source) override {
		mDeltaCount += source * mScale;
		
		if (mDeltaCount > 1.0 || mDeltaCount < 0) {
			mDeltaCount = Ogre::Math::RangeRandom(0, mRandomMax);
		}
		
		return mDeltaCount;
	}
};


/**
 * @brief class for creating ControllerFunction for flashing light.
 *        ControllerFunction calculate value passed to ControllerValue class.
 *        This class using random function to generate rising values from range [0, 1.0] or [maxVal, 1.0],
 *        after reach 1.0 got o begin with new range.
 */
class RandomThresholdScaleControllerFunction : public Ogre::ControllerFunction<Ogre::Real> {
protected:
	Ogre::Real mScale;
	Ogre::Real mRandomLimit;
	Ogre::Real mMaxVal;
	int        mMaxFollowingUseMaxVal;
	int        mCountFollowingUseMaxVal;
public:
	/**
	 * @brief constructor
	 * 
	 * @param factor                scale factor for incoming values (~ speed of this function output)
	 * @param threshold             threshold for random value [0,1], when random greater then threshold use @a maxVal as init value for output, otherwise use 0
	 * @param maxVal                value used as init for output when random > threshold
	 * @param maxFollowingUseMaxVal maximum number of following one by one using @a maxVal as init for output
	 */
	RandomThresholdScaleControllerFunction(Ogre::Real factor, Ogre::Real threshold, Ogre::Real maxVal, int maxFollowingUseMaxVal = 3) :
		ControllerFunction<Ogre::Real>(true), mScale(factor), mRandomLimit(threshold), mMaxVal(maxVal), mMaxFollowingUseMaxVal(maxFollowingUseMaxVal)
	{}
	
	/// calculate value to pass to ControllerValue::setValue
	virtual Ogre::Real calculate(Ogre::Real source) override {
		mDeltaCount += source * mScale;
		
		if (mDeltaCount > 1.0 || mDeltaCount < 0) {
			Ogre::Real rand = Ogre::Math::RangeRandom(0, 1);
			if (rand > mRandomLimit && mCountFollowingUseMaxVal < mMaxFollowingUseMaxVal) {
				++mCountFollowingUseMaxVal;
				mDeltaCount = mMaxVal;
			} else {
				mDeltaCount = 0;
				mCountFollowingUseMaxVal = 0;
			}
		}
		
		return mDeltaCount;
	}
};

}
