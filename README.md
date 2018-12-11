# V-REP-VR-Interface
This repository contains the code to experience V-REP in VR. It can visualize V-REP scenes in openvr compatible devices (HTC-VIVE and HTC-VIVE Pro are tested) and return user manipulations to V-REP. It is also able to visualize the coverage of camera systems (later more).

Installation instructions and a feature overview is now also available in a youtube video:
https://www.youtube.com/watch?v=_GXVdgihVgQ

Some Youtube videos showing the interface in action:
- https://www.youtube.com/watch?v=yMydjviF7yg
- https://www.youtube.com/watch?v=Dsh8oyN4sD0&t

The interface has the following capabilities:
- Read all renderable (importand) geometry from V-REP (colors, opacities and textures are also transfered)
- Synchronize all poses in real time continuously
- Send the positions of controllers to V-REP
- Send buttonpress events to V-REP as string signals

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

@article{bogaerts2018interactive,
  title={Interactive Camera Network Design using a Virtual Reality Interface},
  author={Bogaerts, Boris and Sels, Seppe and Vanlanduit, Steve and Penne, Rudi},
  journal={arXiv preprint arXiv:1809.07593},
  year={2018}
}

