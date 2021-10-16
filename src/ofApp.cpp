#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofBackground( 10, 10, 10);
    ofDisableArbTex();
    height = 80;
    width = 80;
    perlinRange =1.0;
    perlinHeight = 5.0;
	camera.setPosition(ofVec3f(0, -4.f, -10.f));
	camera.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, -1, 0));
    camera.setDistance( 50 );
    camera.setNearClip( 0.1 );
    camera.setFarClip( 300 );
	
	world.setup();
	world.setCamera(&camera);
    
//    ofEnableSmoothing();
//    ofEnableAntiAliasing();
    
    anisotropy = 4.;
    
    float fboDiv = 4.f;
//    ofSetMinMagFilters( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR );
    fbo.allocate( (float)ofGetWidth() / fboDiv, (float)ofGetWidth() / fboDiv, GL_RGB, 4 );
    fbo.begin(); {
        ofClear(0, 0, 0, 255 );
        ofSetColor( 255,90,121, 255);
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight() );
        ofSetColor(0, 0, 205, 255);
        
        int numIterations = 4;
        float inc = (float)fbo.getWidth() / ((float)numIterations);
        for( int i = 0; i < numIterations; i++ ) {
            float tx = (float)i*inc + inc;
            float ty = (float)i*inc + inc;
            
            ofSetColor(255,255,255, 255);
            ofSetLineWidth( 100 );
            if( i % 2 == 0 ) ofSetLineWidth( 0.5 );
            
            ofDrawLine( tx, 0, tx, fbo.getHeight() );
            ofDrawLine( 0, ty, fbo.getWidth(), ty );
        }
        
    } fbo.end();
    
    ofSetLineWidth( 1 );
    
    fbo.getTexture().setTextureMinMagFilter( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR );
    
    fbo.getTexture().bind();
    glGenerateMipmap( fbo.getTexture().texData.textureTarget);
//    ofSetMinMagFilters( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR );
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
//    glSamplerParameterf( fbo.getTextureReference().texData.textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2 );
    fbo.getTexture().unbind();
    
    
    fboSphere.allocate( 8, 8, GL_RGB, 4 );
    fboSphere.begin(); {
        ofClear(0, 0, 0, 255 );
        ofSetColor( 0,0,198 );
        ofDrawRectangle(0, 0, fboSphere.getWidth(), fboSphere.getHeight() );
//        ofSetColor(0, 0, 198, 255 );
//        ofSetLineWidth( 3 );
//        ofDrawLine( 0, 0, fboSphere.getWidth(), fboSphere.getHeight() );
//        ofDrawLine( 0, fboSphere.getHeight(), fboSphere.getWidth(), 0 );
        
    } fboSphere.end();
    
    ofSetLineWidth( 1 );
	
    omesh = ofMesh::plane( 50, 50, 14, 14, OF_PRIMITIVE_TRIANGLES );

    ofQuaternion rquat;
    rquat.makeRotate( 90, 1, 0, 0);
    ofSeedRandom();
    float rseed = ofRandom(0, 10000);
    vector< glm::vec3 >& verts = omesh.getVertices();
    for( int i = 0; i < verts.size(); i++ ) {
        verts[i] = rquat * verts[i];
        verts[i].y = ofSignedNoise( verts[i].x*0.03, verts[i].y*0.02 + verts[i].z*0.06, ofGetElapsedTimef() * 0.1 + rseed ) * 3;
//        verts[i].z = ofNoise(ofMap(i, 0, width, 0, perlinRange),  ofMap(i, 0, height, 0, perlinRange) ) * perlinHeight;
//        omesh.setVertex(i, verts[i]);
//        i++;
    }
    
    for (int i = 0; i<600; i++) {
        shared_ptr< ofxBulletSphere > sphere( new ofxBulletSphere() );
        float trad = fabs(sin( ofGetElapsedTimef() ) * 5);
        sphere->create( world.world, ofVec3f( cos( ofGetElapsedTimef()*10.)*trad ,-6, sin(ofGetElapsedTimef()*10)*trad ), 2., 0.2 );
        sphere->add();
        bulletSpheres.push_back( sphere );
    }
    
    
    vector< glm::vec2 >& tcoords = omesh.getTexCoords();
    for( int i = 0; i < tcoords.size(); i++ ) {
        tcoords[i].x *= 4.f;
        tcoords[i].y *= 4.f;
    }
    mesh = omesh;
    
    bulletMesh = shared_ptr< ofxBulletTriMeshShape >( new ofxBulletTriMeshShape() );
    bulletMesh->create( world.world, mesh, ofVec3f(0,0,0), 0.f, ofVec3f(-10000, -10000, -10000), ofVec3f(10000,10000,10000) );
    bulletMesh->add();
    bulletMesh->enableKinematic();
    bulletMesh->setActivationState( DISABLE_DEACTIVATION );
    
    bDrawDebug  = false;
	bDrawFbos   = false;
    bAnimated   = false;
    b_perlinMesh = false;
    mouse_clicked = false;
    
}

//--------------------------------------------------------------
void ofApp::update() {
    
    if(mouse_clicked) {
        shared_ptr< ofxBulletSphere > sphere( new ofxBulletSphere() );
        float trad = fabs(sin( ofGetElapsedTimef() ) * 5);
        sphere->create( world.world, ofVec3f( cos( ofGetElapsedTimef()*10.)*trad ,-6, sin(ofGetElapsedTimef()*10)*trad ), 2., 0.2 );
        sphere->add();
        bulletSpheres.push_back( sphere );
        
    }
    
    for( int i = 0; i < bulletSpheres.size(); i++ ) {
        ofVec3f spos = bulletSpheres[i]->getPosition();
        if( spos.y > 5 ) {
            bulletSpheres.erase( bulletSpheres.begin() + i );
            break;
        }
    }
    
    if(bAnimated) {
        vector< glm::vec3 >& verts = mesh.getVertices();
        vector< glm::vec3 >& overts = omesh.getVertices();
        for( int i = 0; i < verts.size(); i++ ) {
            verts[i].y = ofSignedNoise( verts[i].x*0.030, verts[i].y*0.030 + verts[i].z*0.030, ofGetElapsedTimef() * 0.75 ) * 3;
        }
        bulletMesh->updateMesh( world.world, mesh );
    }
//    if (b_perlinMesh){
//        // distort the z value of each point in our mesh with perlinNoise
//        int i=0;
//        for (int y = 0; y<height; y++){
//            for (int x=0; x<width; x++){
//                ofVec3f newPosition = mesh.getVertex(i);
//                // use the ofMap function to map our x,y inputs to a variable outpur range so we can see different levels of complexity / density in the perlinNoise. then multiply the z distortion by our perlinHeight value to get amplitude of distortion.
//                newPosition.z = ofNoise(ofMap(x, 0, width, 0, perlinRange),  ofMap(y, 0, height, 0, perlinRange) ) * perlinHeight;
//                mesh.setVertex(i, newPosition); // update the position of the vertex with the new
//                i++;
//            }
//        }
//        bulletMesh->updateMesh( world.world, mesh );
//    }
    world.update( ofGetLastFrameTime(), 12 );
    
    
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofEnableDepthTest();
	camera.begin();
	
	ofSetLineWidth(1.f);
	if(bDrawDebug) world.drawDebug();
	else {
        ofSetColor( 255, 255, 255, 255 );
        fbo.getTexture().setTextureWrap( GL_REPEAT, GL_REPEAT );
        fbo.getTexture().bind();
        glSamplerParameterf( fbo.getTexture().texData.textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy );
        mesh.draw();
        fbo.getTexture().unbind();
        
        fboSphere.getTexture().bind();
        for( int i = 0; i < bulletSpheres.size(); i++ ) {
            bulletSpheres[i]->draw();
        }
        fboSphere.getTexture().unbind();
    }
	
    
    if( bDrawDebug ) {
        ofDrawAxis(0.5);
    }
	
	camera.end();
    ofDisableDepthTest();
    
    ofSetColor( 255 );
    if( bDrawFbos ) {
        fbo.draw( ofGetWidth()-20-fbo.getWidth(), 20 );
        fboSphere.draw( ofGetWidth()-20-fboSphere.getWidth(), 40+fbo.getHeight() );
    }
	
	ofSetColor(255, 255, 255);
	stringstream ss;
	ss << "framerate: " << ofToString(ofGetFrameRate(),0) << endl;
	ss << "draw debug (d): " << ofToString(bDrawDebug, 0) << endl;
    ss << "draw textures (e): " << ofToString(bDrawFbos, 0) << endl;
    ss << "click to add water: " << ofToString(bulletSpheres.size(), 0) << endl;
    ss << "animated (a): " << ofToString(bAnimated, 0) << endl;
	ofDrawBitmapString( ss.str().c_str(), 20, 20 );
}

//--------------------------------------------------------------
void ofApp::exit() {
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
    
		case 'd':
			bDrawDebug = !bDrawDebug;
			break;
        case 'e':
            bDrawFbos = !bDrawFbos;
        case 127:
            bulletSpheres.clear();
            break;
        case 'a':
            bAnimated = !bAnimated;
            break;
        case 'p':
            b_perlinMesh = !b_perlinMesh;
            break;
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    mouse_clicked = !mouse_clicked;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    mouse_clicked = !mouse_clicked;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
	
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) { 
	
}

