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

getVisibleHandles = function(inInts, inFloats, inStrings, inBuffer)
	handles = simGetObjectsInTree(sim_handle_scene, sim.object_shape_type, 0)
	ret = {}
	if (toRestore==nil) then
		toRestore = {}
	end
	for i = 1, #handles, 1 do
		property=sim.getObjectSpecialProperty(handles[i])
        val = sim.boolAnd32(property, sim.objectspecialproperty_renderable)
		if val>0 then
			simpleShapeHandles=sim.ungroupShape(handles[i])
			--simpleShapeHandles = {handles[i]}
			if #simpleShapeHandles>1 then
				toRestore[#toRestore + 1] = simpleShapeHandles
			end
			for ii = 1, #simpleShapeHandles, 1 do
				ret[#ret+1] = simpleShapeHandles[ii]
			end
		end
	end
	handles = simGetObjectsInTree(sim_handle_scene, sim.object_shape_type, 0)
	numberOfObjects = #handles
	return ret, {}, {}, ''
end


function sysCall_cleanup()
    if (toRestore==nil)==false then
		for i = 1, #toRestore, 1 do
			sim.groupShapes(toRestore[i]) -- vrep does not seem to do this correctly 
		end
	end
end

getGeometryInformation = function(inInts, inFloats, inStrings, inBuffer)
	h = inInts[1]
	local size = {0,0,0,-1}
	local returnValues = {}
	vertices,indices=sim.getShapeMesh(h)
    a=sim.getShapeViz(h,0)
    size[1] = #vertices
    size[2] = #indices
    size[3] = 0
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
	
	dump, size[6] = sim.getObjectInt32Parameter(h, sim.objintparam_visibility_layer)
	return size, returnValues, {objectName}, ''
end


function getLightInfo(inInts,inFloats,inStrings,inBuffer)
	handles = simGetObjectsInTree(sim_handle_scene, sim.object_light_type, 0)
	inInts = {#handles}
	returnValues = {}
	for i = 1, #handles, 1 do		
		dimp, dump2, c1, c2 = sim.getLightParameters(handles[i])
		returnValues[#returnValues + 1] = c1[1]
		returnValues[#returnValues + 1] = c1[2]
		returnValues[#returnValues + 1] = c1[3]
		returnValues[#returnValues + 1] = c2[1]
		returnValues[#returnValues + 1] = c2[2]
		returnValues[#returnValues + 1] = c2[3]
		dump, returnValues[#returnValues + 1] = sim.getObjectFloatParameter(handles[i], sim.lightfloatparam_spot_cutoff)
		returnValues[#returnValues] =  180 * returnValues[#returnValues]/3.1415
		dump, returnValues[#returnValues + 1] = sim.getObjectFloatParameter(handles[i], sim.lightfloatparam_spot_exponent)
		dump, returnValues[#returnValues + 1] = sim.getObjectFloatParameter(handles[i], sim.lightfloatparam_const_attenuation)
		dump, returnValues[#returnValues + 1] = sim.getObjectFloatParameter(handles[i], sim.lightfloatparam_lin_attenuation)
		dump, returnValues[#returnValues + 1] = sim.getObjectFloatParameter(handles[i], sim.lightfloatparam_quad_attenuation)
		inInts[#inInts + 1] = handles[i]
	end
	return inInts, returnValues, {}, ''
end

getTextureInformation = function(inInts,inFloats,inStrings,inBuffer)
    a=sim.getShapeViz(inInts[1],0)
    sim.saveImage(a.texture.texture,a.texture.resolution,1,"textureTransfer.png",100)
    return {}, a.texture.coordinates, {sim.getStringParameter(sim.stringparam_application_path) .."/textureTransfer.png"}, ''
end

helloCams = function(inInts, inFloats, inStrings,inBuffer)
    val = {}
    val[1] = sim.getObjectHandle('Camera_feeder@silentError')
    return val, {},{}, ''
end

helloVolume = function(inInts, inFloats, inStrings,inBuffer)
    val = {}
    val[1] = sim.getObjectHandle('Field@silentError')
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
	return result, {}, {}, ''
end

function getEstetics(inInts, inFloats, inStrings, inBuffer)
	local t1 = sim.getArrayParameter(sim.arrayparam_background_color1)
	local t2 = sim.getArrayParameter(sim.arrayparam_background_color2)
	result = {t1[1], t1[2], t1[3], t2[1], t2[2], t2[3]}
	return {}, result, {}, ''
end

function sysCall_sensing() -- example of something to do with the buttonpress events
	sim.setIntegerSignal("VisibleLayers", sim.getInt32Parameter(sim.intparam_visible_layers))
	if (sim.getScriptSimulationParameter(sim.handle_self, "High_quality_render")==true) and (goodRendering==false) then
		goodRendering = true
		print("The good rendering is activated")
		sim.setIntegerSignal("High_quality_render",1)
	end
	
	if (sim.getScriptSimulationParameter(sim.handle_self, "High_quality_render")==false) and (goodRendering==true) then
		goodRendering = false
		print("The good rendering is deactivated")
		sim.setIntegerSignal("High_quality_render",0)
	end
	
	if (numberOfObjects==nil) or (dynamicLoading==false) then
		return
	end
    handles = simGetObjectsInTree(sim_handle_scene, sim.object_shape_type, 0)
	if(numberOfObjects<#handles) then
		sim.setIntegerSignal('dynamic_load_request',1)
	end
end

getVisibleHandlesDynamic = function(inInts, inFloats, inStrings, inBuffer)
	handles = simGetObjectsInTree(sim_handle_scene, sim.object_shape_type, 0)
	ret = {}
	for i = numberOfObjects, #handles, 1 do
		property=sim.getObjectSpecialProperty(handles[i])
        val = sim.boolAnd32(property, sim.objectspecialproperty_renderable)
		if val>0 then
			simpleShapeHandles=sim.ungroupShape(handles[i])
			for ii = 1, #simpleShapeHandles, 1 do
				ret[#ret+1] = simpleShapeHandles[ii]
			end
		end
	end
	handles = simGetObjectsInTree(sim_handle_scene, sim.object_shape_type, 0)
	numberOfObjects = #handles
	return ret, {}, {}, ''
end

doWeHaveManu = function(inInts, inFloats, inStrings, inBuffer)
	h = {}
	h[1] = sim.getObjectHandle('Menu@silentError')
	return h, {}, {}, ''
end


function sysCall_init()
    -- simRemoteApi.start(19999,1300,true) -- if using different port

	-- Lighting strength parameters
	sim.setFloatSignal('AmbientStrength',0.3) -- 0-1
	sim.setFloatSignal('DiffuseStrength',0.7)
	sim.setFloatSignal('SpecularStrength',0.3)
	sim.setFloatSignal('SpecularPower',5.0)
	
	-- Initalize all possible signals 
    sim.setIntegerSignal('L_Trigger_Press',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_Trigger_Press',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_Trigger_Touch',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_Trigger_Touch',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_Grip_Press',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_Grip_Press',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_Grip_Touch',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_Grip_Touch',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_TrackPad_Press',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_TrackPad_Press',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_TrackPad_Touch',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_TrackPad_Touch',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_Joystick_Press',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_Joystick_Press',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_Joystick_Touch',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_Joystick_Touch',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_ApplicationMenu_Press',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_ApplicationMenu_Press',0) -- set up string signal to transfer button state
	
	sim.setIntegerSignal('L_ApplicationMenu_Touch',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('R_ApplicationMenu_Touch',0) -- set up string signal to transfer button state
	
	-- Now some extra signals
	sim.setFloatSignal('L_Trackpad_pos_x',0) -- trackpad
	sim.setFloatSignal('L_Trackpad_pos_y',0)
	sim.setFloatSignal('L_Trackpad_pos_z',0)
	sim.setFloatSignal('R_Trackpad_pos_x',0)
	sim.setFloatSignal('R_Trackpad_pos_y',0)
	sim.setFloatSignal('R_Trackpad_pos_z',0)
	
	sim.setFloatSignal('L_Joystick_pos_x',0) -- joystick, whatever this might be
	sim.setFloatSignal('L_Joystick_pos_y',0)
	sim.setFloatSignal('L_Joystick_pos_z',0)
	sim.setFloatSignal('R_Joystick_pos_x',0)
	sim.setFloatSignal('R_Joystick_pos_y',0)
	sim.setFloatSignal('R_Joystick_pos_z',0)
	
	dynamicLoading = false
	goodRendering = false
	if (sim.getScriptSimulationParameter(sim.handle_self, "Enable_dynamic_objects_loading")==true) then
		dynamicLoading = true
		print("Dynamic object loading is activated")
	end
	if (sim.getScriptSimulationParameter(sim.handle_self, "High_quality_render")==true) then
		goodRendering = true
		sim.setIntegerSignal("High_quality_render",1)
		print("The good rendering is activated")
	else
		sim.setIntegerSignal("High_quality_render",0)
	end
	sim.setIntegerSignal('dynamic_load_request',0)
end
