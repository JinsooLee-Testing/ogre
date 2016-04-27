#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include <Ogre.h>
#include <OIS/OIS.h>
#include <iostream>
#include <cmath>
using namespace Ogre;
using namespace std;

class ESCListener : public FrameListener {
	OIS::Keyboard *mKeyboard;

public:
	ESCListener(OIS::Keyboard *keyboard) : mKeyboard(keyboard) {}
	bool frameStarted(const FrameEvent &evt)
	{
		mKeyboard->capture();
		return !mKeyboard->isKeyDown(OIS::KC_ESCAPE);
	}
};


class MainListener : public FrameListener {
	OIS::Keyboard *mKeyboard;
	Root* mRoot;
	SceneNode *mProfessorNode, *mfishNode, *mfishRotationPivotNode;

public:
	MainListener(Root* root, OIS::Keyboard *keyboard) : mKeyboard(keyboard), mRoot(root)
	{
		mProfessorNode = mRoot->getSceneManager("main")->getSceneNode("Professor");
		mfishRotationPivotNode = mRoot->getSceneManager("main")->getSceneNode("Pivot");
		mfishNode = mRoot->getSceneManager("main")->getSceneNode("fish");

		mProfessorNode->setPosition(0.0f, 0.0f, 0.0f);
		mfishRotationPivotNode->setPosition(0.0f, 0.0f, 0.0f);
		mfishRotationPivotNode->setInheritOrientation(false);
	}

	bool frameStarted(const FrameEvent &evt)
	{
		enum state{ TURNING, MOVE };
		static float ProfessorSpeed = 100.0f;
		static Vector3 ProfessorDirection(0, 0, 1);
		static float ProfessorRotateDegree = 1.0f;
		static float RotatingCount = 0.0f;
		static bool ProfessorTurning = false;
		const Real MAX_POSITION = 250.0f;
		const Real MIN_POSITION = -250.0f;
		static Vector3 FishDirection(0, 0, 0);
		static Vector3 FishPivotPosition(0, 0, 0);

		mProfessorNode->translate(0, 0, ProfessorSpeed * ProfessorDirection.z  * evt.timeSinceLastFrame);

		if (MAX_POSITION <= mProfessorNode->getPosition().z || MIN_POSITION >= mProfessorNode->getPosition().z)
		{
			ProfessorTurning = true;
			ProfessorDirection.z *= -1.0f;
		}
		else	// -250.f  <  mProfessorNode->getPosition().z  >  250.f
		{

		}

		if (ProfessorTurning == true)
		{
			mProfessorNode->yaw(Degree(ProfessorRotateDegree));
			RotatingCount += ProfessorRotateDegree;
			if (180 <= RotatingCount)
			{
				ProfessorTurning = false;
				RotatingCount = 0.0f;
			}
		}

		mfishRotationPivotNode->yaw(Degree(-1));
		FishPivotPosition.x = mProfessorNode->getPosition().x;
		FishPivotPosition.y = mProfessorNode->getPosition().y;
		FishPivotPosition.z = mProfessorNode->getPosition().z;
		mfishRotationPivotNode->setPosition(FishPivotPosition);

		return true;
	}

};

class LectureApp {

	Root* mRoot;
	RenderWindow* mWindow;
	SceneManager* mSceneMgr;
	Camera* mCamera;
	Viewport* mViewport;
	OIS::Keyboard* mKeyboard;
	OIS::InputManager *mInputManager;

	MainListener* mMainListener;
	ESCListener* mESCListener;

public:

	LectureApp() {}

	~LectureApp() {}

	void go(void)
	{
		// OGRE�� ���� ��Ʈ ������Ʈ ����
#if !defined(_DEBUG)
		mRoot = new Root("plugins.cfg", "ogre.cfg", "ogre.log");
#else
		mRoot = new Root("plugins_d.cfg", "ogre.cfg", "ogre.log");
#endif


		// �ʱ� ������ ���ǱԷ��̼� ���� - ogre.cfg �̿�
		if (!mRoot->restoreConfig()) {
			if (!mRoot->showConfigDialog()) return;
		}

		mWindow = mRoot->initialise(true, "Rotate : Copyleft by Dae-Hyun Lee");


		// ESC key�� ������ ���, ����� ���� ������ ������ Ż���� ó��
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		OIS::ParamList pl;
		mWindow->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		mInputManager = OIS::InputManager::createInputSystem(pl);
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));


		mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "main");
		mCamera = mSceneMgr->createCamera("main");


		mCamera->setPosition(0.0f, 100.0f, 700.0f);
		mCamera->lookAt(0.0f, 100.0f, 0.0f);

		mCamera->setNearClipDistance(5.0f);

		mViewport = mWindow->addViewport(mCamera);
		mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.5f));
		mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));


		ResourceGroupManager::getSingleton().addResourceLocation("resource.zip", "Zip");
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		mSceneMgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));

		// ��ǥ�� ǥ��
		Ogre::Entity* mAxesEntity = mSceneMgr->createEntity("Axes", "axes.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode("AxesNode", Ogre::Vector3(0, 0, 0))->attachObject(mAxesEntity);
		mSceneMgr->getSceneNode("AxesNode")->setScale(5, 5, 5);

		_drawGridPlane();


		Entity* entity1 = mSceneMgr->createEntity("Professor", "DustinBody.mesh");
		SceneNode* node1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("Professor", Vector3(0.0f, 0.0f, 0.0f));
		node1->attachObject(entity1);

		SceneNode* pivotNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Pivot", Vector3(0, 0, 0));

		Entity* entity2 = mSceneMgr->createEntity("fish", "fish.mesh");
		SceneNode* node2 = pivotNode->createChildSceneNode("fish", Vector3(100.0f, 0.0f, 0.0f));
		node2->yaw(Degree(90.0f));
		node2->attachObject(entity2);
		node2->scale(10, 10, 10);
		//mSceneMgr->getSceneNode("fish")->setScale(10, 10, 10);

		mESCListener = new ESCListener(mKeyboard);
		mRoot->addFrameListener(mESCListener);

		mMainListener = new MainListener(mRoot, mKeyboard);
		mRoot->addFrameListener(mMainListener);


		mRoot->startRendering();

		mInputManager->destroyInputObject(mKeyboard);
		OIS::InputManager::destroyInputSystem(mInputManager);

		delete mRoot;
	}

private:
	void _drawGridPlane(void)
	{
		Ogre::ManualObject* gridPlane = mSceneMgr->createManualObject("GridPlane");
		Ogre::SceneNode* gridPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("GridPlaneNode");

		Ogre::MaterialPtr gridPlaneMaterial = Ogre::MaterialManager::getSingleton().create("GridPlanMaterial", "General");
		gridPlaneMaterial->setReceiveShadows(false);
		gridPlaneMaterial->getTechnique(0)->setLightingEnabled(true);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setDiffuse(1, 1, 1, 0);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setAmbient(1, 1, 1);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1, 1, 1);

		gridPlane->begin("GridPlaneMaterial", Ogre::RenderOperation::OT_LINE_LIST);
		for (int i = 0; i<21; i++)
		{
			gridPlane->position(-500.0f, 0.0f, 500.0f - i * 50);
			gridPlane->position(500.0f, 0.0f, 500.0f - i * 50);

			gridPlane->position(-500.f + i * 50, 0.f, 500.0f);
			gridPlane->position(-500.f + i * 50, 0.f, -500.f);
		}

		gridPlane->end();

		gridPlaneNode->attachObject(gridPlane);
	}
};


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		LectureApp app;

		try {

			app.go();

		}
		catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif