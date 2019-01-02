-- Copyright (c) 2018, Boris Bogaerts
-- All rights reserved.

-- Redistribution and use in source and binary forms, with or without 
-- modification, are permitted provided that the following conditions 
-- are met:

-- 1. Redistributions of source code must retain the above copyright 
-- notice, this list of conditions and the following disclaimer.

-- 2. Redistributions in binary form must reproduce the above copyright 
-- notice, this list of conditions and the following disclaimer in the 
-- documentation and/or other materials provided with the distribution.

-- 3. Neither the name of the copyright holder nor the names of its 
-- contributors may be used to endorse or promote products derived from 
-- this software without specific prior written permission.

-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
-- A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
-- HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
-- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
-- LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
-- DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
-- THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

getNextGeometryInformation = function(inInts,inFloats,inStrings,inBuffer)
    local shapeIndex = inInts[1]
    local size = {0,0,0,-1}
    local returnValues = {}
     while (true) do
        h=sim.getObjects(shapeIndex,sim.object_shape_type)
        if (h<0) then
            break
        elseif (h==-1) then
            break
        end
        
        property=sim.getObjectSpecialProperty(h)
        val = sim.boolAnd32(property, sim.objectspecialproperty_renderable)
        if val>0 then
            vertices,indices=sim.getShapeMesh(h)
            a=sim.getShapeViz(h,0)
            size[1] = #vertices
            size[2] = #indices
            size[3] = shapeIndex+1
            size[4] = h
            if (a.texture==nil) then --two modes (1) no texture coordinates
                size[5] = 0
                for i = 1, #vertices, 1 do
                    returnValues[#returnValues + 1] = vertices[i]
                end
                for i = 1, #indices, 1 do
                    returnValues[#returnValues + 1] = indices[i]
                end
            else -- (2) with texture coordinates (basically duplicate vertices per triangle)
                size[5] = 1
                size[1] = #indices*3
                for i = 1, #indices, 1 do
                    val = ((indices[i])*3)
                    returnValues[#returnValues + 1] = vertices[val+1]
                    returnValues[#returnValues + 1] = vertices[val+2]
                    returnValues[#returnValues + 1] = vertices[val+3]
                end
                for i = 1, #indices, 1 do
                returnValues[#returnValues + 1] = i-1
                end
            end          
            dump, color=sim.getShapeColor(h, nil ,sim.colorcomponent_ambient_diffuse)
            for i = 1, 3, 1 do
                returnValues[#returnValues + 1] = color[i]
            end
            dump, opacity=sim.getShapeColor(h, nil ,sim.colorcomponent_transparency)
			
			if (opacity[1]==0.5) then
				returnValues[#returnValues + 1] = 1 -- opacity[1] -- (0.5 if not turned ON)
			else
				returnValues[#returnValues + 1] = opacity[1] -- (0.5 if not turned ON)
			end
            objectName=sim.getObjectName(h)
            break
        else
            shapeIndex=shapeIndex+1
        end
     end
    return size, returnValues, {objectName}, ''
end

getTextureInformation = function(inInts,inFloats,inStrings,inBuffer)
    a=sim.getShapeViz(inInts[1],0)
    sim.saveImage(a.texture.texture,a.texture.resolution,1,"textureTransfer.png",100)
    return {}, a.texture.coordinates, {"C:/Program Files/V-REP3/V-REP_PRO_EDU/textureTransfer.png"}, ''
end

helloCams = function(inInts, inFloats, inStrings,inBuffer)
    local savedState=sim.getInt32Parameter(sim.intparam_error_report_mode)
    sim.setInt32Parameter(sim.intparam_error_report_mode,0)
    val = {}
    val[1] = sim.getObjectHandle('Camera_feeder')
    sim.setInt32Parameter(sim.intparam_error_report_mode,savedState)
    return val, {},{}, ''
end

helloVolume = function(inInts, inFloats, inStrings,inBuffer)
    local savedState=sim.getInt32Parameter(sim.intparam_error_report_mode)
    sim.setInt32Parameter(sim.intparam_error_report_mode,0)
    val = {}
    val[1] = sim.getObjectHandle('Field')
    sim.setInt32Parameter(sim.intparam_error_report_mode,savedState)
    return val, {},{}, ''
end

function displayTextureInfo(name)
    data=sim.getShapeViz(sim.getObjectHandle(name),0)
end

-- This function will check for a child vision sensor
function checkForCam(inInts, inFloats, inStrings, inBuffer)
    h = sim.getObjectHandle("HTC_VIVE")
    cams=sim.getObjectsInTree(h,sim.object_visionsensor_type)
    data = {}
    if (#cams==0) then
        data[1] = 0
    else
        -- get required camera parameters
        result,perspAngle=sim.getObjectFloatParameter(cams[1], sim.visionfloatparam_perspective_angle)
        result,nearClip=sim.getObjectFloatParameter(cams[1], sim.visionfloatparam_near_clipping)
        result,farClip=sim.getObjectFloatParameter(cams[1], sim.visionfloatparam_far_clipping)
        resolution=sim.getVisionSensorResolution(cams[1]) 
        -- fill data structure
        data[1] = cams[1]
        data[2] = resolution[1]
        data[3] = resolution[2]
        data[4] = perspAngle
        data[5] = nearClip
        data[6] = farClip
    end
    return {}, data, {}, ''
end

function isRunning(inInts, inFloats, inStrings, inBuffer)
	result = {}
	simulationState=sim.getSimulationState()
	if (simulationState==sim.simulation_advancing_running) then
		result[1] = 1
	else
	    result[1] = 0
	end
	if (sim.getScriptSimulationParameter(sim.handle_self, "Lock_VTK_controls")==true) then
		result[2] = 1
	else
		result[2] = 0
	end
	print(result)
	return result, {}, {}, ''
end

function getEstetics(inInts, inFloats, inStrings, inBuffer)
	local t1 = sim.getArrayParameter(sim.arrayparam_background_color1)
	local t2 = sim.getArrayParameter(sim.arrayparam_background_color2)
	result = {t1[1], t1[2], t1[3], t2[1], t2[2], t2[3]}
	return {}, result, {}, ''
end

function sysCall_sensing() -- example of something to do with the buttonpress events
    if (hDum==nil) then
		return
	end
	state = sim.getIntegerSignal('RightControllerGrip')==1
	if (state and (attatched==false) and (prevState==false))  then
		sim.setObjectParent(hDum,ch,true)
		attatched = true
	elseif (state and (attatched==true) and (prevState==false)) then
		sim.setObjectParent(hDum,-1,true)
		attatched = false
	end
	prevState = sim.getIntegerSignal('RightControllerGrip')==1
end


function sysCall_init()
    -- simRemoteApi.start(19999,1300,true) -- if using different port
    sim.setIntegerSignal('LeftControllerTrigger',0) -- set up string signal to transfer button state
    sim.setIntegerSignal('LeftControllerGrip',0)
    sim.setIntegerSignal('LeftControllerTrackPadX',0)
    sim.setIntegerSignal('RightControllerTrigger',0)
    sim.setIntegerSignal('RightControllerGrip',0)
    sim.setIntegerSignal('RightControllerTrackPadX',0)
	
	-- see if interactor is active
	h = sim.getObjectHandle('HTC_VIVE')
	ch = sim.getObjectHandle('Controller1')
    name = sim.getScriptSimulationParameter(sim.handle_self, "dummy_child_interactor")
	local savedState=simGetInt32Parameter(sim_intparam_error_report_mode)
    simSetInt32Parameter(sim_intparam_error_report_mode,0) -- we want none of them errors if object does not exist
	hDum = nil
	hDum = sim.getObjectHandle(name)
	simSetInt32Parameter(sim_intparam_error_report_mode,savedState)
	attatched = false
	if (hDum<1) then
		print("Invalid dummy name (standard interactor is disabled)")
		hDum = nil
	end
	prevState = (sim.getIntegerSignal('RightControllerGrip')==1)
end
