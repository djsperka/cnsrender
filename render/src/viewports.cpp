/*
 * viewports.cpp
 *
 *  Created on: Oct 26, 2016
 *      Author: dan
 */

#include "viewports.h"
#include <OpenSG/OSGPassiveBackground.h>
#include <OpenSG/OSGPerspectiveCamera.h>
#include <OpenSG/OSGOrthographicCamera.h>
#include "CameraBeaconFixture.h"
#include "RFixture.h"
#include "CameraBeaconFixture.h"
#include "DummyFixture.h"

OSG_USING_NAMESPACE
using namespace render;

ViewportPtr createGroundPlaneViewport(RHandles& handles)
{
	RFixtureP rootFixtureP;
	RFixtureP cameraBeaconP;
	CameraBeaconStruct beaconSpec;
	PerspectiveCameraPtr cameraPtr;
	ViewportPtr vPtr;

	// prepare default beacon settings and create/update beacon fixture

	beaconSpec.ex = 0;
	beaconSpec.ey = 0;
	beaconSpec.ez = 0;
	beaconSpec.dx = 0;
	beaconSpec.dy = 0;
	beaconSpec.dz = -1;
	beaconSpec.ux = 0;
	beaconSpec.uy = 1;
	beaconSpec.uz = 0;

	cameraBeaconP = CameraBeaconFixture::create();
	update_fixture(cameraBeaconP, &beaconSpec);

	// Create root fixture
	rootFixtureP = DummyFixture::create();

	// Now attach beacon to perspective root
	cameraBeaconP->attachToScenegraph(*rootFixtureP);

	// Add these fixtures to handles.
	handles.addSystemFixtures(rootFixtureP, cameraBeaconP, RenderViewportType::PerspectiveGPFull);

	// create perspective camera with hard coded defaults
	cameraPtr = PerspectiveCamera::create();
	beginEditCP(cameraPtr);
		cameraPtr->setBeacon(cameraBeaconP->getHead());
		cameraPtr->setFov(deg2rad(60));
		cameraPtr->setNear(1);
		cameraPtr->setFar(1000);
		cameraPtr->setAspect(1.0);
	endEditCP(cameraPtr);

	// Create viewport, set camera, root, and background.
	// Create the ground plane viewport.
	vPtr = Viewport::create();
	beginEditCP(vPtr);
		vPtr->setSize(0, 0, 1, 1);
		vPtr->setRoot(rootFixtureP->getHead());
		vPtr->setCamera(cameraPtr);
		vPtr->setBackground(PassiveBackground::create());
		addRefCP(vPtr);
	endEditCP(vPtr);

	return vPtr;
}

ViewportPtr createPerspectiveViewport(RHandles& handles)
{
	RFixtureP rootFixtureP;
	RFixtureP cameraBeaconP;
	CameraBeaconStruct beaconSpec;
	PerspectiveCameraPtr cameraPtr;
	ViewportPtr vPtr;

	// prepare default beacon settings and create/update beacon fixture

	beaconSpec.ex = 0;
	beaconSpec.ey = 0;
	beaconSpec.ez = 0;
	beaconSpec.dx = 0;
	beaconSpec.dy = 0;
	beaconSpec.dz = -1;
	beaconSpec.ux = 0;
	beaconSpec.uy = 1;
	beaconSpec.uz = 0;

	cameraBeaconP = CameraBeaconFixture::create();
	update_fixture(cameraBeaconP, &beaconSpec);

	// Create root fixture
	rootFixtureP = DummyFixture::create();

	// Now attach beacon to perspective root
	cameraBeaconP->attachToScenegraph(*rootFixtureP);

	// Add these fixtures to handles.
	handles.addSystemFixtures(rootFixtureP, cameraBeaconP, RenderViewportType::PerspectiveFull);

	// create perspective camera with hard coded defaults
	cameraPtr = PerspectiveCamera::create();
	beginEditCP(cameraPtr);
		cameraPtr->setBeacon(cameraBeaconP->getHead());
		cameraPtr->setFov(deg2rad(60));
		cameraPtr->setNear(1);
		cameraPtr->setFar(1000);
		cameraPtr->setAspect(1.0);
	endEditCP(cameraPtr);

	// Create viewport, set camera, root, and background.
	// Create the ground plane viewport.
	vPtr = Viewport::create();
	beginEditCP(vPtr);
		vPtr->setSize(0, 0, 1, 1);
		vPtr->setRoot(rootFixtureP->getHead());
		vPtr->setCamera(cameraPtr);
		vPtr->setBackground(PassiveBackground::create());
		addRefCP(vPtr);
	endEditCP(vPtr);

	return vPtr;
}


ViewportPtr createOrthoViewport(RHandles& handles, RenderViewportType vType, unsigned int uiVerticalResolution, double dAspectWoverH)
{
	RFixtureP rootFixtureP;
	RFixtureP cameraBeaconP;
	CameraBeaconStruct beaconSpec;
	OrthographicCameraPtr cameraPtr;
	ViewportPtr vPtr;
	bool bFail = false;

	// prepare default beacon settings and create/update beacon fixture

	beaconSpec.ex = 0;
	beaconSpec.ey = 0;
	beaconSpec.ez = 0;
	beaconSpec.dx = 0;
	beaconSpec.dy = 0;
	beaconSpec.dz = -1;
	beaconSpec.ux = 0;
	beaconSpec.uy = 1;
	beaconSpec.uz = 0;

	cameraBeaconP = CameraBeaconFixture::create();
	update_fixture(cameraBeaconP, &beaconSpec);

	// Create root fixture
	rootFixtureP = DummyFixture::create();

	// Now attach beacon to perspective root
	cameraBeaconP->attachToScenegraph(*rootFixtureP);

	// Add these fixtures to handles.
	handles.addSystemFixtures(rootFixtureP, cameraBeaconP, vType);

	// create perspective camera with hard coded defaults
	cameraPtr = OrthographicCamera::create();
	beginEditCP(cameraPtr);
		cameraPtr->setBeacon(cameraBeaconP->getHead());
		cameraPtr->setNear((float)0.01);
		cameraPtr->setFar(1000.0);
		cameraPtr->setVerticalSize((float)uiVerticalResolution);
		cameraPtr->setAspect((float)dAspectWoverH);
	endEditCP(cameraPtr);

	// Create viewport, set camera, root, and background.
	// The size/location of the viewport depends on the ViewportType
	vPtr = Viewport::create();
	beginEditCP(vPtr);
		switch (vType)
		{
		case RenderViewportType::OrthoFull:
			vPtr->setSize(0, 0, 1, 1);
			break;
		case RenderViewportType::OrthoLeft:
			vPtr->setSize(0, 0, .5, 1);
			break;
		case RenderViewportType::OrthoRight:
			vPtr->setSize(.5, 0, 1, 1);
			break;
		default:
			bFail = true;
			break;
		}
		vPtr->setRoot(rootFixtureP->getHead());
		vPtr->setCamera(cameraPtr);
		vPtr->setBackground(PassiveBackground::create());
		addRefCP(vPtr);
	endEditCP(vPtr);

	// I wait until exit the editCP{} block above. Seems like a good idea,
	// not sure if its necessary to avoid leaving something hanging, but
	// application will likely exit with this exception anyways.
	if (bFail)
		throw std::invalid_argument("RenderViewportType must be OrthoFull|Left|Right");

	return vPtr;
}



