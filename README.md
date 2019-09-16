# V-REP-VR-Toolbox
This repository contains virtual reality tools for VREP. V-REP is a robotics simulation software package that is available here: http://www.coppeliarobotics.com/

Currently two tools are available
- V-REP-VR-Interface
- VR 360 cam (omnidirectional stereo renderer)

# V-REP-VR-Interface
This repository contains the code to experience V-REP in VR. It can visualize V-REP scenes in openvr compatible devices (HTC-VIVE and HTC-VIVE Pro are tested) and return user manipulations to V-REP. It is also able to visualize the coverage of camera systems (later more).

Installation instructions and a feature overview is now also available in a youtube video:
https://www.youtube.com/watch?v=_GXVdgihVgQ
An update video of new features in V2 and V2.1 is also available:
https://youtu.be/ozam2Ew7RdA

Some Youtube videos showing the interface in action:
- https://www.youtube.com/watch?v=yMydjviF7yg
- https://www.youtube.com/watch?v=Dsh8oyN4sD0&t
- https://youtu.be/nakQGTs4Fs0
- https://youtu.be/3Lvhmh3th3Q
- https://www.youtube.com/watch?v=QYX7WIiahbw (courtesy of Mathieu Lauret and Amir Beddiaf)

The interface has the following capabilities:
- Read all renderable geometry from V-REP (colors, opacities and (moving) textures are also transfered)
- Read dynamically generated geometry
- Synchronize all poses in real time
- Send the positions of controllers and headset to V-REP
- Send all controller buttonpress/buttontouch events to V-REP as string signals
- Send trackpad position to V-REP
- Import standard interactions as dummies, that can be attatched to controllers
- Add a customizable menu in VR that when menu items are selected, triggers the execution of V-REP scripts.
- Visibility layers are respected, which allows for the possibility of hiding objects.

# VR 360 cam
This tool renders omnidirectional stereo images for a V-REP vision sensor. You don't need any physical VR device to use this tool (only to view the result). The theory behind this rendering process is exelently explained here : https://developers.google.com/vr/jump/rendering-ods-content.pdf.

Some videos produced with this tool:
- https://youtu.be/7O629j58bTI
- https://youtu.be/pFAptrCYhaQ

To use this tool, import the VR360_cam.ttm model in your V-REP scene. Next launch VR360_cam.exe (maybe as administrator, a file is saved in the location of the .exe file, this action could require administrator privileges). 
This process is demonstrated at the end of this video https://youtu.be/ozam2Ew7RdA.

Note that the HTC_VIVE.ttm model must be in the V-REP scene to do this (you don't need a VIVE for this to work).

Currently the performance is quite decent 1 image in a complex scene renders in approximately 15 seconds (requires 16.384 individual renders, approx 1100 fps).

If you make a video by composing multiple images, do not forget to inject the correct metadata https://github.com/google/spatial-media/releases for it to be interpreted correctly.

# Installation (easy)
Installation instructions are now also available on youtube: https://www.youtube.com/watch?v=_GXVdgihVgQ

This repository can be installed by an installer package available under the release tab in this repository. 

After running the installer the user needs to run the bat file (copyToVREP.bat) as administrator. This bat file can be found in the installation directory of the interface typically (C:\Program Files (x86)\V-REP VR interface). 
IMPORTANT if V-REP is not installed in the default (my default at least) location (C:\Program Files\V-REP3\V-REP_PRO_EDU), than the correct folder should be identified in the bat file. The bat file can be changed by opening it in your favorite text editor (be very precise in the way you specify the folder, no spaces etc).

The bat file will copy following files to your V-REP installation folder:
- Lua scripts that will be accesed by V-REP objects belonging to the interface (required)
- A custom remoteApiConnections.txt file which opens more ports (required, if coverage visualization is used at least)
- Models used by the VR-interface (usefull)
- Example scenes (usefull)

# Usage
The basic principle of the interface is simple. Activate steam and make shure the device is running and recognized. And run the V-REP VR Interface.exe program. If the user starts a simulation in V-REP (if the scene contains the HTC-VIVE.ttm object) the interface will read all geometry and start the visualization. If the user stops the simulation, the interface will stop the visualization and wait for the next simulation start (can ba a different scene, doesn't matter).

More specific instructions are provided in demo scenes. The logical order of which are
- Hello_vr_world.ttt
- Hello_controls.ttt
- Hello_signals.ttt
- Hello_camera_coverage.ttt

# Interactive Camera Network Design using a Virtual Reality Interface
The interface contains the implementation of the paper "Interactive Camera Network Design using a Virtual Reality Interface" available at https://arxiv.org/abs/1809.07593 (yt:https://www.youtube.com/watch?v=Dsh8oyN4sD0&t=2s)

The controls and usage are already explained in example Hello_camera_coverage.ttt. 

Two scenes used in the paper are also provided:
- VR_sensorPlacement_harbour.ttt
- VR_sensorPlacement_office.ttt

Please cite this paper if the interface is used in a relevant context (camera placement/coverage visualization)

@article{bogaerts2019interactive,
  title={Interactive Camera Network Design Using a Virtual Reality Interface},
  author={Bogaerts, Boris and Sels, Seppe and Vanlanduit, Steve and Penne, Rudi},
  journal={Sensors},
  volume={19},
  number={5},
  pages={1003},
  year={2019},
  publisher={Multidisciplinary Digital Publishing Institute}
}

# Enabling Humans to Plan Inspection Paths Using a Virtual Reality Interface
The interface also contains the implementation of the paper "Enabling Humans to Plan Inspection Paths Using a Virtual Reality Interface" available at https://arxiv.org/abs/1909.06077

The usage is the same as in the demo scene: Hello_camera_coverage.ttt. To visualize the quality on a mesh, just make the mesh a child of the Field model.

Additional signals:
- To reset a measurement state: sim.setIntegerSignal("ResetMeasurement", 1)
- To record a path: sim.setIntegerSignal("MeasurementInProgress", 1)

Please cite this paper if the interface is used in a relevant context (Robotic inspection planning)

@article{bogaerts2019enabling,
  title={Enabling Humans to Plan Inspection Paths Using a Virtual Reality Interface},
  author={Bogaerts, Boris and Sels, Seppe and Vanlanduit, Steve and Penne, Rudi},
  journal={arXiv preprint arXiv:1909.06077},
  year={2019}
}

