#pragma once

#include "ofMain.h"
#include "ofxUI.h"

#include "YAMPE/Particle/ForceGeneratorRegistry.h"
#include "YAMPE/Particle/ForceGeneratorRegistry.h"
#include "YAMPE/Particle/ContactRegistry.h"
#include "YAMPE/Particle/ContactGenerators.h"
#include "YAMPE/Particle/Constraints.h"

#include <math.h>

class ofApp : public ofBaseApp {

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // simple 3D world with ground and axes
    const float RANGE = 20;
    ofEasyCam easyCam;
    bool isAxisVisible = true;
    bool isGridVisible = true;
    bool isGroundVisible = true;
    bool isFullScreen = false;
    float cameraHeightRatio;
    ofPlanePrimitive ground;

    // ofxUI code
    void guiEvent(ofxUIEventArgs &e);
    void exit();
    ofxUICanvas *gui;

    // simulation specific stuff goes here
    void reset();

    float ballCount;
    float perturbedBallCount;
    float initAngle;

private:

    YAMPE::ParticleRegistry particles;
    std::vector<YAMPE::P::EqualityAnchoredConstraint::Ref> anchors;
    std::vector<ofPoint> anchorPositions;
    YAMPE::P::ForceGeneratorRegistry forceGenerators;
    YAMPE::P::ContactRegistry::Ref contacts;
    YAMPE::P::GroundContactGenerator groundContactGenerator;
    YAMPE::P::ParticleParticleContactGenerator ppContactGenerator;

    bool isForceVisible;
};

