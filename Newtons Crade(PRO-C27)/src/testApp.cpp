#include "testApp.h"

using namespace YAMPE; using namespace P;


//--------------------------------------------------------------
void testApp::setup() {

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

    gui->addWidgetDown(new ofxUILabel("My GUI", OFX_UI_FONT_MEDIUM));
    gui->addSpacer();
    gui->addLabelButton("Reset", false);
    gui->addLabelButton("Quit", false);
    gui->autoSizeToFitWidgets();
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
    gui->loadSettings("GUI/guiSettings.xml");

    isForceVisible = false;

    ForceGenerator::Ref gravity(new GravityForceGenerator(ofVec3f(0, -1, 0)));

    contacts = ContactRegistry::Ref(new ContactRegistry(100, "All contacts"));

	// bouncing ball due to gravity and ground collision
	for (int k=0; k<10; ++k) {
        Particle::Ref p = Particle::Ref(new Particle());
		p->setLabel(MAKE_STRING("Ball " <<k));
		p->bodyColor = ofColor (ofRandom(255), ofRandom(255), ofRandom(255));
		p->wireColor = p->bodyColor;
        p->radius = ofRandom(0.1, 0.3);
		forceGenerators.add(p, gravity);
		particles.push_back(p);

		groundContactGenerator.particles.push_back(p);
        ppContactGenerator.particles.push_back(p);

	}

    reset();

}

//--------------------------------------------------------------
void testApp::update() {

    float dt = 1.0f / ofGetFrameRate();

	// apply forces on particles
	forceGenerators.applyForce(dt);

	// update all particles
	foreach (p, particles) (*p)->integrate(dt);

    groundContactGenerator.generate(contacts);
    ppContactGenerator.generate(contacts);
    contacts->resolve(dt);
    contacts->clear();

}

//--------------------------------------------------------------
void testApp::draw() {

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
    foreach (p, particles) (**p).draw();

    easyCam.end();

    ofPopStyle();

}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {

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
void testApp::mousePressed(int x, int y, int button) {

    if (x< gui->getRect()->getWidth() && y < gui->getRect()->getHeight()) return;

    easyCam.enableMouseInput();
}


//--------------------------------------------------------------
void testApp::windowResized(int w, int h) {
//    graphCanvas->setPosition(ofClamp(ofGetWidth()-graphCanvas->getRect()->getWidth()-10,0,ofGetWidth()), 10);
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {}
void testApp::mouseMoved(int x, int y ) {}
void testApp::mouseDragged(int x, int y, int button) {}
void testApp::mouseReleased(int x, int y, int button) {}
void testApp::gotMessage(ofMessage msg) {}
void testApp::dragEvent(ofDragInfo dragInfo) {}

//--------------------------------------------------------------
void testApp::exit() {
    gui->saveSettings("GUI/guiSettings.xml");
    delete gui;
}

//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e) {

    easyCam.disableMouseInput();

	string name = e.widget->getName();
	int kind = e.widget->getKind();
	cout << "got event from: " << name << " and kind is " <<endl;

    //string name = e.widget->getName();
    if (name=="Reset") {
        reset();
    } else if (name=="Quit") {
        exit();
    } else if (name=="Pendulum") {
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        cout <<toggle->getValue() <<endl;

    }
}

//--------------------------------------------------------------
void testApp::reset() {
	foreach (p, particles) {
		(*p)->setPosition(ofVec3f(ofRandom(-10,10), ofRandom(2,20), ofRandom(-10,10)));
	}
}
