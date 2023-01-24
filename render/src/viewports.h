/*
 * viewports.h
 *
 *  Created on: Oct 26, 2016
 *      Author: dan
 */

#ifndef RENDER_SRC_VIEWPORTS_H_
#define RENDER_SRC_VIEWPORTS_H_

#include <OpenSG/OSGViewport.h>
#include "RHandles.h"
#include "RTypes.h"

osg::ViewportPtr createGroundPlaneViewport(render::RHandles& handles);
osg::ViewportPtr createPerspectiveViewport(render::RHandles& handles);
osg::ViewportPtr createOrthoViewport(render::RHandles& handles, render::RenderViewportType type, unsigned int uiVerticalResolution, double dAspectWoverH);





#endif /* RENDER_SRC_VIEWPORTS_H_ */
