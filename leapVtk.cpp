
//VTK includes
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkRendererCollection.h>
//Leap Controller includes
#include "Leap.h"
//system includes
#include <iostream>
#include <string.h>
#include <math.h>

using namespace Leap;

//Global Variables, please dont hate me
//I started with only 2 globals and now its ballooned out quite a bit,
//  but Im still not sure how to pass varialbes to the event handler
//  so for the time being its just going to have to be this way
double hand_x = 0;
double hand_y = 0;
double hand_dir_x = 0;
double hand_dir_y = 0;
double grab = 0;
int xSize = 1000;
int ySize = 1000;
double cameraRoll = 0;
double cameraYaw = 0;
double cameraElevation = 0;
double posX = 0;
double posY = 0;
double posZ = 0;
bool handPresent = false;
bool switchBgRed = false;
bool switchBgGreen = false;
vtkSmartPointer<vtkCamera> camera;
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;


class SampleListener : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);

  private:
};

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void SampleListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
  controller.enableGesture(Gesture::TYPE_CIRCLE);
  controller.enableGesture(Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();
  // std::cout << "Frame id: " << frame.id()
  //           << ", timestamp: " << frame.timestamp()
  //           << ", hands: " << frame.hands().count()
  //           << ", extended fingers: " << frame.fingers().extended().count()
  //           << ", tools: " << frame.tools().count()
  //           << ", gestures: " << frame.gestures().count() << std::endl;
  handPresent = false;
  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    // Get the first hand
    const Hand hand = *hl;
    handPresent = true;
    grab = hand.grabStrength();
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
    // std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
    //           << ", palm position: " << hand.palmPosition() << std::endl;
    // Get the hand's normal vector and direction
    const Vector normal = hand.palmNormal();
    const Vector direction = hand.direction();
    const Vector pos = hand.palmPosition();
    //std::cout << "Normal x:" << normal.x << " y:" << normal.y << '\n';
    hand_x = normal.x;
    hand_y = normal.y;
    hand_dir_x = direction.x;
    hand_dir_y = direction.y;
    posX = pos.x;
    posY = pos.y;
    // Calculate the hand's pitch, roll, and yaw angles
    // std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
    //           << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
    //           << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

    // Get the Arm bone
    Arm arm = hand.arm();
    // std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
    //           << " wrist position: " << arm.wristPosition()
    //           << " elbow position: " << arm.elbowPosition() << std::endl;

    // Get fingers
    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;
      // std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
      //           << " finger, id: " << finger.id()
      //           << ", length: " << finger.length()
      //           << "mm, width: " << finger.width() << std::endl;

      // Get finger bones
      for (int b = 0; b < 4; ++b) {
        Bone::Type boneType = static_cast<Bone::Type>(b);
        Bone bone = finger.bone(boneType);
        // std::cout << std::string(6, ' ') <<  boneNames[boneType]
        //           << " bone, start: " << bone.prevJoint()
        //           << ", end: " << bone.nextJoint()
        //           << ", direction: " << bone.direction() << std::endl;
      }
    }
  }

  // Get tools
  const ToolList tools = frame.tools();
  for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
    const Tool tool = *tl;
    // std::cout << std::string(2, ' ') <<  "Tool, id: " << tool.id()
    //           << ", position: " << tool.tipPosition()
    //           << ", direction: " << tool.direction() << std::endl;
  }

  // Get gestures
  const GestureList gestures = frame.gestures();
  for (int g = 0; g < gestures.count(); ++g) {
    Gesture gesture = gestures[g];

    switch (gesture.type()) {
      case Gesture::TYPE_CIRCLE:
      {
        CircleGesture circle = gesture;
        std::string clockwiseness;

        if (circle.pointable().direction().angleTo(circle.normal()) <= PI/2) {
          clockwiseness = "clockwise";
          switchBgRed = true;
        } else {
          clockwiseness = "counterclockwise";
          switchBgGreen = true;
        }

        // Calculate angle swept since last frame
        float sweptAngle = 0;
        if (circle.state() != Gesture::STATE_START) {
          CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
          sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
        }
        // std::cout << std::string(2, ' ')
        //           << "Circle id: " << gesture.id()
        //           << ", state: " << stateNames[gesture.state()]
        //           << ", progress: " << circle.progress()
        //           << ", radius: " << circle.radius()
        //           << ", angle " << sweptAngle * RAD_TO_DEG
        //           <<  ", " << clockwiseness << std::endl;
        break;
      }
      case Gesture::TYPE_SWIPE:
      {
        SwipeGesture swipe = gesture;
        // std::cout << std::string(2, ' ')
        //   << "Swipe id: " << gesture.id()
        //   << ", state: " << stateNames[gesture.state()]
        //   << ", direction: " << swipe.direction()
        //   << ", speed: " << swipe.speed() << std::endl;
        break;
      }
      case Gesture::TYPE_KEY_TAP:
      {
        KeyTapGesture tap = gesture;
        // std::cout << std::string(2, ' ')
        //   << "Key Tap id: " << gesture.id()
        //   << ", state: " << stateNames[gesture.state()]
        //   << ", position: " << tap.position()
        //   << ", direction: " << tap.direction()<< std::endl;
        break;
      }
      case Gesture::TYPE_SCREEN_TAP:
      {
        ScreenTapGesture screentap = gesture;
        // std::cout << std::string(2, ' ')
        //   << "Screen Tap id: " << gesture.id()
        //   << ", state: " << stateNames[gesture.state()]
        //   << ", position: " << screentap.position()
        //   << ", direction: " << screentap.direction()<< std::endl;
        break;
      }
      default:
        std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
        break;
    }
  }

  if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
    std::cout << std::endl;
  }
}

void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void SampleListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

void RenderCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData ) {
  vtkRenderer* renderer = static_cast<vtkRenderer*>(caller);
 
  double timeInSeconds = renderer->GetLastRenderTimeInSeconds();
  double fps = 1.0/timeInSeconds;
  std::cout << "FPS: " << fps << std::endl;
 
  std::cout << "Callback" << std::endl;
}
 
class vtkTimerCallback : public vtkCommand
{
  public:
    static vtkTimerCallback *New()
    {
      vtkTimerCallback *cb = new vtkTimerCallback;
      cb->TimerCount = 0;
      return cb;
    }
 
    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId, void *vtkNotUsed(callData))
    {
      if (vtkCommand::TimerEvent == eventId)
      {
      ++this->TimerCount;
      }
        // cout << "posx:" << posX << " posy:" << posY 
        //     << " dirx:" << hand_dir_x << " diry:" << hand_dir_y << " grab:" << grab << '\n';
      
      //camera->Roll(5);
      //camera->Pitch(5);
      //double x, y, z;
      //camera->GetPosition(x, y, z);
      //std::cout << x << ' ' << y << ' ' << z << '\n';
      //camera->Elevation(0.5);
      //camera->Roll(0.5);
      //camera->Yaw(0.5);
      if(handPresent){
        if(grab < 0.8)
          grab = (1/(20*(grab+0.01)));
        else
          grab = 2;
        if(switchBgRed){
          renderWindowInteractor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->SetBackground(.3, .2, .1);
          switchBgRed = false;
        }
        if(switchBgGreen){
          renderWindowInteractor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->SetBackground(.1, .2, .2);
          switchBgGreen = false;
        }
        /*
        if(posY < 90) {
          camera->Zoom(100-posY);
        }
        else if(posY > 120) {
          camera->Zoom(100-posY);
        }
        */
        camera->Elevation(hand_y*hand_dir_y*grab);
        camera->Azimuth(hand_x*grab);
        renderWindowInteractor->Render();
      }
      
      hand_x = 0;
      hand_dir_y = 0;
      //cout << "x:" << eventX << " y:" << eventY << '\n';

    }
 
  private:
    int TimerCount;
 
};



int main(int argc, char *argv[])
{
  /****** setup VTK ******/ 
  //create a cube
  vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
  cubeSource->SetCenter(0.0, 0.0, 0.0);
  cubeSource->SetXLength(5.0);
  cubeSource->SetYLength(6.0);
  cubeSource->SetZLength(5.0);
 
  // Create a mapper and actor
  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(cubeSource->GetOutputPort());
  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
 
  //Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
 
  //create the camera
  camera = vtkSmartPointer<vtkCamera>::New();
  camera->SetPosition(0, 0, 20);
  camera->SetFocalPoint(0, 0, 0);
  renderer->SetActiveCamera(camera);
  //camera->Roll(50.0);
  // Add the actor to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(.1, .3, .2); // Background color dark green
 

  
  // Initialize must be called prior to creating timer events.
  renderWindowInteractor->Initialize();

  /****** Setup leap controller ******/
  // Create a sample listener and controller
  SampleListener listener;
  Controller controller;

  // Have the sample listener receive events from the controller
  controller.addListener(listener);

  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  /****** Start VTK window ******/
  // Render and interact
  renderWindow->SetWindowName(argv[0]);
  renderWindow->SetSize(xSize, ySize);

  //register the callback for each render event
  vtkSmartPointer<vtkTimerCallback> callback =
    vtkSmartPointer<vtkTimerCallback>::New();

  renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, callback);
  int timerId = renderWindowInteractor->CreateRepeatingTimer(10);
  //std::cout << "timerId: " << timerId << std::endl; 
  renderWindow->Render();
  renderWindowInteractor->Start();
 
  // Remove the leap controller listener during teardown
  // Remove the sample listener when done
  controller.removeListener(listener);


  return EXIT_SUCCESS;
}
