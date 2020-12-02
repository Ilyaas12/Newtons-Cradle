#include "ofApp.h"

using namespace YAMPE; using namespace P;


//--------------------------------------------------------------
void ofApp::setup() {

    easyCam.setDistance(RANGE);

    // instantiate the ground
    ground.set(RANGE, RANGE);
    ground.rotate(90, 1,0,0);

    // lift camera to 'eye' level
    cameraHeightRatio = 0.2f;
    float d = easyCam.getDistance();
    easyCam.setPosition(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio));
    easyCam.setTarget(ofVec3f::zero());

    // specify items on scene
    isGridVisible = false;
    isAxisVisible = true;
    isGroundVisible = true;

    // create a minimal gui
    gui = new ofxUICanvas();

    gui->addWidgetDown(new ofxUISlider("Ball Count", 1.0f, 15.0f, &ballCount, 150, 25));
    gui->addSpacer();
    gui->addWidgetDown(new ofxUISlider("Perturbed Ball Count", 0.f, 15.f, &perturbedBallCount, 150, 25));
    gui->addSpacer();
    gui->addWidgetDown(new ofxUISlider("Initial Angle", 0.0f, 90.f, &initAngle, 150, 25));
    gui->addSpacer();

    gui->addLabelButton("Reset", false);
    gui->addLabelButton("Quit", false);
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);
    gui->loadSettings("GUI/guiSettings.xml");

    isForceVisible = false;

    reset();

}

//--------------------------------------------------------------
void ofApp::update() {

    float dt = 1.0f / ofGetFrameRate();

    // apply forces on particles
    forceGenerators.applyForce(dt);

    // update all particles
    foreach (p, particles) (*p)->integrate(dt);
    // and anchors
    foreach (a, anchors) (*a)->generate(contacts);


    groundContactGenerator.generate(contacts);
    ppContactGenerator.generate(contacts);
    contacts->resolve(dt);
    contacts->clear();
}

//--------------------------------------------------------------
void ofApp::draw() {

    ofEnableDepthTest();
    ofBackgroundGradient(ofColor(128), ofColor(0), OF_GRADIENT_BAR);

    ofPushStyle();
    easyCam.begin();

    if (isGridVisible) {
        ofDrawGrid(10.0f, 5.0f, false, true, true, true);
    } else
        ofDrawGrid(10.0f, 10.0f, false, false, true, false);

    if (isAxisVisible) ofDrawAxis(1);
    if (isGroundVisible) {
        ofPushStyle();
        ofSetHexColor(0xB87333);
        ground.draw();
        ofPopStyle();
    }

    // render all particles
    int anchorIndex = 0;
    foreach (p, particles) {
        (**p).draw();
        // draw anchor from p to anchor's position
        ofLine((**p).position, anchorPositions[anchorIndex++] );
    }
    easyCam.end();

    ofPopStyle();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

    float d = easyCam.getDistance();
    switch (key) {
        case 'h':   // hide/show gui
            gui->toggleVisible();
            break;
        case 'a':
            isAxisVisible = !isAxisVisible;
            break;
        case 'd':
            isGridVisible = !isGridVisible;
            break;
        case 'g':
            isGroundVisible = !isGroundVisible;
            break;
        case 'l':   // set the ground to be level
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'z':
            easyCam.setPosition(0, cameraHeightRatio*d,
                                d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio));
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'Z':
            easyCam.setPosition(0, 0, d);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'x':
            easyCam.setPosition(d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio), cameraHeightRatio*d, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'X':
            easyCam.setPosition(d, 0, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'Y':
            easyCam.setPosition(0.001, d, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'f':
            isFullScreen = !isFullScreen;
            if (isFullScreen) {
                ofSetFullscreen(false);
            } else {
                ofSetFullscreen(true);
            }
            break;

            // simulation specific keys
        case 'p':
            isForceVisible = !isForceVisible;
            foreach (p, particles) (**p).isForceVisible = isForceVisible;
            break;
        case 'r':
            reset();
            break;
    }

}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

    if (x< gui->getRect()->getWidth() && y < gui->getRect()->getHeight()) return;

    easyCam.enableMouseInput();
}


//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) { }

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y ) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}

//--------------------------------------------------------------
void ofApp::exit() {
    gui->saveSettings("GUI/guiSettings.xml");
    delete gui;
}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e) {

    easyCam.disableMouseInput();

    string name = e.widget->getName();
    int kind = e.widget->getKind();
    //cout << "got event from: " << name << " and kind is " <<endl;

    if (name=="Reset") {
        reset();
    } else if (name=="Quit") {
        ofExit();
    }
    else if (name == "Ball Count") {
        int val = ((ofxUISlider*) e.widget)->getValue();
        ((ofxUISlider*) e.widget)->setValue( val );
    }
    else if (name == "Perturbed Ball Count") {
        if (((ofxUISlider*) e.widget)->getValue() > ballCount)
            ((ofxUISlider*) e.widget)->setValue( (int) ballCount);
    }

    reset();
}

//--------------------------------------------------------------
void ofApp::reset() {

    particles.clear();
    forceGenerators.clear();
    ppContactGenerator.particles.clear();
    groundContactGenerator.particles.clear();
    anchors.clear();
    anchorPositions.clear();

    ForceGenerator::Ref gravity(new GravityForceGenerator(ofVec3f(0, -1, 0)));
    contacts = ContactRegistry::Ref(new ContactRegistry(100, "All contacts"));

    for (int k=0; k<(int)ballCount; ++k) {
        Particle::Ref p = Particle::Ref(new Particle());
        p->setLabel(MAKE_STRING("Ball " <<k));

        float r = 0.5f;
        p->radius = r;

        float xOffset = (ballCount/2)*r*2;
        p->setPosition(ofVec3f( (-xOffset)+(k*2*r + r) , 1.f, 0.f));
        ofVec3f anchorPos(p->position.x, p->position.y+5.0f, p->position.z);

        if (k<(int)perturbedBallCount) {
            p->bodyColor = ofColor (255, 0, 0);
            float length = anchorPos.distance(p->position);
            float x = anchorPositions[k].x + (length * -sin(initAngle * (PI/180)));
            float y = anchorPositions[k].y + (length * -cos(initAngle * (PI/180)));
            p->position = ofVec3f( x, y, 0.f);
        }
        else {
            p->bodyColor = ofColor (0, 255, 255);
        }
        p->wireColor = p->bodyColor;

        forceGenerators.add(p, gravity);

        EqualityAnchoredConstraint::Ref anchor = EqualityAnchoredConstraint::Ref(new EqualityAnchoredConstraint(p, anchorPos, 5.0f, 0.0f, "EqualityAnchoredConstraint"+k));
        anchors.push_back(anchor);
        anchorPositions.push_back(anchorPos);

        particles.push_back(p);

        groundContactGenerator.particles.push_back(p);
        ppContactGenerator.particles.push_back(p);

    }
}
